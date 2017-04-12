
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentScript.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentScript.h"
#include "Content/GEContentData.h"
#include "Core/GETime.h"
#include "Entities/GEScene.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Content;

const ObjectName cInitFunctionName = ObjectName("init");
const ObjectName cUpdateFunctionName = ObjectName("update");
const ObjectName cInputTouchBeginFunctionName = ObjectName("inputTouchBegin");
const ObjectName cInputTouchMoveFunctionName = ObjectName("inputTouchMove");
const ObjectName cInputTouchEndFunctionName = ObjectName("inputTouchEnd");

//
//  ScriptInstance
//
ScriptInstance::ScriptInstance()
   : SerializableArrayElement("ScriptInstance")
   , bInitialized(false)
{
   cScript = Allocator::alloc<Script>();
   GEInvokeCtor(Script, cScript);

   GERegisterProperty(ScriptInstance, String, ScriptName);

   iBasePropertiesCount = getPropertiesCount();
}

ScriptInstance::~ScriptInstance()
{
   GEInvokeDtor(Script, cScript);
   Allocator::free(cScript);
}

void ScriptInstance::setScriptName(const char* FileName)
{
   if(!FileName || strlen(FileName) == 0)
      return;

   bInitialized = false;

   cScript->reset();

   if(!cScript->loadFromFile(FileName))
      return;

   sScriptName = FileName;

   while(getPropertiesCount() > iBasePropertiesCount)
      removeProperty(getPropertiesCount() - 1);

   registerScriptProperties();
}

const char* ScriptInstance::getScriptName() const
{
   return sScriptName.c_str();
}

void ScriptInstance::registerScriptProperties()
{
   const GESTLVector(ObjectName)& vGlobalVariableNames = cScript->getGlobalVariableNames();

   for(int i = (int)(vGlobalVariableNames.size() - 1); i >= 0; i--)
   {
      const ObjectName& vGlobalVariableName = vGlobalVariableNames[i];
      const char* sGlobalVariableName = vGlobalVariableName.getString().c_str();
      ValueType ePropertyValue = cScript->getVariableType(sGlobalVariableName);

      if(ePropertyValue == ValueType::Count)
         continue;

      PropertySetter setter = [this, sGlobalVariableName](Value& cValue)
      {
         switch(cValue.getType())
         {
         case ValueType::Bool:
            cScript->setVariable<bool>(sGlobalVariableName, cValue.getAsBool());
            break;
         case ValueType::Int:
            cScript->setVariable<int>(sGlobalVariableName, cValue.getAsInt());
            break;
         case ValueType::Float:
            cScript->setVariable<float>(sGlobalVariableName, cValue.getAsFloat());
            break;
         case ValueType::String:
            cScript->setVariable<const char*>(sGlobalVariableName, cValue.getAsString());
            break;
         }
      };
      PropertyGetter getter = [this, sGlobalVariableName, ePropertyValue]() -> Value
      {
         switch(ePropertyValue)
         {
         case ValueType::Bool:
            return Value(cScript->getVariable<bool>(sGlobalVariableName));
         default:
            return Value(ePropertyValue, cScript->getVariable<const char*>(sGlobalVariableName));
         }
      };

      registerProperty(vGlobalVariableName, ePropertyValue, setter, getter);
   }

   if(cOwner)
   {
      Entity* cEntity = static_cast<ComponentScript*>(cOwner)->getOwner();
      EventArgs sEventArgs;
      sEventArgs.Sender = cEntity;
      cEntity->triggerEvent(EventPropertiesUpdated, &sEventArgs);
   }
}

void ScriptInstance::update()
{
   if(sScriptName.empty())
      return;

   cScript->setVariable<Scene*>("scene", Scene::getActiveScene());
   cScript->setVariable<float>("deltaTime", Time::getClock(0).getDelta());

   if(!bInitialized)
   {
      Entity* cEntity = static_cast<ComponentScript*>(cOwner)->getOwner();
      cScript->setVariable<Entity*>("entity", cEntity);

      if(cScript->isFunctionDefined(cInitFunctionName))
      {
         cScript->runFunction("init");
      }

      bInitialized = true;
   }

   if(cScript->isFunctionDefined(cUpdateFunctionName))
   {
      cScript->runFunction("update");
   }
}

void ScriptInstance::inputTouchBegin(int ID, const Vector2& Point)
{
   if(sScriptName.empty())
      return;

   if(!cScript->isFunctionDefined(cInputTouchBeginFunctionName))
      return;

   std::function<void(int, const Vector2&)> function =
      cScript->getFunction<void(int, const Vector2&)>(cInputTouchBeginFunctionName.getString().c_str());

   if(function)
   {
      try
      {
         function(ID, Point);
      }
      catch(...)
      {
         Script::handleFunctionError(cInputTouchBeginFunctionName.getString().c_str());
      }
   }
}

void ScriptInstance::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   if(sScriptName.empty())
      return;

   if(!cScript->isFunctionDefined(cInputTouchMoveFunctionName))
      return;

   std::function<void(int, const Vector2&, const Vector2&)> function =
      cScript->getFunction<void(int, const Vector2&, const Vector2&)>(cInputTouchMoveFunctionName.getString().c_str());

   if(function)
   {
      try
      {
         function(ID, PreviousPoint, CurrentPoint);
      }
      catch(...)
      {
         Script::handleFunctionError(cInputTouchMoveFunctionName.getString().c_str());
      }
   }
}

void ScriptInstance::inputTouchEnd(int ID, const Vector2& Point)
{
   if(sScriptName.empty())
      return;

   if(!cScript->isFunctionDefined(cInputTouchEndFunctionName))
      return;

   std::function<void(int, const Vector2&)> function =
      cScript->getFunction<void(int, const Vector2&)>(cInputTouchEndFunctionName.getString().c_str());

   if(function)
   {
      try
      {
         function(ID, Point);
      }
      catch(...)
      {
         Script::handleFunctionError(cInputTouchEndFunctionName.getString().c_str());
      }
   }
}


//
//  ComponentScript
//
ComponentScript::ComponentScript(Entity* Owner)
   : Component(Owner)
{
   cClassName = ObjectName("Script");

   GERegisterPropertyArray(ComponentScript, ScriptInstance);
}

ComponentScript::~ComponentScript()
{
   GEReleasePropertyArray(ComponentScript, ScriptInstance);
}

void ComponentScript::update()
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      getScriptInstance(i)->update();
   }
}

void ComponentScript::inputTouchBegin(int ID, const Vector2& Point)
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      getScriptInstance(i)->inputTouchBegin(ID, Point);
   }
}

void ComponentScript::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      getScriptInstance(i)->inputTouchMove(ID, PreviousPoint, CurrentPoint);
   }
}

void ComponentScript::inputTouchEnd(int ID, const Vector2& Point)
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      getScriptInstance(i)->inputTouchEnd(ID, Point);
   }
}
