
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEScript.cpp ---
//
//////////////////////////////////////////////////////////////////

#if defined _MSC_VER
# pragma warning(disable : 4503)
#endif

#include "Core/GEScript.h"
#include "Core/GEPlatform.h"
#include "Core/GEApplication.h"
#include "Core/GEDevice.h"
#include "Core/GEInterpolator.h"
#include "Core/GEPhysics.h"
#include "Content/GEContentData.h"
#include "Entities/GEScene.h"
#include "Entities/GEComponentTransform.h"
#include "Entities/GEComponentSprite.h"
#include "Entities/GEComponentParticleSystem.h"
#include "Entities/GEComponentCamera.h"
#include "Entities/GEComponentCollider.h"
#include "Entities/GEComponentUIElement.h"
#include "Entities/GEComponentScript.h"
#include "Rendering/GERenderSystem.h"
#include "Rendering/GEMaterial.h"

#if defined (GE_PLATFORM_WINDOWS)
# if defined (GE_64_BIT)
#  if defined (_DEBUG)
#   pragma comment(lib, "../../../GameEngine/Externals/lua/lib/x64/luad.lib")
#  else
#   pragma comment(lib, "../../../GameEngine/Externals/lua/lib/x64/lua.lib")
#  endif
# else
#  if defined (_DEBUG)
#   pragma comment(lib, "../../../GameEngine/Externals/lua/lib/x86/luad.lib")
#  else
#   pragma comment(lib, "../../../GameEngine/Externals/lua/lib/x86/lua.lib")
#  endif
# endif
#endif


using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Entities;
using namespace GE::Rendering;


//
//  (Global functions for Lua)
//
void luaLog(const char* sMessage)
{
   Device::log("[Lua] %s", sMessage);
}


//
//  Script
//
GESTLSet(uint) Script::sDefaultGlobalNames;

Script::Script()
{
   reset();

   if(sDefaultGlobalNames.empty())
   {
      lua_State* luaState = lua.lua_state();

      lua_pushglobaltable(luaState);
      lua_pushnil(luaState);

      while(lua_next(luaState, -2) != 0)
      {
         const char* sVariableName = lua_tostring(luaState, -2);
         ObjectName cVariableName = ObjectName(sVariableName);
         sDefaultGlobalNames.insert(cVariableName.getID());
         lua_pop(luaState, 1);
      }

      lua_pop(luaState, 1);

      // add GE variable names
      sDefaultGlobalNames.insert(ObjectName("deltaTime").getID());
      sDefaultGlobalNames.insert(ObjectName("entity").getID());
      sDefaultGlobalNames.insert(ObjectName("this").getID());
   }
}

Script::~Script()
{
}

void Script::handleScriptError(const char* ScriptName, const char* Msg)
{
   if(Msg)
   {
      Device::log("Lua error (the '%s' script could not be loaded): %s", ScriptName, Msg);
   }
   else
   {
      Device::log("Lua error (the '%s' script could not be loaded)", ScriptName);
   }
}

void Script::handleFunctionError(const char* FunctionName, const char* Msg)
{
   if(Msg)
   {
      Device::log("Lua error ('%s' function): %s", FunctionName, Msg);
   }
   else
   {
      Device::log("Lua error ('%s' function)", FunctionName);
   }
}

void Script::reset()
{
   vGlobalVariableNames.clear();
   vGlobalFunctionNames.clear();
   mFunctions.clear();

   lua = sol::state(sol::default_at_panic, customAlloc);
   lua.open_libraries();
   registerTypes();
}

void Script::loadFromCode(const GESTLString& Code)
{
   try
   {
      lua.script(Code.c_str());
      collectGlobalSymbols();
   }
   catch(...)
   {
      handleScriptError("<code>");
   }
}

bool Script::loadFromFile(const char* FileName)
{
   try
   {
      ContentData cContentData;

      if(Application::ContentType == ApplicationContentType::Xml)
      {
         if(!Device::contentFileExists("Scripts", FileName, "lua"))
            return false;

         Device::readContentFile(ContentType::GenericTextData, "Scripts", FileName, "lua", &cContentData);
#if defined (GE_EDITOR_SUPPORT)
         sol::protected_function_result luaResult = lua.do_string(cContentData.getData());

         if(!luaResult.valid())
         {
            sol::error luaError = luaResult;
            handleScriptError(FileName, luaError.what());
            return false;
         }
#else
         lua.script(cContentData.getData());
#endif
      }
      else
      {
#if defined (GE_64_BIT)
         char sFileNamex64[64];
         sprintf(sFileNamex64, "x64_%s", FileName);

         if(!Device::contentFileExists("Scripts", sFileNamex64, "luabc"))
            return false;

         Device::readContentFile(ContentType::GenericBinaryData, "Scripts", sFileNamex64, "luabc", &cContentData);
#else
         if(!Device::contentFileExists("Scripts", FileName, "luabc"))
            return false;

         Device::readContentFile(ContentType::GenericBinaryData, "Scripts", FileName, "luabc", &cContentData);
#endif
         lua_State* luaState = lua.lua_state();
         luaL_loadbuffer(luaState, cContentData.getData(), cContentData.getDataSize(), 0);
         lua_pcall(luaState, 0, LUA_MULTRET, 0);
      }

      collectGlobalSymbols();
   }
   catch(...)
   {
      handleScriptError(FileName);
      return false;
   }

   return true;
}

ValueType Script::getVariableType(const ObjectName& VariableName) const
{
   lua_State* luaState = lua.lua_state();
   lua_getglobal(luaState, VariableName.getString().c_str());

   ValueType eValueType = ValueType::Count;
   int iIndex = lua_gettop(luaState);

   if(lua_isboolean(luaState, iIndex))
      eValueType = ValueType::Bool;
   else if(lua_isinteger(luaState, iIndex))
      eValueType = ValueType::Int;
   else if(lua_isnumber(luaState, iIndex))
      eValueType = ValueType::Float;
   else if(lua_isstring(luaState, iIndex))
      eValueType = ValueType::String;
   
   lua_pop(luaState, 1);

   return eValueType;
}

bool Script::isFunctionDefined(const ObjectName& FunctionName) const
{
   return mFunctions.find(FunctionName.getID()) != mFunctions.end();
}

void* Script::customAlloc(void*, void* ptr, size_t, size_t nsize)
{
   if(nsize == 0)
   {
      if(ptr)
      {
         Allocator::free(ptr);
      }

      return 0;
   }

   return ptr
      ? Allocator::realloc<char>(ptr, (uint)nsize, AllocationCategory::Scripting)
      : Allocator::alloc<char>((uint)nsize, AllocationCategory::Scripting);
}

bool Script::alphabeticalComparison(const ObjectName& l, const ObjectName& r)
{
   return strcmp(l.getString().c_str(), r.getString().c_str()) < 0;
}

void Script::collectGlobalSymbols()
{
   // collect all global user symbols
   GESTLVector(ObjectName) vGlobalUserSymbols;

   lua_State* luaState = lua.lua_state();

   lua_pushglobaltable(luaState);
   lua_pushnil(luaState);

   while(lua_next(luaState, -2) != 0)
   {
      const char* sVariableName = lua_tostring(luaState, -2);
      ObjectName cVariableName = ObjectName(sVariableName);

      if(sDefaultGlobalNames.find(cVariableName.getID()) == sDefaultGlobalNames.end())
      {
         vGlobalUserSymbols.push_back(cVariableName);
      }

      lua_pop(luaState, 1);
   }

   lua_pop(luaState, 1);

   // fill the lists of variables and functions
   for(uint i = 0; i < vGlobalUserSymbols.size(); i++)
   {
      lua_getglobal(luaState, vGlobalUserSymbols[i].getString().c_str());
      
      if(lua_isfunction(luaState, lua_gettop(luaState)))
      {
         vGlobalFunctionNames.push_back(vGlobalUserSymbols[i]);
         mFunctions[vGlobalUserSymbols[i].getID()] = lua[vGlobalUserSymbols[i].getString().c_str()];
      }
      else
      {
         vGlobalVariableNames.push_back(vGlobalUserSymbols[i]);
      }

      lua_pop(luaState, 1);
   }

   std::sort(vGlobalVariableNames.begin(), vGlobalVariableNames.end(), alphabeticalComparison);
   std::sort(vGlobalFunctionNames.begin(), vGlobalFunctionNames.end(), alphabeticalComparison);
}

void Script::registerTypes()
{
   //
   //  Global functions
   //
   lua["log"] = luaLog;

   //
   //  GE
   //
   lua.new_usertype<Vector2>
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
   lua.new_usertype<Vector3>
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
   lua.new_usertype<Rotation>
   (
      "Rotation"
      , sol::constructors<sol::types<>, sol::types<const Vector3&>, sol::types<const Vector3&, float>>()
      , "setFromEulerAngles", &Rotation::setFromEulerAngles
      , "setFromAxisAngle", &Rotation::setFromAxisAngle
      , "setFromQuaternion", &Rotation::setFromQuaternion
   );
   lua.new_usertype<Color>
   (
      "Color"
      , sol::constructors<sol::types<>, sol::types<float, float, float, float>>()
      , "Red", &Color::Red
      , "Green", &Color::Green
      , "Blue", &Color::Blue
      , "Alpha", &Color::Alpha
   );

   //
   //  GE::Core
   //
   lua.new_usertype<ObjectName>
   (
      "ObjectName"
      , sol::constructors<sol::types<>, sol::types<const char*>>()
   );
   lua.new_usertype<Value>
   (
      "Value"
      , sol::constructors<sol::types<bool>, sol::types<int>, sol::types<float>, sol::types<const char*>>()
      , "getAsBool", &Value::getAsBool
      , "getAsInt", &Value::getAsInt
      , "getAsFloat", &Value::getAsFloat
      , "getAsString", &Value::getAsString
   );
   lua.new_usertype<Serializable>
   (
      "Serializable"
      , "get", &Serializable::get
      , "set", &Serializable::set
   );
   lua.new_enum
   (
      "InterpolationMode"
      , "Linear", InterpolationMode::Linear
      , "Quadratic", InterpolationMode::Quadratic
      , "QuadraticInverse", InterpolationMode::QuadraticInverse
      , "Logarithmic", InterpolationMode::Logarithmic
   );
   lua.new_usertype<PropertyInterpolator<float>>
   (
      "PropertyInterpolatorFloat"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &PropertyInterpolator<float>::animate
      , "update", &PropertyInterpolator<float>::update
   );
   lua.new_usertype<PropertyInterpolator<Vector2>>
   (
      "PropertyInterpolatorVector2"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &PropertyInterpolator<Vector2>::animate
      , "update", &PropertyInterpolator<Vector2>::update
   );
   lua.new_usertype<PropertyInterpolator<Vector3>>
   (
      "PropertyInterpolatorVector3"
      , sol::constructors<sol::types<Serializable*, const ObjectName&, InterpolationMode>>()
      , "animate", &PropertyInterpolator<Vector3>::animate
      , "update", &PropertyInterpolator<Vector3>::update
   );
   lua.new_usertype<Physics::Ray>
   (
      "Ray"
      , sol::constructors<sol::types<const Vector3&, const Vector3&>>()
   );
   lua.new_usertype<Physics::HitInfo>
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
   lua.new_usertype<Skeleton>
   (
      "Skeleton"
      , "getBonesCount", &Skeleton::getBonesCount
   );

   //
   //  GE::Entities
   //
   lua.new_usertype<Scene>
   (
      "Scene"
      , "getActiveScene", &Scene::getActiveScene
      , "getEntity", &Scene::getEntity
      , "addEntity", (Entity* (Scene::*)(const ObjectName&, Entity*))&Scene::addEntity
      , "addPrefab", (Entity* (Scene::*)(const char*, const ObjectName&, Entity*))&Scene::addPrefab
      , "removeEntity", (bool (Scene::*)(const ObjectName&))&Scene::removeEntity
      , sol::base_classes, sol::bases<Serializable>()
   );
   lua.new_usertype<Entity>
   (
      "Entity"
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
      , "getComponentSkeleton", &Entity::getComponent<ComponentSkeleton>
      , "getComponentScript", &Entity::getComponent<ComponentScript>
      , "getChildByIndex", &Entity::getChildByIndex
      , "getChildByName", &Entity::getChildByName
      , "init", &Entity::init
      , sol::base_classes, sol::bases<Serializable>()
   );
   lua.new_usertype<Component>
   (
      "Component"
      , "getOwner", &Component::getOwner
      , sol::base_classes, sol::bases<Serializable>()
   );
   lua.new_usertype<ComponentTransform>
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
   lua.new_usertype<ComponentRenderable>
   (
      "ComponentRenderable"
      , "getMaterialPassCount", &ComponentRenderable::getMaterialPassCount
      , "getMaterialPass", &ComponentRenderable::getMaterialPass
      , "addMaterialPass", &ComponentRenderable::addMaterialPass
      , "removeMaterialPass", &ComponentRenderable::removeMaterialPass
      , "getVisible", &ComponentRenderable::getVisible
      , "getColor", &ComponentRenderable::getColor
      , "setVisible", &ComponentRenderable::setVisible
      , "setColor", &ComponentRenderable::setColor
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_usertype<ComponentSprite>
   (
      "ComponentSprite"
      , "isOver", &ComponentSprite::isOver
      , "getTextureAtlasIndex", &ComponentSprite::getTextureAtlasIndex
      , "setTextureAtlasIndex", &ComponentSprite::setTextureAtlasIndex
      , "setTextureAtlasName", &ComponentSprite::setTextureAtlasName
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   lua.new_usertype<ComponentLabel>
   (
      "ComponentLabel"
      , "setText", &ComponentLabel::setText
      , "setStringID", &ComponentLabel::setStringID
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   lua.new_usertype<ComponentMesh>
   (
      "ComponentMesh"
      , "getMeshName", &ComponentMesh::getMeshName
      , "setMeshName", &ComponentMesh::setMeshName
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   lua.new_usertype<ComponentParticleSystem>
   (
      "ComponentParticleSystem"
      , "getEmitterActive", &ComponentParticleSystem::getEmitterActive
      , "setEmitterActive", &ComponentParticleSystem::setEmitterActive
      , "burst", &ComponentParticleSystem::burst
      , sol::base_classes, sol::bases<ComponentRenderable, Component, Serializable>()
   );
   lua.new_usertype<ComponentCamera>
   (
      "ComponentCamera"
      , "getScreenRay", &ComponentCamera::getScreenRay
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_usertype<ComponentCollider>
   (
      "ComponentCollider"
      , "checkCollision", &ComponentCollider::checkCollision
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_usertype<ComponentUIElement>
   (
      "ComponentUIElement"
      , "getAlpha", &ComponentUIElement::getAlpha
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_usertype<ComponentSkeleton>
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
   lua.new_usertype<AnimationPlayInfo>
   (
      "AnimationPlayInfo"
      , "AnimationName", &AnimationPlayInfo::AnimationName
      , "Mode", &AnimationPlayInfo::Mode
      , "BlendTime", &AnimationPlayInfo::BlendTime
      , "Speed", &AnimationPlayInfo::Speed
   );
   lua.new_usertype<ComponentScript>
   (
      "ComponentScript"
      , "getScriptInstanceCount", &ComponentScript::getScriptInstanceCount
      , "getScriptInstance", &ComponentScript::getScriptInstance
      , "addScriptInstance", &ComponentScript::addScriptInstance
      , "removeScriptInstance", &ComponentScript::removeScriptInstance
      , sol::base_classes, sol::bases<Component, Serializable>()
   );
   lua.new_usertype<ScriptInstance>
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
   lua.new_usertype<RenderSystem>
   (
      "RenderSystem"
      , "getInstance", &RenderSystem::getInstance
      , "getActiveCamera", &RenderSystem::getActiveCamera
   );
   lua.new_usertype<MaterialPass>
   (
      "MaterialPass"
      , "getActive", &MaterialPass::getActive
      , "getMaterialName", &MaterialPass::getMaterialName
      , "setActive", &MaterialPass::setActive
      , "setMaterialName", &MaterialPass::setMaterialName
      , sol::base_classes, sol::bases<Serializable>()
   );
}
