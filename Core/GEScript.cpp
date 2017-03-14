
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
#include "Content/GEContentData.h"
#include "Entities/GEScene.h"
#include "Entities/GEComponentTransform.h"
#include "Entities/GEComponentSprite.h"

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


//
//  (Extensions for lua types)
//
class luaEntity : public Entity
{
public:
   ComponentTransform* getComponentTransform() { return getComponent<ComponentTransform>(); }
   ComponentSprite* getComponentSprite() { return getComponent<ComponentSprite>(); }
};


//
//  Script
//
Script::Script()
{
   lua.open_libraries();
   registerTypes();
}

Script::~Script()
{
}

void Script::loadFromCode(const GESTLString& Code)
{
   lua.script(Code.c_str());
}

void Script::loadFromFile(const char* FileName)
{
   ContentData cContentData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Scripts", FileName, "lua", &cContentData);
      lua.script(cContentData.getData());
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Scripts", FileName, "luabc", &cContentData);
      lua_State* luaState = lua.lua_state();
      luaL_loadbuffer(luaState, cContentData.getData(), cContentData.getDataSize(), 0);
      lua_pcall(luaState, 0, LUA_MULTRET, 0);
   }
}

void Script::setVariableInt(const char* VariableName, int Value)
{
   lua[VariableName] = Value;
}

void Script::setVariableFloat(const char* VariableName, float Value)
{
   lua[VariableName] = Value;
}

bool Script::isFunctionDefined(const char* FunctionName) const
{
   lua_getglobal(lua, FunctionName);
   return lua_isfunction(lua, lua_gettop(lua));
}

void Script::runFunction(const char* FunctionName)
{
   std::function<void()> luaFunction = (std::function<void()>)lua[FunctionName];
   luaFunction();
}

void Script::registerTypes()
{
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

   //
   //  GE::Entities
   //
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
   );
   lua.new_usertype<ComponentSprite>
   (
      "ComponentSprite"
      , "isOver", &ComponentSprite::isOver
      , sol::base_classes, sol::bases<ComponentRenderable>()
   );
   lua.new_usertype<Entity>
   (
      "Entity"
      , "getComponentTransform", &luaEntity::getComponentTransform
      , "getComponentSprite", &luaEntity::getComponentSprite
   );
   lua.new_usertype<Scene>
   (
      "Scene"
      , "getActiveScene", &Scene::getActiveScene
      , "getEntity", &Scene::getEntity
      , "addEntity", (Entity* (Scene::*)(const ObjectName&, Entity*))&Scene::addEntity
      , "addPrefab", (Entity* (Scene::*)(const char*, const ObjectName&, Entity*))&Scene::addPrefab
   );
}
