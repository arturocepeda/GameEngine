
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
#include "Core/GEScript.h"
#include "Core/GETime.h"
#include "Core/GEEvents.h"
#include "Entities/GEScene.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Content;

const ObjectName cInitFunctionName = ObjectName("init");
const ObjectName cActivateFunctionName = ObjectName("activate");
const ObjectName cDeactivateFunctionName = ObjectName("deactivate");
const ObjectName cUpdateFunctionName = ObjectName("update");
const ObjectName cInputTouchBeginFunctionName = ObjectName("inputTouchBegin");
const ObjectName cInputTouchMoveFunctionName = ObjectName("inputTouchMove");
const ObjectName cInputTouchEndFunctionName = ObjectName("inputTouchEnd");

const ObjectName* cInternalFunctionNames[] =
{
   &cInitFunctionName,
   &cActivateFunctionName,
   &cDeactivateFunctionName,
   &cUpdateFunctionName,
   &cInputTouchBeginFunctionName,
   &cInputTouchMoveFunctionName,
   &cInputTouchEndFunctionName,
};
const uint iInternalFuncionNamesCount = sizeof(cInternalFunctionNames) / sizeof(ObjectName*);

//
//  ScriptInstance
//
ScriptInstance::ScriptInstance()
   : SerializableArrayElement("ScriptInstance")
   , bActive(true)
   , bInitialized(false)
{
   cScript = Allocator::alloc<Script>();
   GEInvokeCtor(Script, cScript);

   GERegisterPropertySpecialEditor(ObjectName, ScriptName, PropertyEditor::Script);
   GERegisterProperty(Bool, Active);

#if defined (GE_EDITOR_SUPPORT)
   registerAction("Reload", [this]
   {
      const uint iNumProperties = getPropertiesCount() - iBasePropertiesCount;
      vCachedPropertyValues.resize(iNumProperties);

      for(uint i = 0; i < iNumProperties; i++)
      {
         const Property& sProperty = getProperty(i + iBasePropertiesCount);
         vCachedPropertyValues[i].PropertyNameHash = sProperty.Name.getID();
         vCachedPropertyValues[i].PropertyValue = sProperty.Getter();
      }

      setScriptName(getScriptName());
   });
#endif

   iBasePropertiesCount = getPropertiesCount();
   iBaseActionsCount = getActionsCount();
}

ScriptInstance::~ScriptInstance()
{
   GEInvokeDtor(Script, cScript);
   Allocator::free(cScript);
}

void ScriptInstance::setScriptName(const ObjectName& Name)
{
   if(Name.isEmpty())
      return;

   bInitialized = false;

   cScript->reset();

   if(!cScript->loadFromFile(Name.getString().c_str()))
      return;

   cScriptName = Name;

   while(getPropertiesCount() > iBasePropertiesCount)
      removeProperty(getPropertiesCount() - 1);

   while(getActionsCount() > iBaseActionsCount)
      removeAction(getActionsCount() - 1);

   registerScriptProperties();
   registerScriptActions();
}

const ObjectName& ScriptInstance::getScriptName() const
{
   return cScriptName;
}

void ScriptInstance::setActive(bool Value)
{
   if(bActive == Value)
      return;

   bActive = Value;

   if(bActive)
   {
      if(cScript->isFunctionDefined(cActivateFunctionName))
      {
         cScript->runFunction<void>(cActivateFunctionName);
      }
   }
   else
   {
      if(cScript->isFunctionDefined(cDeactivateFunctionName))
      {
         cScript->runFunction<void>(cDeactivateFunctionName);
      }
   }
}

bool ScriptInstance::getActive() const
{
   return bActive;
}

void ScriptInstance::registerScriptProperties()
{
   const GESTLVector(ObjectName)& vGlobalVariableNames = cScript->getGlobalVariableNames();

   for(uint i = 0; i < vGlobalVariableNames.size(); i++)
   {
      const ObjectName& cGlobalVariableName = vGlobalVariableNames[i];
      const char* sGlobalVariableName = cGlobalVariableName.getString().c_str();
      ValueType ePropertyValue = cScript->getVariableType(sGlobalVariableName);

      if(ePropertyValue == ValueType::Count)
         continue;

      PropertySetter setter = [this, sGlobalVariableName](const Value& cValue)
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

      registerProperty(cGlobalVariableName, ePropertyValue, setter, getter);
   }

   if(cOwner)
   {
      Entity* cEntity = static_cast<ComponentScript*>(cOwner)->getOwner();
      EventArgs sEventArgs;
      sEventArgs.Sender = cEntity;
      cEntity->triggerEvent(Events::PropertiesUpdated, &sEventArgs);
   }
}

void ScriptInstance::registerScriptActions()
{
   const GESTLVector(ObjectName)& vGlobalFunctionNames = cScript->getGlobalFunctionNames();

   for(uint i = 0; i < vGlobalFunctionNames.size(); i++)
   {
      const ObjectName& cGlobalFunctionName = vGlobalFunctionNames[i];
      bool bInternalFunction = false;

      for(uint j = 0; j < iInternalFuncionNamesCount; j++)
      {
         if(cGlobalFunctionName == *cInternalFunctionNames[j])
         {
            bInternalFunction = true;
            break;
         }
      }

      if(bInternalFunction)
         continue;

      const char* sGlobalFunctionName = cGlobalFunctionName.getString().c_str();

      registerAction(sGlobalFunctionName, [this, cGlobalFunctionName]
      {
         cScript->runFunction<void>(cGlobalFunctionName);
      });
   }
}

void ScriptInstance::update()
{
   if(!vCachedPropertyValues.empty())
   {
      for(uint i = 0; i < vCachedPropertyValues.size(); i++)
      {
         ObjectName cPropertyName = ObjectName(vCachedPropertyValues[i].PropertyNameHash);
         const Property* cProperty = getProperty(cPropertyName);

         if(cProperty)
         {
            cProperty->Setter(vCachedPropertyValues[i].PropertyValue);
         }
      }

      vCachedPropertyValues.clear();
      vCachedPropertyValues.shrink_to_fit();
   }

   if(!bActive || cScriptName.isEmpty())
      return;

   Entity* cEntity = static_cast<ComponentScript*>(cOwner)->getOwner();

   if(!bInitialized)
   {
      cScript->setVariable<Entity*>("entity", cEntity);
      cScript->setVariable<ScriptInstance*>("this", this);

      if(cScript->isFunctionDefined(cInitFunctionName))
      {
         cScript->runFunction<void>(cInitFunctionName);
      }

      if(cScript->isFunctionDefined(cActivateFunctionName))
      {
         cScript->runFunction<void>(cActivateFunctionName);
      }

      bInitialized = true;
   }

   if(cScript->isFunctionDefined(cUpdateFunctionName))
   {
      cScript->setVariable<float>("deltaTime", Time::getClock(cEntity->getClockIndex()).getDelta());
      cScript->runFunction<void>(cUpdateFunctionName);
   }
}

bool ScriptInstance::inputTouchBegin(int ID, const Vector2& Point)
{
   if(!bActive)
      return false;

   if(!cScriptName.isEmpty() && cScript->isFunctionDefined(cInputTouchBeginFunctionName))
   {
      if(cScript->runFunction<bool>(cInputTouchBeginFunctionName, ID, Point))
         return true;
   }

   return false;
}

bool ScriptInstance::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   if(!bActive)
      return false;

   if(!cScriptName.isEmpty() && cScript->isFunctionDefined(cInputTouchMoveFunctionName))
   {
      if(cScript->runFunction<bool>(cInputTouchMoveFunctionName, ID, PreviousPoint, CurrentPoint))
         return true;
   }

   return false;
}

bool ScriptInstance::inputTouchEnd(int ID, const Vector2& Point)
{
   if(!bActive)
      return false;

   if(!cScriptName.isEmpty() && cScript->isFunctionDefined(cInputTouchEndFunctionName))
   {
      if(cScript->runFunction<bool>(cInputTouchEndFunctionName, ID, Point))
         return true;
   }

   return false;
}


//
//  ComponentScript
//
ComponentScript::ComponentScript(Entity* Owner)
   : Component(Owner)
{
   cClassName = ObjectName("Script");

   GERegisterPropertyArray(ScriptInstance);
}

ComponentScript::~ComponentScript()
{
   GEReleasePropertyArray(ScriptInstance);
}

void ComponentScript::update()
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      getScriptInstance(i)->update();
   }
}

bool ComponentScript::inputTouchBegin(int ID, const Vector2& Point)
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      if(getScriptInstance(i)->inputTouchBegin(ID, Point))
         return true;
   }

   return false;
}

bool ComponentScript::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      if(getScriptInstance(i)->inputTouchMove(ID, PreviousPoint, CurrentPoint))
         return true;
   }

   return false;
}

bool ComponentScript::inputTouchEnd(int ID, const Vector2& Point)
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      if(getScriptInstance(i)->inputTouchEnd(ID, Point))
         return true;
   }

   return false;
}
