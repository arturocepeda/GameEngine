
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
#include "Content/GELocalizedString.h"
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


GESTLSet(uint32_t) gPredefinedGlobalSymbols;


//
//  Namespace
//
Namespace::Namespace()
   : mState(0)
   , mParent(0)
{
}

Namespace::Namespace(State* pState, const char* pName)
   : mName(pName)
   , mState(pState)
   , mParent(0)
{
}

Namespace::~Namespace()
{
}

void Namespace::load(Namespace* pParent)
{
   mParent = pParent;
   mTable = pParent ? pParent->mTable[mName.c_str()] : mState->globals();

   collectSymbols();
}

void Namespace::unload()
{
   mTable = sol::lua_nil;
   mParent = 0;
}

Namespace* Namespace::addNamespace(const ObjectName& pName)
{
    mTable.create(pName.getString());

    mNamespaces[pName.getID()] = Namespace(mState, pName.getString());
    Namespace* ns = &mNamespaces.find(pName.getID())->second;
    ns->load(this);

    return ns;
}

Namespace* Namespace::addNamespaceFromModule(const ObjectName& pName, const char* pModuleName)
{
   ContentData contentData;

   if(!Environment::loadModuleContent(&contentData, pModuleName))
      return 0;

   const uint32_t codeSize = Application::ContentType == ApplicationContentType::Xml
      ? contentData.getDataSize() - 1
      : contentData.getDataSize();

   {
      sol::load_result loadFunction = mState->load_buffer(contentData.getData(), codeSize, pModuleName);

      if(loadFunction.valid())
      {
         sol::object moduleContent = loadFunction();

         if(moduleContent.get_type() == sol::type::table)
         {
            mTable[pName.getString()] = moduleContent;
         }
         else
         {
            Environment::handleScriptError(pModuleName, "does not return any valid module");
            return 0;
         }
      }
      else
      {
         sol::error luaError = loadFunction;
         Environment::handleScriptError(pModuleName, luaError.what());
         return 0;
      }
   }

   mNamespaces[pName.getID()] = Namespace(mState, pName.getString());
   Namespace* ns = &mNamespaces.find(pName.getID())->second;
   ns->load(this);

   return ns;
}

Namespace* Namespace::getNamespace(const ObjectName& pName)
{
   GESTLMap(uint32_t, Namespace)::iterator it = mNamespaces.find(pName.getID());
   return it != mNamespaces.end() ? &it->second : 0;
}

void Namespace::removeNamespace(const ObjectName& pName)
{
   GESTLMap(uint32_t, Namespace)::iterator it = mNamespaces.find(pName.getID());

   if(it != mNamespaces.end())
   {
      mTable[pName.getString()] = sol::lua_nil;
      mNamespaces.erase(it);
   }
}

ValueType Namespace::getVariableType(const ObjectName& pVariableName) const
{
   lua_State* luaState = mState->lua_state();
   sol::object variableRef = mTable[pVariableName.getString()];
   variableRef.push();

   ValueType valueType = ValueType::Count;
   bool userDataType = false;
   const int index = lua_gettop(luaState);

   if(lua_isboolean(luaState, index))
      valueType = ValueType::Bool;
   else if(lua_isinteger(luaState, index))
      valueType = ValueType::Int;
   else if(lua_isnumber(luaState, index))
      valueType = ValueType::Float;
   else if(lua_isstring(luaState, index))
      valueType = ValueType::String;
   else if(lua_isuserdata(luaState, index))
      userDataType = true;
   
   lua_pop(luaState, 1);

   if(userDataType)
   {
      if(variableRef.is<Vector3>())
         valueType = ValueType::Vector3;
      else if(variableRef.is<Vector2>())
         valueType = ValueType::Vector2;
      else if(variableRef.is<Color>())
         valueType = ValueType::Color;
   }

   return valueType;
}

bool Namespace::isFunctionDefined(const ObjectName& pFunctionName) const
{
   return mFunctions.find(pFunctionName.getID()) != mFunctions.end();
}

uint32_t Namespace::getFunctionParametersCount(const ObjectName& FunctionName) const
{
   lua_State* luaState = mState->lua_state();
   lua_Debug luaDebug;

   Function function = mTable[FunctionName.getString()];
   function.push();

   lua_getinfo(luaState, ">u", &luaDebug);

   return (uint32_t)luaDebug.nparams;
}

bool Namespace::alphabeticalComparison(const ObjectName& pL, const ObjectName& pR)
{
   return strcmp(pL.getString(), pR.getString()) < 0;
}

void Namespace::collectSymbols()
{
   GEProfilerMarker("Namespace::collectSymbols()");

   mGlobalVariableNames.clear();
   mGlobalFunctionNames.clear();
   mFunctions.clear();
   mNamespaces.clear();

   GESTLVector(ObjectName) tableNames;

   lua_State* luaState = mState->lua_state();

   mTable.push();
   lua_pushnil(luaState);

   while(lua_next(luaState, -2))
   {
      if(lua_type(luaState, -2) == LUA_TSTRING)
      {
         const char* symbolString = lua_tostring(luaState, -2);
         const ObjectName symbolName = ObjectName(symbolString);

         if(gPredefinedGlobalSymbols.find(symbolName.getID()) == gPredefinedGlobalSymbols.end())
         {
            const int stackTop = lua_gettop(luaState);

            if(lua_istable(luaState, stackTop))
            {
               tableNames.push_back(symbolName);
            }
            else if(lua_isfunction(luaState, stackTop))
            {
               mGlobalFunctionNames.push_back(symbolName);
               mFunctions[symbolName.getID()] = mTable[symbolString];
            }
            else
            {
               mGlobalVariableNames.push_back(symbolName);
            }
         }
      }

      lua_pop(luaState, 1);
   }

   lua_pop(luaState, 1);

   if(!tableNames.empty())
   {
      for(size_t i = 0; i < tableNames.size(); i++)
      {
         const ObjectName& tableName = tableNames[i];
         mNamespaces[tableName.getID()] = Namespace(mState, tableName.getString());
         Namespace* ns = &mNamespaces.find(tableName.getID())->second;
         ns->load(this);
      }
   }

#if defined (GE_EDITOR_SUPPORT)
   std::sort(mGlobalVariableNames.begin(), mGlobalVariableNames.end(), alphabeticalComparison);
   std::sort(mGlobalFunctionNames.begin(), mGlobalFunctionNames.end(), alphabeticalComparison);
#endif
}



//
//  Environment
//
const size_t MemoryPoolSize = 16u * 1024u * 1024u;

void* Environment::smAllocatorBuffer = 0;
tlsf_t Environment::smAllocator = 0;
GEMutex mAllocatorMutex;

GESTLVector(Environment::registerTypesExtension) Environment::smRegisterTypesExtensions;

Environment::Environment()
   : mLua(sol::default_at_panic, customAlloc)
   , mGlobalNamespace(&mLua, "_G")
#if defined (GE_EDITOR_SUPPORT)
   , mDebugBreakpointLine(0)
   , mDebuggerActive(false)
#endif
{
   mLua.open_libraries();
   registerTypes();

   lua_gc(mLua.lua_state(), LUA_GCSETPAUSE, 200);
   lua_gc(mLua.lua_state(), LUA_GCSETSTEPMUL, 200);
   lua_gc(mLua.lua_state(), LUA_GCRESTART, 0);
}

Environment::~Environment()
{
}

void Environment::initStaticData()
{
   GEAssert(!smAllocatorBuffer);
   GEAssert(!smAllocator);

   // initialize allocator
   smAllocatorBuffer = Allocator::alloc<char>(MemoryPoolSize, AllocationCategory::Scripting);
   smAllocator = tlsf_create_with_pool(smAllocatorBuffer, MemoryPoolSize);
   GEMutexInit(mAllocatorMutex);

   // collect predefined global symbols
   collectPredefinedGlobalSymbols();
}

void Environment::releaseStaticData()
{
   // clear predefined global symbols
   gPredefinedGlobalSymbols.clear();

   // release allocator
   GEMutexDestroy(mAllocatorMutex);
   tlsf_destroy(smAllocator);
   smAllocator = 0;
   Allocator::free(smAllocatorBuffer);
   smAllocatorBuffer = 0;
}

void Environment::addRegisterTypesExtension(registerTypesExtension pExtension)
{
   smRegisterTypesExtensions.push_back(pExtension);
}

bool Environment::loadModuleContent(ContentData* pContentData, const char* pModuleName)
{
   if(Application::ContentType == ApplicationContentType::Xml)
   {
      if(!Device::contentFileExists("Scripts", pModuleName, "lua"))
         return false;

      Device::readContentFile(ContentType::GenericTextData, "Scripts", pModuleName, "lua", pContentData);
   }
   else
   {
#if defined (GE_64_BIT)
      char sFileNamex64[64];
      sprintf(sFileNamex64, "x64_%s", pModuleName);

      if(!Device::contentFileExists("Scripts", sFileNamex64, "luabc"))
         return false;

      Device::readContentFile(ContentType::GenericBinaryData, "Scripts", sFileNamex64, "luabc", pContentData);
#else
      if(!Device::contentFileExists("Scripts", pModuleName, "luabc"))
         return false;

      Device::readContentFile(ContentType::GenericBinaryData, "Scripts", pModuleName, "luabc", pContentData);
#endif
   }

   return true;
}

bool Environment::loadModule(State* pState, const char* pModuleName, Table* pOutReturnValue)
{
   GEProfilerMarker("Environment::loadModule()");

   ContentData contentData;

   if(!loadModuleContent(&contentData, pModuleName))
      return false;

   const uint32_t codeSize = Application::ContentType == ApplicationContentType::Xml
      ? contentData.getDataSize() - 1
      : contentData.getDataSize();

   sol::load_result loadFunction = pState->load_buffer(contentData.getData(), codeSize, pModuleName);

   if(!loadFunction.valid())
   {
      sol::error luaError = loadFunction;
      handleScriptError(pModuleName, luaError.what());
      return false;
   }

   sol::object moduleContent = loadFunction();

   if(moduleContent.get_type() == sol::type::table)
   {
      *pOutReturnValue = moduleContent;
   }
   else
   {
      handleScriptError(pModuleName, "does not return any valid module");
      return false;
   }

   return true;
}

void Environment::handleScriptError(const char* pScriptName, const char* pMsg)
{
   if(pMsg)
   {
      Log::log(LogType::Error, "Lua error (the '%s' script could not be loaded): %s", pScriptName, pMsg);
   }
   else
   {
      Log::log(LogType::Error, "Lua error (the '%s' script could not be loaded)", pScriptName);
   }
}

void Environment::handleFunctionError(const char* pFunctionName, const char* pMsg)
{
   if(pMsg)
   {
      Log::log(LogType::Error, "Lua error ('%s' function): %s", pFunctionName, pMsg);
   }
   else
   {
      Log::log(LogType::Error, "Lua error ('%s' function)", pFunctionName);
   }
}

struct LuaDump
{
   char* mPtr;
   size_t mSize;
   size_t mMaxSize;
};

int luaDumpWriter(lua_State* pLua, const void* pPtr, size_t pSize, void* pUserData)
{
   LuaDump* dump = static_cast<LuaDump*>(pUserData);
   size_t nextBytecodeLength = dump->mSize + pSize;

   if(nextBytecodeLength > dump->mMaxSize)
      return -1;

   memcpy(dump->mPtr + dump->mSize, pPtr, pSize);
   dump->mSize = nextBytecodeLength;
   return 0;
}

size_t Environment::compileScript(const char* pCode, size_t pBytecodeMaxSize, char* pOutBytecode)
{
   LuaDump dump;
   dump.mPtr = pOutBytecode;
   dump.mSize = 0u;
   dump.mMaxSize = pBytecodeMaxSize;

   lua_State* lua = luaL_newstate();
   luaL_loadstring(lua, pCode);
   int result = lua_dump(lua, luaDumpWriter, &dump, 1);
   lua_close(lua);

   return result == 0 ? dump.mSize : 0u;
}

void Environment::load()
{
   mGlobalNamespace.load(0);
}

void Environment::collectGarbage()
{
   lua_gc(mLua.lua_state(), LUA_GCCOLLECT, 0);
}

void Environment::collectGarbageStep()
{
   lua_gc(mLua.lua_state(), LUA_GCSTEP, 0);
}

void Environment::loadFromCode(const GESTLString& pCode)
{
   mLua.script(pCode.c_str());
   mGlobalNamespace.load(0);
}

bool Environment::loadFromFile(const char* pFileName)
{
   GEProfilerMarker("Environment::loadFromFile()");

   Table returnValue;

   if(!loadModule(&mLua, pFileName, &returnValue))
      return false;

   mGlobalNamespace.load(0);

   return true;
}

#if defined (GE_EDITOR_SUPPORT)
void Environment::setDebugBreakpointLine(uint32_t pLine)
{
   mDebugBreakpointLine = pLine;
}

void Environment::enableDebugger()
{
   mLua["debugger"] = [this](int pEvent, int pLine)
   {
      if(!mDebuggerActive)
      {
         if(mDebugBreakpointLine == 0 || mDebugBreakpointLine == (uint32_t)pLine)
         {
            mDebuggerActive = true;
         }
         else
         {
            return;
         }
      }

      std::string sCodeStr = mLua.script("return debug.getinfo(3).source");
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

         if(abs(iCurrentLine - pLine) < 10)
         {
            printf("%c%4d: %s\n", iCurrentLine == pLine ? '>' : ' ', iCurrentLine, sCodeLine);
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
            mLua.script(sCommandBuffer);
            bContinue = false;
         }
      }
      while(!bContinue);
   };
   mLua.script("debug.sethook(debugger, \"l\")");
}

void Environment::disableDebugger()
{
   mLua.script("debug.sethook()");
   mLua["debugger"] = nullptr;
   mDebuggerActive = false;
}
#endif

void* Environment::customAlloc(void*, void* pPtr, size_t, size_t pNewSize)
{
   GEMutexLock(mAllocatorMutex);

   if(pNewSize == 0)
   {
      if(pPtr)
      {
         tlsf_free(smAllocator, pPtr);
      }

      GEMutexUnlock(mAllocatorMutex);

      return 0;
   }

   void* pMemoryBlock = pPtr
      ? tlsf_realloc(smAllocator, pPtr, pNewSize)
      : tlsf_malloc(smAllocator, pNewSize);

   GEMutexUnlock(mAllocatorMutex);

   return pMemoryBlock;
}

void Environment::collectPredefinedGlobalSymbols()
{
   gPredefinedGlobalSymbols.clear();

   Environment cDefaultEnv;
   lua_State* luaState = cDefaultEnv.mLua.lua_state();

   lua_pushglobaltable(luaState);
   lua_pushnil(luaState);

   while(lua_next(luaState, -2))
   {
      const char* sVariableName = lua_tostring(luaState, -2);
      const ObjectName cVariableName = ObjectName(sVariableName);
      addPredefinedGlobalSymbol(cVariableName);
      lua_pop(luaState, 1);
   }

   lua_pop(luaState, 1);
}

void Environment::addPredefinedGlobalSymbol(const ObjectName& pSymbol)
{
   gPredefinedGlobalSymbols.insert(pSymbol.getID());
}

void Environment::registerTypes()
{
   GEProfilerMarker("Environment::registerTypes()");

   //
   //  Module loading
   //
   mLua["packageSearcher"] = [this](const char* sModuleName)
   {
      char moduleLoaderName[64];
      sprintf(moduleLoaderName, "%sLoader", sModuleName);
      mLua[moduleLoaderName] = [this, sModuleName]() -> Table
      {
         Table returnValue;
         loadModule(&mLua, sModuleName, &returnValue);
         return returnValue;
      };

      char codeBuffer[256];
      sprintf(codeBuffer, "package.preload['%s'] = %s", sModuleName, moduleLoaderName);
      mLua.script(codeBuffer);
   };
   mLua.script("table.insert(package.searchers, 1, packageSearcher)");

   //
   //  Global functions
   //
   mLua["logInfo"] = [](const char* sMessage){ Log::log(LogType::Info, "[Lua] %s", sMessage); };
   mLua["logWarning"] = [](const char* sMessage){ Log::log(LogType::Warning, "[Lua] %s", sMessage); };
   mLua["logError"] = [](const char* sMessage){ Log::log(LogType::Error, "[Lua] %s", sMessage); };

   //
   //  GE
   //
   mLua.new_simple_usertype<Vector2>
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
   mLua.new_simple_usertype<Vector3>
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
   mLua.new_simple_usertype<Rotation>
   (
      "Rotation"
      , sol::constructors<sol::types<>, sol::types<const Vector3&>, sol::types<const Vector3&, float>>()
      , "setFromEulerAngles", &Rotation::setFromEulerAngles
      , "setFromAxisAngle", &Rotation::setFromAxisAngle
      , "setFromQuaternion", &Rotation::setFromQuaternion
   );
   mLua.new_simple_usertype<Color>
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
   mLua.new_simple_usertype<ObjectName>
   (
      "ObjectName"
      , sol::constructors<sol::types<>, sol::types<const char*>>()
      , "getID", &ObjectName::getID
      , "getString", &ObjectName::getString
      , "isEmpty", &ObjectName::isEmpty
      , sol::meta_method::equal_to, &ObjectName::operator==
   );
   mLua.new_simple_usertype<Value>
   (
      "Value"
      , sol::constructors<sol::types<bool>, sol::types<int>, sol::types<float>, sol::types<const char*>,
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
   mLua.new_simple_usertype<Serializable>
   (
      "Serializable"
      , "is", &Serializable::is
      , "has", &Serializable::has
      , "get", &Serializable::get
      , "set", &Serializable::set
      , "executeAction", &Serializable::executeAction
   );
   mLua.new_simple_usertype<Clock>
   (
      "Clock"
      , "getDelta", &Clock::getDelta
   );
   mLua.new_enum
   (
      "InterpolationMode"
      , "Linear", InterpolationMode::Linear
      , "Quadratic", InterpolationMode::Quadratic
      , "QuadraticInverse", InterpolationMode::QuadraticInverse
      , "Cubic", InterpolationMode::Cubic
      , "CubicInverse", InterpolationMode::CubicInverse
      , "Quartic", InterpolationMode::Quartic
      , "QuarticInverse", InterpolationMode::QuarticInverse
      , "Quintic", InterpolationMode::Quintic
      , "QuinticInverse", InterpolationMode::QuinticInverse
      , "Logarithmic", InterpolationMode::Logarithmic
   );
   mLua.new_simple_usertype<PropertyInterpolator<float>>
   (
      "PropertyInterpolatorFloat"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "setClockName", &PropertyInterpolator<float>::setClockName
      , "animate", &PropertyInterpolator<float>::animate
      , "alternate", &PropertyInterpolator<float>::alternate
      , "stopOnStartValue", &PropertyInterpolator<float>::stopOnStartValue
      , "update", &PropertyInterpolator<float>::update
      , "getActive", &PropertyInterpolator<float>::getActive
   );
   mLua.new_simple_usertype<PropertyInterpolator<Vector2>>
   (
      "PropertyInterpolatorVector2"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "setClockName", &PropertyInterpolator<Vector2>::setClockName
      , "animate", &PropertyInterpolator<Vector2>::animate
      , "alternate", &PropertyInterpolator<Vector2>::alternate
      , "stopOnStartValue", &PropertyInterpolator<Vector2>::stopOnStartValue
      , "update", &PropertyInterpolator<Vector2>::update
      , "getActive", &PropertyInterpolator<Vector2>::getActive
   );
   mLua.new_simple_usertype<PropertyInterpolator<Vector3>>
   (
      "PropertyInterpolatorVector3"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "setClockName", &PropertyInterpolator<Vector3>::setClockName
      , "animate", &PropertyInterpolator<Vector3>::animate
      , "alternate", &PropertyInterpolator<Vector3>::alternate
      , "stopOnStartValue", &PropertyInterpolator<Vector3>::stopOnStartValue
      , "update", &PropertyInterpolator<Vector3>::update
      , "getActive", &PropertyInterpolator<Vector3>::getActive
   );
   mLua.new_simple_usertype<PropertyInterpolator<Color>>
   (
      "PropertyInterpolatorColor"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "setClockName", &PropertyInterpolator<Color>::setClockName
      , "animate", &PropertyInterpolator<Color>::animate
      , "alternate", &PropertyInterpolator<Color>::alternate
      , "stopOnStartValue", &PropertyInterpolator<Color>::stopOnStartValue
      , "update", &PropertyInterpolator<Color>::update
      , "getActive", &PropertyInterpolator<Color>::getActive
   );
   mLua.new_simple_usertype<CurveKey>
   (
      "CurveKey"
      , "getTimePosition", &CurveKey::getTimePosition
      , "getValue", &CurveKey::getValue
      , "setTimePosition", &CurveKey::setTimePosition
      , "setValue", &CurveKey::setValue
   );
   mLua.new_simple_usertype<Curve>
   (
      "Curve"
      , "getValue", &Curve::getValue
      , "getCurveKeyCount", &Curve::getCurveKeyCount
      , "getCurveKey", &Curve::getCurveKey
   );
   mLua.new_enum
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
   mLua.new_simple_usertype<CurvePropertyInterpolator>
   (
      "CurvePropertyInterpolator"
      , sol::constructors<sol::types<Curve*, Serializable*, const ObjectName&, PropertyValueComponent>>()
      , "setClockName", &CurvePropertyInterpolator::setClockName
      , "animate", &CurvePropertyInterpolator::animate
      , "animateInverse", &CurvePropertyInterpolator::animateInverse
      , "update", &CurvePropertyInterpolator::update
      , "getActive", &CurvePropertyInterpolator::getActive
   );
   mLua.new_simple_usertype<BezierCurve>
   (
      "BezierCurve"
      , "getPoint", &BezierCurve::getPoint
   );
   mLua.new_simple_usertype<BezierPropertyInterpolator>
   (
      "BezierPropertyInterpolator"
      , sol::constructors<sol::types<BezierCurve*, Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &BezierPropertyInterpolator::animate
      , "animateInverse", &BezierPropertyInterpolator::animateInverse
      , "update", &BezierPropertyInterpolator::update
      , "getActive", &BezierPropertyInterpolator::getActive
   );
   mLua.new_simple_usertype<Physics::Ray>
   (
      "Ray"
      , sol::constructors<sol::types<const Vector3&, const Vector3&>>()
   );
   mLua.new_simple_usertype<Physics::HitInfo>
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
   mLua.new_simple_usertype<Skeleton>
   (
      "Skeleton"
      , "getBonesCount", &Skeleton::getBonesCount
   );
   mLua.new_simple_usertype<LocalizedStringsManager>
   (
      "LocalizedStringsManager"
      , "getInstance", &LocalizedStringsManager::getInstance
      , "setVariable", &LocalizedStringsManager::setVariable
      , "getVariable", &LocalizedStringsManager::getVariable
   );

   //
   //  GE::Input
   //
   mLua.new_simple_usertype<InputSystem>
   (
      "InputSystem"
      , "getInstance", &InputSystem::getInstance
      , "setInputEnabled", &InputSystem::setInputEnabled
      , "getMousePosition", &InputSystem::getMousePosition
   );

   //
   //  GE::Entities
   //
   mLua.new_simple_usertype<Scene>
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
   mLua.new_simple_usertype<Entity>
   (
      "Entity"
      , "getName", &Entity::getName
      , "getFullName", &Entity::getFullName
      , "getActive", &Entity::getActive
      , "setActive", &Entity::setActive
      , "getClock", &Entity::getClock
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
   mLua.new_simple_usertype<Component>
   (
      "Component"
      , "getOwner", &Component::getOwner
      , sol::base_classes, sol::bases<Serializable>()
   );
   mLua.new_simple_usertype<ComponentTransform>
   (
      "ComponentTransform"
      , "move", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::move
      , "rotate", &ComponentTransform::rotate
      , "scale", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::scale
      , "getPosition", &ComponentTransform::getPosition
      , "getWorldPosition", &ComponentTransform::getWorldPosition
      , "getRotation", &ComponentTransform::getRotation
      , "getWorldRotation", &ComponentTransform::getWorldRotation
      , "getOrientation", &ComponentTransform::getOrientation
      , "getWorldOrientation", &ComponentTransform::getWorldOrientation
      , "getScale", &ComponentTransform::getScale
      , "getWorldScale", &ComponentTransform::getWorldScale
      , "getForwardVector", &ComponentTransform::getForwardVector
      , "getRightVector", &ComponentTransform::getRightVector
      , "getUpVector", &ComponentTransform::getUpVector
      , "setPosition", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::setPosition
      , "setRotation", &ComponentTransform::setRotation
      , "setOrientation", &ComponentTransform::setOrientation
      , "setScale", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::setScale
      , "setWorldPosition", &ComponentTransform::setWorldPosition
      , "setForwardVector", &ComponentTransform::setForwardVector
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentRenderable>
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
   mLua.new_simple_usertype<ComponentSprite>
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
   mLua.new_simple_usertype<ComponentLabel>
   (
      "ComponentLabel"
      , "getText", &ComponentLabel::getText
      , "getStringID", &ComponentLabel::getStringID
      , "getCharacterCountLimit", &ComponentLabel::getCharacterCountLimit
      , "getTextLength", &ComponentLabel::getTextLength
      , "setText", &ComponentLabel::setText
      , "setStringID", &ComponentLabel::setStringID
      , "setCharacterCountLimit", &ComponentLabel::setCharacterCountLimit
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentMesh>
   (
      "ComponentMesh"
      , "getMeshName", &ComponentMesh::getMeshName
      , "setMeshName", &ComponentMesh::setMeshName
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentParticleSystem>
   (
      "ComponentParticleSystem"
      , "getEmitterActive", &ComponentParticleSystem::getEmitterActive
      , "setEmitterActive", &ComponentParticleSystem::setEmitterActive
      , "burst", &ComponentParticleSystem::burst
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentCamera>
   (
      "ComponentCamera"
      , "getScreenRay", &ComponentCamera::getScreenRay
      , "worldToScreen", &ComponentCamera::worldToScreen
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentCollider>
   (
      "ComponentCollider"
      , "checkCollision", &ComponentCollider::checkCollision
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentUIElement>
   (
      "ComponentUIElement"
      , "getAlpha", &ComponentUIElement::getAlpha
      , "setAlpha", &ComponentUIElement::setAlpha
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentUI2DElement>
   (
      "ComponentUI2DElement"
      , "getOffset", &ComponentUI2DElement::getOffset
      , "getScaledYOffset", &ComponentUI2DElement::getScaledYOffset
      , "setOffset", &ComponentUI2DElement::setOffset
      , "setScaledYOffset", &ComponentUI2DElement::setScaledYOffset
      , sol::base_classes, sol::bases<ComponentUIElement, Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentUI3DElement>
   (
      "ComponentUI3DElement"
      , "getCanvasIndex", &ComponentUI3DElement::getCanvasIndex
      , "setCanvasIndex", &ComponentUI3DElement::setCanvasIndex
      , sol::base_classes, sol::bases<ComponentUIElement, Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentSkeleton>
   (
      "ComponentSkeleton"
      , "getSkeleton", &ComponentSkeleton::getSkeleton
      , "getBoneEntity", (Entity* (ComponentSkeleton::*)(const ObjectName&) const)&ComponentSkeleton::getBoneEntity
      , "getBoneEntityByIndex", (Entity* (ComponentSkeleton::*)(uint) const)&ComponentSkeleton::getBoneEntity
      , "playAnimation", &ComponentSkeleton::playAnimation
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   mLua.new_enum
   (
      "AnimationPlayMode"
      , "Loop", AnimationPlayMode::Loop
      , "Once", AnimationPlayMode::Once
   );
   mLua.new_simple_usertype<AnimationPlayInfo>
   (
      "AnimationPlayInfo"
      , "AnimationName", &AnimationPlayInfo::AnimationName
      , "Mode", &AnimationPlayInfo::Mode
      , "BlendTime", &AnimationPlayInfo::BlendTime
      , "Speed", &AnimationPlayInfo::Speed
   );
   mLua.new_simple_usertype<ComponentDataContainer>
   (
      "ComponentDataContainer"
      , "getVariable", (const Value* (ComponentDataContainer::*)(const ObjectName&))&ComponentDataContainer::getVariable
      , "setVariable", &ComponentDataContainer::setVariable
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentAudioSource>
   (
      "ComponentAudioSource"
      , "playAudioEvent", &ComponentAudioSource::playAudioEvent
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   mLua.new_simple_usertype<ComponentScript>
   (
      "ComponentScript"
      , "getScriptInstanceCount", &ComponentScript::getScriptInstanceCount
      , "getScriptInstance", &ComponentScript::getScriptInstance
      , "getScriptInstanceByName", &ComponentScript::getScriptInstanceByName
      , "addScriptInstance", &ComponentScript::addScriptInstance
      , "removeScriptInstance", &ComponentScript::removeScriptInstance
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   mLua.new_simple_usertype<ScriptInstance>
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
   mLua.new_simple_usertype<RenderSystem>
   (
      "RenderSystem"
      , "getInstance", &RenderSystem::getInstance
      , "getActiveCamera", &RenderSystem::getActiveCamera
      , "getFPS", &RenderSystem::getFPS
   );
   mLua.new_simple_usertype<Texture>
   (
      "Texture"
      , "getAtlasSize", &Texture::getAtlasSize
      , "getAtlasName", &Texture::getAtlasName
   );
   mLua.new_simple_usertype<Material>
   (
      "Material"
      , "getShaderProgram", &Material::getShaderProgram
      , "getDiffuseColor", &Material::getDiffuseColor
      , "getDiffuseTexture", &Material::getDiffuseTexture
      , "getBlendingMode", &Material::getBlendingMode
   );
   mLua.new_simple_usertype<MaterialPass>
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
   mLua.new_simple_usertype<AudioEventInstance>
   (
      "AudioEventInstance"
   );
   mLua.new_simple_usertype<AudioSystem>
   (
      "AudioSystem"
      , "getInstance", &AudioSystem::getInstance
      , "loadAudioBank", &AudioSystem::loadAudioBank
      , "unloadAudioBank", &AudioSystem::unloadAudioBank
      , "playAudioEvent", &AudioSystem::playAudioEvent
      , "setPosition", &AudioSystem::setPosition
   );

   // Extensions
   if(!smRegisterTypesExtensions.empty())
   {
      for(uint i = 0; i < smRegisterTypesExtensions.size(); i++)
      {
         smRegisterTypesExtensions[i](mLua);
      }
   }
}
