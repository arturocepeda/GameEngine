
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


ComponentScript::ComponentScript(Entity* Owner)
   : Component(Owner)
   , bInitialized(false)
{
   cClassName = ObjectName("Script");

   cScript = Allocator::alloc<Script>();
   GEInvokeCtor(Script, cScript);

   GERegisterProperty(ComponentScript, String, ScriptName);

   iBasePropertiesCount = getPropertiesCount();
}

ComponentScript::~ComponentScript()
{
   GEInvokeDtor(Script, cScript);
   Allocator::free(cScript);
}

void ComponentScript::update()
{
   if(sScriptName.empty())
      return;

   if(!bInitialized)
   {
      cScript->setVariable<Entity*>("entity", cOwner);

      if(cScript->isFunctionDefined("init"))
      {
         cScript->runFunction("init");
      }

      bInitialized = true;
   }

   cScript->setVariable<Scene*>("scene", Scene::getActiveScene());
   cScript->setVariable<float>("deltaTime", Time::getClock(0).getDelta());
   
   if(cScript->isFunctionDefined("update"))
   {
      cScript->runFunction("update");
   }
}

void ComponentScript::setScriptName(const char* FileName)
{
   if(!FileName || strlen(FileName) == 0)
      return;

   sScriptName = FileName;
   cScript->loadFromFile(FileName);
   bInitialized = false;

   while(getPropertiesCount() > iBasePropertiesCount)
      removeProperty(getPropertiesCount() - 1);

   registerScriptProperties();
}

const char* ComponentScript::getScriptName() const
{
   return sScriptName.c_str();
}

void ComponentScript::registerScriptProperties()
{
   const GESTLVector(ObjectName)& vGlobalVariableNames = cScript->getGlobalVariableNames();

   for(uint i = 0; i < vGlobalVariableNames.size(); i++)
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
         return Value(ePropertyValue, cScript->getVariable<const char*>(sGlobalVariableName));
      };

      registerProperty(vGlobalVariableName, ePropertyValue, setter, getter);
   }

   EventArgs sEventArgs;
   sEventArgs.Sender = cOwner;
   cOwner->triggerEvent(EventPropertiesUpdated, &sEventArgs);
}
