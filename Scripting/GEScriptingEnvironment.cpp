
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Scripting
//
//  --- GEScriptingEnvironment.cpp ---
//
//////////////////////////////////////////////////////////////////

#if defined _MSC_VER
# pragma warning(disable : 4503)
#endif

#include "GEScriptingEnvironment.h"

#include "Core/GEPlatform.h"
#include "Core/GEApplication.h"
#include "Core/GELog.h"
#include "Core/GEDevice.h"
#include "Core/GEInterpolator.h"
#include "Core/GEPhysics.h"
#include "Core/GEProfiler.h"
#include "Content/GEContentData.h"
#include "Input/GEInputSystem.h"
#include "Entities/GEScene.h"
#include "Entities/GEComponentTransform.h"
#include "Entities/GEComponentSprite.h"
#include "Entities/GEComponentParticleSystem.h"
#include "Entities/GEComponentCamera.h"
#include "Entities/GEComponentCollider.h"
#include "Entities/GEComponentUIElement.h"
#include "Entities/GEComponentDataContainer.h"
#include "Entities/GEComponentAudio.h"
#include "Entities/GEComponentScript.h"
#include "Rendering/GERenderSystem.h"
#include "Rendering/GEMaterial.h"
#include "Audio/GEAudioSystem.h"
#include "Types/GECurve.h"
#include "Types/GEBezierCurve.h"

#include <cstring>


using namespace GE;
using namespace GE::Scripting;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Entities;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Input;


//
//  ScriptingEnvironment
//
const size_t MemoryPoolSize = 16 * 1024 * 1024;

void* ScriptingEnvironment::pAllocatorBuffer = 0;
tlsf_t ScriptingEnvironment::pAllocator = 0;
GEMutex mAllocatorMutex;

GESTLSet(uint) ScriptingEnvironment::sPredefinedGlobalSymbols;
GESTLVector(ScriptingEnvironment::registerTypesExtension) ScriptingEnvironment::vRegisterTypesExtensions;

GESTLMap(uint32_t, ScriptingEnvironment::SharedEnvironment) ScriptingEnvironment::mSharedEnvironments;

ScriptingEnvironment::ScriptingEnvironment()
   : bReset(true)
#if defined (GE_EDITOR_SUPPORT)
   , iDebugBreakpointLine(0)
   , bDebuggerActive(false)
#endif
{
   GEMutexInit(mAccessMutex);

   lua.open_libraries();
   registerTypes();
}

ScriptingEnvironment::~ScriptingEnvironment()
{
   GEMutexDestroy(mAccessMutex);
}

void ScriptingEnvironment::initStaticData()
{
   GEAssert(!pAllocatorBuffer);
   GEAssert(!pAllocator);

   // initialize allocator
   pAllocatorBuffer = Allocator::alloc<char>(MemoryPoolSize, AllocationCategory::Scripting);
   pAllocator = tlsf_create_with_pool(pAllocatorBuffer, MemoryPoolSize);
   GEMutexInit(mAllocatorMutex);

   // collect predefined global symbols
   collectPredefinedGlobalSymbols();
}

void ScriptingEnvironment::releaseStaticData()
{
   // clear predefined global symbols
   sPredefinedGlobalSymbols.clear();

   // release allocator
   GEMutexDestroy(mAllocatorMutex);
   tlsf_destroy(pAllocator);
   pAllocator = 0;
   Allocator::free(pAllocatorBuffer);
   pAllocatorBuffer = 0;
}

void ScriptingEnvironment::addRegisterTypesExtension(registerTypesExtension Extension)
{
   vRegisterTypesExtensions.push_back(Extension);
   collectPredefinedGlobalSymbols();
}

ScriptingEnvironment* ScriptingEnvironment::requestSharedEnvironment(const Core::ObjectName& Name)
{
   ScriptingEnvironment* env = 0;

   GESTLMap(uint32_t, SharedEnvironment)::iterator it = mSharedEnvironments.find(Name.getID());

   if(it == mSharedEnvironments.end())
   {
      env = Allocator::alloc<ScriptingEnvironment>();
      GEInvokeCtor(ScriptingEnvironment, env)();

      SharedEnvironment sharedEnv;
      sharedEnv.Env = env;
      sharedEnv.Refs = 1;

      std::pair<uint32_t, SharedEnvironment> sharedEnvironmentPair;
      sharedEnvironmentPair.first = Name.getID();
      sharedEnvironmentPair.second = sharedEnv;

      mSharedEnvironments.insert(sharedEnvironmentPair);
   }
   else
   {
      env = it->second.Env;
      it->second.Refs++;
   }

   return env;
}

void ScriptingEnvironment::leaveSharedEnvironment(const Core::ObjectName& Name)
{
   GESTLMap(uint32_t, SharedEnvironment)::iterator it = mSharedEnvironments.find(Name.getID());

   if(it != mSharedEnvironments.end())
   {
      it->second.Refs--;

      if(it->second.Refs == 0)
      {
         GEInvokeDtor(ScriptingEnvironment, it->second.Env);
         Allocator::free(it->second.Env);

         mSharedEnvironments.erase(it);
      }
   }
}

void ScriptingEnvironment::handleScriptError(const char* ScriptName, const char* Msg)
{
   if(Msg)
   {
      Log::log(LogType::Error, "Lua error (the '%s' script could not be loaded): %s", ScriptName, Msg);
   }
   else
   {
      Log::log(LogType::Error, "Lua error (the '%s' script could not be loaded)", ScriptName);
   }
}

void ScriptingEnvironment::handleFunctionError(const char* FunctionName, const char* Msg)
{
   if(Msg)
   {
      Log::log(LogType::Error, "Lua error ('%s' function): %s", FunctionName, Msg);
   }
   else
   {
      Log::log(LogType::Error, "Lua error ('%s' function)", FunctionName);
   }
}

void ScriptingEnvironment::reset()
{
   if(bReset)
      return;

   GEProfilerMarker("ScriptingEnvironment::reset()");

   vGlobalVariableNames.clear();
   vGlobalFunctionNames.clear();
   mFunctions.clear();

   lua = sol::state(sol::default_at_panic, customAlloc);
   lua.open_libraries();
   registerTypes();

   bReset = true;
}

void ScriptingEnvironment::collectGarbage()
{
   lua.collect_garbage();
}

void ScriptingEnvironment::loadFromCode(const GESTLString& Code)
{
   lua.script(Code.c_str());
   collectGlobalSymbols();
   bReset = false;
}

bool ScriptingEnvironment::loadFromFile(const char* FileName)
{
   GEProfilerMarker("ScriptingEnvironment::loadFromFile()");

   sol::table returnValue;

   if(!loadModule(FileName, &returnValue))
      return false;

#if defined (GE_EDITOR_SUPPORT)
   loadModule("_ed_debug", &returnValue);
#endif

   collectGlobalSymbols();
   bReset = false;

   return true;
}

ValueType ScriptingEnvironment::getVariableType(const ObjectName& VariableName) const
{
   lua_State* luaState = lua.lua_state();
   lua_getglobal(luaState, VariableName.getString());

   ValueType eValueType = ValueType::Count;
   bool bUserDataType = false;
   int iIndex = lua_gettop(luaState);

   if(lua_isboolean(luaState, iIndex))
      eValueType = ValueType::Bool;
   else if(lua_isinteger(luaState, iIndex))
      eValueType = ValueType::Int;
   else if(lua_isnumber(luaState, iIndex))
      eValueType = ValueType::Float;
   else if(lua_isstring(luaState, iIndex))
      eValueType = ValueType::String;
   else if(lua_isuserdata(luaState, iIndex))
      bUserDataType = true;
   
   lua_pop(luaState, 1);

   if(bUserDataType)
   {
      const sol::object& cVariableRef = lua[VariableName.getString()];

      if(cVariableRef.is<Vector3>())
         eValueType = ValueType::Vector3;
      else if(cVariableRef.is<Vector2>())
         eValueType = ValueType::Vector2;
      else if(cVariableRef.is<Color>())
         eValueType = ValueType::Color;
   }

   return eValueType;
}

bool ScriptingEnvironment::isFunctionDefined(const ObjectName& FunctionName) const
{
   return mFunctions.find(FunctionName.getID()) != mFunctions.end();
}

uint ScriptingEnvironment::getFunctionParametersCount(const ObjectName& FunctionName) const
{
   lua_State* luaState = lua.lua_state();
   lua_Debug luaDebug;

   lua_getglobal(luaState, FunctionName.getString());
   lua_getinfo(luaState, ">u", &luaDebug);

   return (uint)luaDebug.nparams;
}

#if defined (GE_EDITOR_SUPPORT)
void ScriptingEnvironment::setDebugBreakpointLine(uint Line)
{
   iDebugBreakpointLine = Line;
}

void ScriptingEnvironment::enableDebugger()
{
   lua["debugger"] = [this](int iEvent, int iLine)
   {
      if(!bDebuggerActive)
      {
         if(iDebugBreakpointLine == 0 || iDebugBreakpointLine == (uint)iLine)
         {
            bDebuggerActive = true;
         }
         else
         {
            return;
         }
      }

      std::string sCodeStr = lua.script("return debug.getinfo(3).source");
      size_t iCodeLength = sCodeStr.length();

      char sCodeLine[256];
      int iCurrentLine = 1;

      size_t iCodeGlobalCharIndex = 0;
      size_t iCodeLineCharIndex = 0;

      printf("\n");

      while(iCodeGlobalCharIndex < iCodeLength)
      {
         while(sCodeStr[iCodeGlobalCharIndex] != '\n' && sCodeStr[iCodeGlobalCharIndex] != '\0')
         {
            sCodeLine[iCodeLineCharIndex++] = sCodeStr[iCodeGlobalCharIndex++];
         }

         sCodeLine[iCodeLineCharIndex] = '\0';
         iCodeLineCharIndex = 0;

         if(abs(iCurrentLine - iLine) < 10)
         {
            printf("%c%4d: %s\n", iCurrentLine == iLine ? '>' : ' ', iCurrentLine, sCodeLine);
         }

         iCurrentLine++;
         iCodeGlobalCharIndex++;
      }

      std::string sCommandBuffer;
      bool bContinue = false;

      do
      {
         printf("\nDebugger> ");
         std::getline(std::cin, sCommandBuffer);

         if(strcmp(sCommandBuffer.c_str(), "exit") == 0)
         {
            disableDebugger();
            bContinue = true;
         }
         else if(strcmp(sCommandBuffer.c_str(), "s") == 0)
         {
            bContinue = true;
         }
         else
         {
            lua.script(sCommandBuffer);
            bContinue = false;
         }
      }
      while(!bContinue);
   };
   lua.script("debug.sethook(debugger, \"l\")");
}

void ScriptingEnvironment::disableDebugger()
{
   lua.script("debug.sethook()");
   lua["debugger"] = nullptr;
   bDebuggerActive = false;
}
#endif

void* ScriptingEnvironment::customAlloc(void*, void* ptr, size_t, size_t nsize)
{
   GEMutexLock(mAllocatorMutex);

   if(nsize == 0)
   {
      if(ptr)
      {
         tlsf_free(pAllocator, ptr);
      }

      GEMutexUnlock(mAllocatorMutex);

      return 0;
   }

   void* pMemoryBlock = ptr
      ? tlsf_realloc(pAllocator, ptr, nsize)
      : tlsf_malloc(pAllocator, nsize);

   GEMutexUnlock(mAllocatorMutex);

   return pMemoryBlock;
}

bool ScriptingEnvironment::alphabeticalComparison(const ObjectName& l, const ObjectName& r)
{
   return strcmp(l.getString(), r.getString()) < 0;
}

void ScriptingEnvironment::collectPredefinedGlobalSymbols()
{
   sPredefinedGlobalSymbols.clear();

   ScriptingEnvironment cDefaultEnv;
   lua_State* luaState = cDefaultEnv.lua.lua_state();

   lua_pushglobaltable(luaState);
   lua_pushnil(luaState);

   while(lua_next(luaState, -2) != 0)
   {
      const char* sVariableName = lua_tostring(luaState, -2);
      ObjectName cVariableName = ObjectName(sVariableName);
      addPredefinedGlobalSymbol(cVariableName);
      lua_pop(luaState, 1);
   }

   lua_pop(luaState, 1);

   // add GE variable names
   addPredefinedGlobalSymbol(ObjectName("deltaTime"));
   addPredefinedGlobalSymbol(ObjectName("entity"));
   addPredefinedGlobalSymbol(ObjectName("this"));
}

void ScriptingEnvironment::addPredefinedGlobalSymbol(const ObjectName& Symbol)
{
   sPredefinedGlobalSymbols.insert(Symbol.getID());
}

bool ScriptingEnvironment::loadModule(const char* sModuleName, sol::table* pOutReturnValue)
{
   GEProfilerMarker("ScriptingEnvironment::loadModule()");

   ContentData cContentData;
   uint32_t codeSize = 0;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      if(!Device::contentFileExists("Scripts", sModuleName, "lua"))
         return false;

      Device::readContentFile(ContentType::GenericTextData, "Scripts", sModuleName, "lua", &cContentData);
      codeSize = cContentData.getDataSize() - 1;
   }
   else
   {
#if defined (GE_64_BIT)
      char sFileNamex64[64];
      sprintf(sFileNamex64, "x64_%s", sModuleName);

      if(!Device::contentFileExists("Scripts", sFileNamex64, "luabc"))
         return false;

      Device::readContentFile(ContentType::GenericBinaryData, "Scripts", sFileNamex64, "luabc", &cContentData);
#else
      if(!Device::contentFileExists("Scripts", sModuleName, "luabc"))
         return false;

      Device::readContentFile(ContentType::GenericBinaryData, "Scripts", sModuleName, "luabc", &cContentData);
#endif
      codeSize = cContentData.getDataSize();
   }

   sol::load_result loadFunction = lua.load_buffer(cContentData.getData(), codeSize, sModuleName);

   if(!loadFunction.valid())
   {
      sol::error luaError = loadFunction;
      handleScriptError(sModuleName, luaError.what());
      return false;
   }

   *pOutReturnValue = loadFunction();

   return true;
}

void ScriptingEnvironment::collectGlobalSymbols()
{
   GEProfilerMarker("ScriptingEnvironment::collectGlobalSymbols()");

   // collect all global user symbols
   GESTLVector(ObjectName) vGlobalUserSymbols;

   lua_State* luaState = lua.lua_state();

   lua_pushglobaltable(luaState);
   lua_pushnil(luaState);

   while(lua_next(luaState, -2) != 0)
   {
      const char* sVariableName = lua_tostring(luaState, -2);
      ObjectName cVariableName = ObjectName(sVariableName);

      if(sPredefinedGlobalSymbols.find(cVariableName.getID()) == sPredefinedGlobalSymbols.end())
      {
         vGlobalUserSymbols.push_back(cVariableName);
      }

      lua_pop(luaState, 1);
   }

   lua_pop(luaState, 1);

   // fill the lists of variables and functions
   for(uint i = 0; i < vGlobalUserSymbols.size(); i++)
   {
      lua_getglobal(luaState, vGlobalUserSymbols[i].getString());
      
      if(lua_isfunction(luaState, lua_gettop(luaState)))
      {
         vGlobalFunctionNames.push_back(vGlobalUserSymbols[i]);
         mFunctions[vGlobalUserSymbols[i].getID()] = lua[vGlobalUserSymbols[i].getString()];
      }
      else
      {
         vGlobalVariableNames.push_back(vGlobalUserSymbols[i]);
      }

      lua_pop(luaState, 1);
   }

#if defined (GE_EDITOR_SUPPORT)
   std::sort(vGlobalVariableNames.begin(), vGlobalVariableNames.end(), alphabeticalComparison);
   std::sort(vGlobalFunctionNames.begin(), vGlobalFunctionNames.end(), alphabeticalComparison);
#endif
}

void ScriptingEnvironment::registerTypes()
{
   GEProfilerMarker("ScriptingEnvironment::registerTypes()");

   //
   //  Module loading
   //
   lua["packageSearcher"] = [this](const char* sModuleName)
   {
      char moduleLoaderName[64];
      sprintf(moduleLoaderName, "%sLoader", sModuleName);
      lua[moduleLoaderName] = [this, sModuleName]() -> sol::table
      {
         sol::table returnValue;
         loadModule(sModuleName, &returnValue);
         return returnValue;
      };

      char codeBuffer[256];
      sprintf(codeBuffer, "package.preload['%s'] = %s", sModuleName, moduleLoaderName);
      lua.script(codeBuffer);
   };
   lua.script("table.insert(package.searchers, 1, packageSearcher)");

   //
   //  Global functions
   //
   lua["logInfo"] = [](const char* sMessage){ Log::log(LogType::Info, "[Lua] %s", sMessage); };
   lua["logWarning"] = [](const char* sMessage){ Log::log(LogType::Warning, "[Lua] %s", sMessage); };
   lua["logError"] = [](const char* sMessage){ Log::log(LogType::Error, "[Lua] %s", sMessage); };

   //
   //  GE
   //
   lua.new_simple_usertype<Vector2>
   (
      "Vector2"
      , sol::constructors<sol::types<>, sol::types<float, float>>()
      , "X", &Vector2::X
      , "Y", &Vector2::Y
      , "normalize", &Vector2::normalize
      , "getSquaredLength", &Vector2::getSquaredLength
      , "getLength", &Vector2::getLength
      , "getDistanceTo", &Vector2::getDistanceTo
      , "lerp", &Vector2::lerp
      , sol::meta_method::addition, &Vector2::operator+
      , sol::meta_method::subtraction, (Vector2 (Vector2::*)(const Vector2&) const)&Vector2::operator-
      , sol::meta_method::multiplication, (Vector2 (Vector2::*)(const float) const)&Vector2::operator*
      , sol::meta_method::unary_minus, (Vector2 (Vector2::*)())&Vector2::operator-
   );
   lua.new_simple_usertype<Vector3>
   (
      "Vector3"
      , sol::constructors<sol::types<>, sol::types<float, float, float>, sol::types<float>>()
      , "X", &Vector3::X
      , "Y", &Vector3::Y
      , "Z", &Vector3::Z
      , "normalize", &Vector3::normalize
      , "getSquaredLength", &Vector3::getSquaredLength
      , "getLength", &Vector3::getLength
      , "getDistanceTo", &Vector3::getDistanceTo
      , "dotProduct", &Vector3::dotProduct
      , "crossProduct", &Vector3::crossProduct
      , "lerp", &Vector3::lerp
      , sol::meta_method::addition, &Vector3::operator+
      , sol::meta_method::subtraction, (Vector3 (Vector3::*)(const Vector3&) const)&Vector3::operator-
      , sol::meta_method::multiplication, (Vector3 (Vector3::*)(const float) const)&Vector3::operator*
      , sol::meta_method::unary_minus, (Vector3 (Vector3::*)())&Vector3::operator-
   );
   lua.new_simple_usertype<Rotation>
   (
      "Rotation"
      , sol::constructors<sol::types<>, sol::types<const Vector3&>, sol::types<const Vector3&, float>>()
      , "setFromEulerAngles", &Rotation::setFromEulerAngles
      , "setFromAxisAngle", &Rotation::setFromAxisAngle
      , "setFromQuaternion", &Rotation::setFromQuaternion
   );
   lua.new_simple_usertype<Color>
   (
      "Color"
      , sol::constructors<sol::types<>, sol::types<float, float, float, float>>()
      , "Red", &Color::Red
      , "Green", &Color::Green
      , "Blue", &Color::Blue
      , "Alpha", &Color::Alpha
      , sol::meta_method::equal_to, &Color::operator==
   );

   //
   //  GE::Core
   //
   lua.new_simple_usertype<ObjectName>
   (
      "ObjectName"
      , sol::constructors<sol::types<>, sol::types<const char*>>()
      , "getID", &ObjectName::getID
      , "getString", &ObjectName::getString
      , "isEmpty", &ObjectName::isEmpty
      , sol::meta_method::equal_to, &ObjectName::operator==
   );
   lua.new_simple_usertype<Value>
   (
      "Value"
      , sol::constructors<sol::types<bool>, sol::types<uint8_t>, sol::types<int>, sol::types<uint32_t>,
         sol::types<float>, sol::types<const char*>,
         sol::types<Vector2>, sol::types<Vector3>, sol::types<Color>, sol::types<const ObjectName&>>()
      , "setAsBool", &Value::setAsBool
      , "setAsByte", &Value::setAsByte
      , "setAsInt", &Value::setAsInt
      , "setAsUInt", &Value::setAsUInt
      , "setAsFloat", &Value::setAsFloat
      , "setAsString", &Value::setAsString
      , "setAsVector2", &Value::setAsVector2
      , "setAsVector3", &Value::setAsVector3
      , "setAsColor", &Value::setAsColor
      , "setAsObjectName", &Value::setAsObjectName
      , "getAsBool", &Value::getAsBool
      , "getAsByte", &Value::getAsByte
      , "getAsInt", &Value::getAsInt
      , "getAsUInt", &Value::getAsUInt
      , "getAsFloat", &Value::getAsFloat
      , "getAsString", &Value::getAsString
      , "getAsVector2", &Value::getAsVector2
      , "getAsVector3", &Value::getAsVector3
      , "getAsColor", &Value::getAsColor
      , "getAsObjectName", &Value::getAsObjectName
   );
   lua.new_simple_usertype<Serializable>
   (
      "Serializable"
      , "get", &Serializable::get
      , "set", &Serializable::set
      , "executeAction", &Serializable::executeAction
   );
   lua.new_enum
   (
      "InterpolationMode"
      , "Linear", InterpolationMode::Linear
      , "Quadratic", InterpolationMode::Quadratic
      , "QuadraticInverse", InterpolationMode::QuadraticInverse
      , "Logarithmic", InterpolationMode::Logarithmic
   );
   lua.new_simple_usertype<PropertyInterpolator<float>>
   (
      "PropertyInterpolatorFloat"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &PropertyInterpolator<float>::animate
      , "alternate", &PropertyInterpolator<float>::alternate
      , "update", &PropertyInterpolator<float>::update
   );
   lua.new_simple_usertype<PropertyInterpolator<Vector2>>
   (
      "PropertyInterpolatorVector2"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &PropertyInterpolator<Vector2>::animate
      , "alternate", &PropertyInterpolator<Vector2>::alternate
      , "update", &PropertyInterpolator<Vector2>::update
   );
   lua.new_simple_usertype<PropertyInterpolator<Vector3>>
   (
      "PropertyInterpolatorVector3"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &PropertyInterpolator<Vector3>::animate
      , "alternate", &PropertyInterpolator<Vector3>::alternate
      , "update", &PropertyInterpolator<Vector3>::update
   );
   lua.new_simple_usertype<PropertyInterpolator<Color>>
   (
      "PropertyInterpolatorColor"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &PropertyInterpolator<Color>::animate
      , "alternate", &PropertyInterpolator<Color>::alternate
      , "update", &PropertyInterpolator<Color>::update
   );
   lua.new_simple_usertype<CurveKey>
   (
      "CurveKey"
      , "getTimePosition", &CurveKey::getTimePosition
      , "getValue", &CurveKey::getValue
      , "setTimePosition", &CurveKey::setTimePosition
      , "setValue", &CurveKey::setValue
   );
   lua.new_simple_usertype<Curve>
   (
      "Curve"
      , "getValue", &Curve::getValue
      , "getCurveKeyCount", &Curve::getCurveKeyCount
      , "getCurveKey", &Curve::getCurveKey
   );
   lua.new_enum
   (
      "PropertyValueComponent"
      , "None", PropertyValueComponent::None
      , "X", PropertyValueComponent::X
      , "Y", PropertyValueComponent::Y
      , "Z", PropertyValueComponent::Z
      , "Red", PropertyValueComponent::Red
      , "Green", PropertyValueComponent::Green
      , "Blue", PropertyValueComponent::Blue
      , "Alpha", PropertyValueComponent::Alpha
   );
   lua.new_simple_usertype<CurvePropertyInterpolator>
   (
      "CurvePropertyInterpolator"
      , sol::constructors<sol::types<Curve*, Serializable*, const ObjectName&, PropertyValueComponent>>()
      , "setClockName", &CurvePropertyInterpolator::setClockName
      , "animate", &CurvePropertyInterpolator::animate
      , "animateInverse", &CurvePropertyInterpolator::animateInverse
      , "update", &CurvePropertyInterpolator::update
   );
   lua.new_simple_usertype<BezierCurve>
   (
      "BezierCurve"
      , "getPoint", &BezierCurve::getPoint
   );
   lua.new_simple_usertype<BezierPropertyInterpolator>
   (
      "BezierPropertyInterpolator"
      , sol::constructors<sol::types<BezierCurve*, Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &BezierPropertyInterpolator::animate
      , "animateInverse", &BezierPropertyInterpolator::animateInverse
      , "update", &BezierPropertyInterpolator::update
   );
   lua.new_simple_usertype<Physics::Ray>
   (
      "Ray"
      , sol::constructors<sol::types<const Vector3&, const Vector3&>>()
   );
   lua.new_simple_usertype<Physics::HitInfo>
   (
      "HitInfo"
      , sol::constructors<sol::types<>>()
      , "Collider", &Physics::HitInfo::Collider
      , "Position", &Physics::HitInfo::Position
      , "Normal", &Physics::HitInfo::Normal
      , "Distance", &Physics::HitInfo::Distance
   );

   //
   //  GE::Content
   //
   lua.new_simple_usertype<Skeleton>
   (
      "Skeleton"
      , "getBonesCount", &Skeleton::getBonesCount
   );

   //
   //  GE::Input
   //
   lua.new_simple_usertype<InputSystem>
   (
      "InputSystem"
      , "getInstance", &InputSystem::getInstance
      , "setInputEnabled", &InputSystem::setInputEnabled
      , "getMousePosition", &InputSystem::getMousePosition
   );

   //
   //  GE::Entities
   //
   lua.new_simple_usertype<Scene>
   (
      "Scene"
      , "getActiveScene", &Scene::getActiveScene
      , "getPermanentScene", &Scene::getPermanentScene
      , "getEntity", &Scene::getEntity
      , "addEntity", (Entity* (Scene::*)(const ObjectName&, Entity*))&Scene::addEntity
      , "addPrefab", (Entity* (Scene::*)(const char*, const ObjectName&, Entity*))&Scene::addPrefab
      , "cloneEntity", &Scene::cloneEntity
      , "removeEntity", (bool (Scene::*)(const ObjectName&))&Scene::removeEntity
      , "renameEntity", &Scene::renameEntity
      , sol::base_classes, sol::bases<Serializable>()
   );
   lua.new_simple_usertype<Entity>
   (
      "Entity"
      , "getName", &Entity::getName
      , "getFullName", &Entity::getFullName
      , "getActive", &Entity::getActive
      , "setActive", &Entity::setActive
      , "addComponent", (Component* (Entity::*)(const Core::ObjectName&))&Entity::addComponent
      , "addComponentTransform", &Entity::addComponent<ComponentTransform>
      , "addComponentSprite", &Entity::addComponent<ComponentSprite>
      , "addComponentMesh", &Entity::addComponent<ComponentMesh>
      , "addComponentParticleSystem", &Entity::addComponent<ComponentParticleSystem>
      , "addComponentCamera", &Entity::addComponent<ComponentCamera>
      , "addComponentColliderSphere", &Entity::addComponent<ComponentColliderSphere>
      , "addComponentColliderMesh", &Entity::addComponent<ComponentColliderMesh>
      , "addComponentUI2DElement", &Entity::addComponent<ComponentUI2DElement>
      , "addComponentUI3DElement", &Entity::addComponent<ComponentUI3DElement>
      , "addComponentSkeleton", &Entity::addComponent<ComponentSkeleton>
      , "addComponentDataContainer", &Entity::addComponent<ComponentDataContainer>
      , "addComponentScript", &Entity::addComponent<ComponentScript>
      , "getComponentTransform", &Entity::getComponent<ComponentTransform>
      , "getComponentRenderable", &Entity::getComponent<ComponentRenderable>
      , "getComponentSprite", &Entity::getComponent<ComponentSprite>
      , "getComponentLabel", &Entity::getComponent<ComponentLabel>
      , "getComponentMesh", &Entity::getComponent<ComponentMesh>
      , "getComponentParticleSystem", &Entity::getComponent<ComponentParticleSystem>
      , "getComponentCamera", &Entity::getComponent<ComponentCamera>
      , "getComponentCollider", &Entity::getComponent<ComponentCollider>
      , "getComponentUIElement", &Entity::getComponent<ComponentUIElement>
      , "getComponentUI2DElement", &Entity::getComponent<ComponentUI2DElement>
      , "getComponentUI3DElement", &Entity::getComponent<ComponentUI3DElement>
      , "getComponentSkeleton", &Entity::getComponent<ComponentSkeleton>
      , "getComponentDataContainer", &Entity::getComponent<ComponentDataContainer>
      , "getComponentAudioSource", &Entity::getComponent<ComponentAudioSource>
      , "getComponentScript", &Entity::getComponent<ComponentScript>
      , "getChildrenCount", &Entity::getChildrenCount
      , "getChildByIndex", &Entity::getChildByIndex
      , "getChildByName", &Entity::getChildByName
      , "getParent", &Entity::getParent
      , "init", &Entity::init
      , sol::base_classes, sol::bases<Serializable>()
   );
   lua.new_simple_usertype<Component>
   (
      "Component"
      , "getOwner", &Component::getOwner
      , sol::base_classes, sol::bases<Serializable>()
   );
   lua.new_simple_usertype<ComponentTransform>
   (
      "ComponentTransform"
      , "move", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::move
      , "rotate", &ComponentTransform::rotate
      , "scale", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::scale
      , "getPosition", &ComponentTransform::getPosition
      , "getWorldPosition", &ComponentTransform::getWorldPosition
      , "getRotation", &ComponentTransform::getRotation
      , "getOrientation", &ComponentTransform::getOrientation
      , "getWorldRotation", &ComponentTransform::getWorldRotation
      , "getScale", &ComponentTransform::getScale
      , "getWorldScale", &ComponentTransform::getWorldScale
      , "getForwardVector", &ComponentTransform::getForwardVector
      , "getRightVector", &ComponentTransform::getRightVector
      , "getUpVector", &ComponentTransform::getUpVector
      , "setPosition", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::setPosition
      , "setRotation", &ComponentTransform::setRotation
      , "setOrientation", &ComponentTransform::setOrientation
      , "setScale", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::setScale
      , "setForwardVector", &ComponentTransform::setForwardVector
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentRenderable>
   (
      "ComponentRenderable"
      , "getMaterialPassCount", &ComponentRenderable::getMaterialPassCount
      , "getMaterialPass", &ComponentRenderable::getMaterialPass
      , "addMaterialPass", &ComponentRenderable::addMaterialPass
      , "removeMaterialPass", &ComponentRenderable::removeMaterialPass
      , "getVisible", &ComponentRenderable::getVisible
      , "getColor", &ComponentRenderable::getColor
      , "getRenderPriority", &ComponentRenderable::getRenderPriority
      , "setVisible", &ComponentRenderable::setVisible
      , "setColor", &ComponentRenderable::setColor
      , "setRenderPriority", &ComponentRenderable::setRenderPriority
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentSprite>
   (
      "ComponentSprite"
      , "isOver", &ComponentSprite::isOver
      , "getTextureAtlasName", &ComponentSprite::getTextureAtlasName
      , "getSize", &ComponentSprite::getSize
      , "getScaledYSize", &ComponentSprite::getScaledYSize
      , "setTextureAtlasName", &ComponentSprite::setTextureAtlasName
      , "setSize", &ComponentSprite::setSize
      , "setScaledYSize", &ComponentSprite::setScaledYSize
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentLabel>
   (
      "ComponentLabel"
      , "getText", &ComponentLabel::getText
      , "getStringID", &ComponentLabel::getStringID
      , "setText", &ComponentLabel::setText
      , "setStringID", &ComponentLabel::setStringID
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentMesh>
   (
      "ComponentMesh"
      , "getMeshName", &ComponentMesh::getMeshName
      , "setMeshName", &ComponentMesh::setMeshName
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentParticleSystem>
   (
      "ComponentParticleSystem"
      , "getEmitterActive", &ComponentParticleSystem::getEmitterActive
      , "setEmitterActive", &ComponentParticleSystem::setEmitterActive
      , "burst", &ComponentParticleSystem::burst
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentCamera>
   (
      "ComponentCamera"
      , "getScreenRay", &ComponentCamera::getScreenRay
      , "worldToScreen", &ComponentCamera::worldToScreen
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentCollider>
   (
      "ComponentCollider"
      , "checkCollision", &ComponentCollider::checkCollision
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentUIElement>
   (
      "ComponentUIElement"
      , "getAlpha", &ComponentUIElement::getAlpha
      , "setAlpha", &ComponentUIElement::setAlpha
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentUI2DElement>
   (
      "ComponentUI2DElement"
      , "getOffset", &ComponentUI2DElement::getOffset
      , "getScaledYOffset", &ComponentUI2DElement::getScaledYOffset
      , "setOffset", &ComponentUI2DElement::setOffset
      , "setScaledYOffset", &ComponentUI2DElement::setScaledYOffset
      , sol::base_classes, sol::bases<ComponentUIElement, Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentUI3DElement>
   (
      "ComponentUI3DElement"
      , "getCanvasIndex", &ComponentUI3DElement::getCanvasIndex
      , "setCanvasIndex", &ComponentUI3DElement::setCanvasIndex
      , sol::base_classes, sol::bases<ComponentUIElement, Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentSkeleton>
   (
      "ComponentSkeleton"
      , "getSkeleton", &ComponentSkeleton::getSkeleton
      , "getBoneEntity", (Entity* (ComponentSkeleton::*)(const ObjectName&) const)&ComponentSkeleton::getBoneEntity
      , "getBoneEntityByIndex", (Entity* (ComponentSkeleton::*)(uint) const)&ComponentSkeleton::getBoneEntity
      , "playAnimation", &ComponentSkeleton::playAnimation
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_enum
   (
      "AnimationPlayMode"
      , "Loop", AnimationPlayMode::Loop
      , "Once", AnimationPlayMode::Once
   );
   lua.new_simple_usertype<AnimationPlayInfo>
   (
      "AnimationPlayInfo"
      , "AnimationName", &AnimationPlayInfo::AnimationName
      , "Mode", &AnimationPlayInfo::Mode
      , "BlendTime", &AnimationPlayInfo::BlendTime
      , "Speed", &AnimationPlayInfo::Speed
   );
   lua.new_simple_usertype<ComponentDataContainer>
   (
      "ComponentDataContainer"
      , "getVariable", (const Value* (ComponentDataContainer::*)(const ObjectName&))&ComponentDataContainer::getVariable
      , "setVariable", &ComponentDataContainer::setVariable
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentAudioSource>
   (
      "ComponentAudioSource"
      , "playAudioEvent", &ComponentAudioSource::playAudioEvent
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_simple_usertype<ComponentScript>
   (
      "ComponentScript"
      , "getScriptInstanceCount", &ComponentScript::getScriptInstanceCount
      , "getScriptInstance", &ComponentScript::getScriptInstance
      , "getScriptInstanceByName", &ComponentScript::getScriptInstanceByName
      , "addScriptInstance", &ComponentScript::addScriptInstance
      , "removeScriptInstance", &ComponentScript::removeScriptInstance
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_simple_usertype<ScriptInstance>
   (
      "ScriptInstance"
      , "getScriptName", &ScriptInstance::getScriptName
      , "setScriptName", &ScriptInstance::setScriptName
      , "getActive", &ScriptInstance::getActive
      , "setActive", &ScriptInstance::setActive
      , sol::base_classes, sol::bases<Serializable>()
   );

   //
   //  GE::Rendering
   //
   lua.new_simple_usertype<RenderSystem>
   (
      "RenderSystem"
      , "getInstance", &RenderSystem::getInstance
      , "getActiveCamera", &RenderSystem::getActiveCamera
   );
   lua.new_simple_usertype<Texture>
   (
      "Texture"
      , "getAtlasSize", &Texture::getAtlasSize
      , "getAtlasName", &Texture::getAtlasName
   );
   lua.new_simple_usertype<Material>
   (
      "Material"
      , "getShaderProgram", &Material::getShaderProgram
      , "getDiffuseColor", &Material::getDiffuseColor
      , "getDiffuseTexture", &Material::getDiffuseTexture
      , "getBlendingMode", &Material::getBlendingMode
   );
   lua.new_simple_usertype<MaterialPass>
   (
      "MaterialPass"
      , "getActive", &MaterialPass::getActive
      , "getMaterialName", &MaterialPass::getMaterialName
      , "getMaterial", &MaterialPass::getMaterial
      , "setActive", &MaterialPass::setActive
      , "setMaterialName", &MaterialPass::setMaterialName
      , sol::base_classes, sol::bases<Serializable>()
   );

   //
   //  GE::Audio
   //
   lua.new_simple_usertype<AudioSystem>
   (
      "AudioSystem"
      , "getInstance", &AudioSystem::getInstance
      , "loadAudioBank", &AudioSystem::loadAudioBank
      , "unloadAudioBank", &AudioSystem::unloadAudioBank
   );

   // Extensions
   if(!vRegisterTypesExtensions.empty())
   {
      for(uint i = 0; i < vRegisterTypesExtensions.size(); i++)
      {
         vRegisterTypesExtensions[i](lua);
      }
   }
}
