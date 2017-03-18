
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

#include "Core/GEScript.h"
#include "Core/GEPlatform.h"
#include "Core/GEApplication.h"
#include "Core/GEDevice.h"
#include "Core/GEPhysics.h"
#include "Content/GEContentData.h"
#include "Entities/GEScene.h"
#include "Entities/GEComponentTransform.h"
#include "Entities/GEComponentSprite.h"
#include "Entities/GEComponentCamera.h"
#include "Entities/GEComponentCollider.h"
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
//  (Extensions for Lua types)
//
class luaEntity : public Entity
{
public:
   ComponentTransform* getComponentTransform() { return getComponent<ComponentTransform>(); }
   ComponentRenderable* getComponentRenderable() { return getComponent<ComponentRenderable>(); }
   ComponentSprite* getComponentSprite() { return getComponent<ComponentSprite>(); }
   ComponentCamera* getComponentCamera() { return getComponent<ComponentCamera>(); }
   ComponentCollider* getComponentCollider() { return getComponent<ComponentCollider>(); }
};


//
//  Script
//
GESTLSet(uint) Script::sDefaultGlobalNames;

Script::Script()
{
   lua.open_libraries();
   registerTypes();

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
      sDefaultGlobalNames.insert(ObjectName("scene").getID());
      sDefaultGlobalNames.insert(ObjectName("entity").getID());
   }
}

Script::~Script()
{
}

void Script::handleScriptError(const char* ScriptName)
{
   Device::log("Lua error (the '%s' script could not be loaded)", ScriptName);
}

void Script::handleFunctionError(const char* FunctionName)
{
   Device::log("Lua error ('%s' function)", FunctionName);
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

void Script::loadFromFile(const char* FileName)
{
   try
   {
      ContentData cContentData;

      if(Application::ContentType == ApplicationContentType::Xml)
      {
         Device::readContentFile(ContentType::GenericTextData, "Scripts", FileName, "lua", &cContentData);
         lua.script(cContentData.getData());
      }
      else
      {
#if defined (GE_64_BIT)
         char sFileNamex64[64];
         sprintf(sFileNamex64, "x64_%s", FileName);
         Device::readContentFile(ContentType::GenericBinaryData, "Scripts", sFileNamex64, "luabc", &cContentData);
#else
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
   }
}

ValueType Script::getVariableType(const char* VariableName) const
{
   lua_State* luaState = lua.lua_state();
   lua_getglobal(luaState, VariableName);

   int iIndex = lua_gettop(luaState);

   if(lua_isboolean(luaState, iIndex))
      return ValueType::Bool;

   if(lua_isinteger(luaState, iIndex))
      return ValueType::Int;

   if(lua_isnumber(luaState, iIndex))
      return ValueType::Float;

   if(lua_isstring(luaState, iIndex))
      return ValueType::String;
   
   return ValueType::Count;
}

bool Script::isFunctionDefined(const ObjectName& FunctionName) const
{
   for(uint i = 0; i < vGlobalFunctionNames.size(); i++)
   {
      if(vGlobalFunctionNames[i] == FunctionName)
      {
         return true;
      }
   }

   return false;
}

void Script::runFunction(const char* FunctionName)
{
   std::function<void()> luaFunction = (std::function<void()>)lua[FunctionName];

   try
   {
      luaFunction();
   }
   catch(...)
   {
      handleFunctionError(FunctionName);
   }
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
   vGlobalVariableNames.clear();
   vGlobalFunctionNames.clear();

   for(uint i = 0; i < vGlobalUserSymbols.size(); i++)
   {
      lua_getglobal(luaState, vGlobalUserSymbols[i].getString().c_str());
      
      if(lua_isfunction(luaState, lua_gettop(luaState)))
      {
         vGlobalFunctionNames.push_back(vGlobalUserSymbols[i]);
      }
      else
      {
         vGlobalVariableNames.push_back(vGlobalUserSymbols[i]);
      }
   }
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
   );
   lua.new_usertype<Vector3>
   (
      "Vector3"
      , sol::constructors<sol::types<>, sol::types<float, float, float>, sol::types<float>>()
      , "X", &Vector3::X
      , "Y", &Vector3::Y
      , "Z", &Vector3::Z
      , "getDistanceTo", &Vector3::getDistanceTo
   );
   lua.new_usertype<Rotation>
   (
      "Rotation"
      , sol::constructors<sol::types<>, sol::types<const Vector3&>, sol::types<const Vector3&, float>>()
   );

   //
   //  GE::Core
   //
   lua.new_usertype<ObjectName>
   (
      "ObjectName"
      , sol::constructors<sol::types<const char*>>()
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
   //  GE::Entities
   //
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
      , "setPosition", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::setPosition
      , "setRotation", &ComponentTransform::setRotation
      , "setOrientation", &ComponentTransform::setOrientation
      , "setScale", (void (ComponentTransform::*)(const Vector3&))&ComponentTransform::setScale
      , sol::base_classes, sol::bases<Component>()
   );
   lua.new_usertype<ComponentRenderable>
   (
      "ComponentRenderable"
      , "getMaterialPassCount", &ComponentRenderable::getMaterialPassCount
      , "getMaterialPass", &ComponentRenderable::getMaterialPass
      , "addMaterialPass", &ComponentRenderable::addMaterialPass
      , "removeMaterialPass", &ComponentRenderable::removeMaterialPass
      , sol::base_classes, sol::bases<Component>()
   );
   lua.new_usertype<ComponentSprite>
   (
      "ComponentSprite"
      , "isOver", &ComponentSprite::isOver
      , sol::base_classes, sol::bases<ComponentRenderable>()
   );
   lua.new_usertype<ComponentCamera>
   (
      "ComponentCamera"
      , "getScreenRay", &ComponentCamera::getScreenRay
      , sol::base_classes, sol::bases<Component>()
   );
   lua.new_usertype<ComponentCollider>
   (
      "ComponentCollider"
      , "checkCollision", &ComponentCollider::checkCollision
      , sol::base_classes, sol::bases<Component>()
   );
   lua.new_usertype<Entity>
   (
      "Entity"
      , "getComponentTransform", &luaEntity::getComponentTransform
      , "getComponentRenderable", &luaEntity::getComponentRenderable
      , "getComponentSprite", &luaEntity::getComponentSprite
      , "getComponentCamera", &luaEntity::getComponentCamera
      , "getComponentCollider", &luaEntity::getComponentCollider
   );
   lua.new_usertype<Scene>
   (
      "Scene"
      , "getActiveScene", &Scene::getActiveScene
      , "getEntity", &Scene::getEntity
      , "addEntity", (Entity* (Scene::*)(const ObjectName&, Entity*))&Scene::addEntity
      , "addPrefab", (Entity* (Scene::*)(const char*, const ObjectName&, Entity*))&Scene::addPrefab
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
      , "setActive", &MaterialPass::setActive
      , "getMaterialName", &MaterialPass::getMaterialName
      , sol::base_classes, sol::bases<Serializable>()
   );
}
