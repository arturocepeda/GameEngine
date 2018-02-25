
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Entities
//
//  --- GEComponentScript.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentScript.h"
#include "Scripting/GEScriptingEnvironment.h"
#include "Content/GEContentData.h"
#include "Core/GETime.h"
#include "Core/GEEvents.h"
#include "Core/GEProfiler.h"
#include "Entities/GEScene.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Content;
using namespace GE::Scripting;

const ObjectName cInitFunctionName = ObjectName("init");
const ObjectName cActivateFunctionName = ObjectName("activate");
const ObjectName cDeactivateFunctionName = ObjectName("deactivate");
const ObjectName cUpdateFunctionName = ObjectName("update");
const ObjectName cInputMouseFunctionName = ObjectName("inputMouse");
const ObjectName cInputTouchBeginFunctionName = ObjectName("inputTouchBegin");
const ObjectName cInputTouchMoveFunctionName = ObjectName("inputTouchMove");
const ObjectName cInputTouchEndFunctionName = ObjectName("inputTouchEnd");

const ObjectName* cInternalFunctionNames[] =
{
   &cInitFunctionName,
   &cActivateFunctionName,
   &cDeactivateFunctionName,
   &cUpdateFunctionName,
   &cInputMouseFunctionName,
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
   , eScriptSettings((uint8_t)ScriptSettingsBitMask::Active)
   , bInitialized(false)
#if defined (GE_EDITOR_SUPPORT)
   , iDebugBreakpointLine(0)
#endif
{
   cEnv = Allocator::alloc<ScriptingEnvironment>();
   GEInvokeCtor(ScriptingEnvironment, cEnv);

   GERegisterPropertySpecialEditor(ObjectName, ScriptName, PropertyEditor::Script);
   GERegisterPropertyBitMask(ScriptSettingsBitMask, ScriptSettings);

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

#if defined (GE_EDITOR_SUPPORT)
   GERegisterProperty(UInt, DebugBreakpointLine);

   registerAction("Debug", [this]
   {
      cEnv->enableDebugger();
   });
#endif

   iBasePropertiesCount = getPropertiesCount();
   iBaseActionsCount = getActionsCount();
}

ScriptInstance::~ScriptInstance()
{
   GEInvokeDtor(ScriptingEnvironment, cEnv);
   Allocator::free(cEnv);
}

void ScriptInstance::setScriptName(const ObjectName& Name)
{
   if(Name.isEmpty())
      return;

   bInitialized = false;

   cEnv->reset();

   if(!cEnv->loadFromFile(Name.getString().c_str()))
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

void ScriptInstance::setScriptSettings(uint8_t BitMask)
{
   if(eScriptSettings == BitMask)
      return;

   bool bScriptActiveBefore = GEHasFlag(eScriptSettings, ScriptSettingsBitMask::Active);

   eScriptSettings = BitMask;

   bool bScriptActiveNow = GEHasFlag(eScriptSettings, ScriptSettingsBitMask::Active);

   if(bScriptActiveBefore != bScriptActiveNow)
   {
      if(bScriptActiveNow)
      {
         if(cEnv->isFunctionDefined(cActivateFunctionName))
         {
            cEnv->runFunction<void>(cActivateFunctionName);
         }
      }
      else
      {
         if(cEnv->isFunctionDefined(cDeactivateFunctionName))
         {
            cEnv->runFunction<void>(cDeactivateFunctionName);
         }
      }
   }
}

uint8_t ScriptInstance::getScriptSettings() const
{
   return eScriptSettings;
}

#if defined (GE_EDITOR_SUPPORT)
void ScriptInstance::setDebugBreakpointLine(uint Line)
{
   iDebugBreakpointLine = Line;
   cEnv->setDebugBreakpointLine(Line);
}

uint ScriptInstance::getDebugBreakpointLine() const
{
   return iDebugBreakpointLine;
}
#endif

void ScriptInstance::setActive()
{
   setScriptSettings(eScriptSettings | (uint8_t)ScriptSettingsBitMask::Active);
}

bool ScriptInstance::getActive() const
{
   return GEHasFlag(eScriptSettings, ScriptSettingsBitMask::Active);
}

bool ScriptInstance::getThreadSafe() const
{
   return GEHasFlag(eScriptSettings, ScriptSettingsBitMask::ThreadSafe);
}

void ScriptInstance::registerScriptProperties()
{
   const GESTLVector(ObjectName)& vGlobalVariableNames = cEnv->getGlobalVariableNames();

   for(uint i = 0; i < vGlobalVariableNames.size(); i++)
   {
      const ObjectName& cGlobalVariableName = vGlobalVariableNames[i];
      const char* sGlobalVariableName = cGlobalVariableName.getString().c_str();
      ValueType ePropertyValue = cEnv->getVariableType(sGlobalVariableName);

      if(ePropertyValue == ValueType::Count)
         continue;

      PropertySetter setter = [this, sGlobalVariableName](const Value& cValue)
      {
         switch(cValue.getType())
         {
         case ValueType::Bool:
            cEnv->setVariable<bool>(sGlobalVariableName, cValue.getAsBool());
            break;
         case ValueType::Int:
            cEnv->setVariable<int>(sGlobalVariableName, cValue.getAsInt());
            break;
         case ValueType::Float:
            cEnv->setVariable<float>(sGlobalVariableName, cValue.getAsFloat());
            break;
         case ValueType::String:
            cEnv->setVariable<const char*>(sGlobalVariableName, cValue.getAsString());
            break;
         case ValueType::Vector3:
            cEnv->setVariable<Vector3>(sGlobalVariableName, cValue.getAsVector3());
            break;
         case ValueType::Vector2:
            cEnv->setVariable<Vector2>(sGlobalVariableName, cValue.getAsVector2());
            break;
         case ValueType::Color:
            cEnv->setVariable<Color>(sGlobalVariableName, cValue.getAsColor());
            break;
         }
      };
      PropertyGetter getter = [this, sGlobalVariableName, ePropertyValue]() -> Value
      {
         switch(ePropertyValue)
         {
         case ValueType::Bool:
            return Value(cEnv->getVariable<bool>(sGlobalVariableName));
         case ValueType::Vector3:
            return Value(cEnv->getVariable<Vector3>(sGlobalVariableName));
         case ValueType::Vector2:
            return Value(cEnv->getVariable<Vector2>(sGlobalVariableName));
         case ValueType::Color:
            return Value(cEnv->getVariable<Color>(sGlobalVariableName));
         default:
            return Value(ePropertyValue, cEnv->getVariable<const char*>(sGlobalVariableName));
         }
      };

      registerProperty(cGlobalVariableName, ePropertyValue, setter, getter);
   }

   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated);
}

void ScriptInstance::registerScriptActions()
{
   const GESTLVector(ObjectName)& vGlobalFunctionNames = cEnv->getGlobalFunctionNames();

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

      if(cEnv->getFunctionParametersCount(cGlobalFunctionName) > 0)
         continue;

      const char* sGlobalFunctionName = cGlobalFunctionName.getString().c_str();

      registerAction(sGlobalFunctionName, [this, cGlobalFunctionName]
      {
         cEnv->runFunction<void>(cGlobalFunctionName);
      });
   }
}

void ScriptInstance::update()
{
   GEProfilerMarker("ScriptInstance::update()");

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

   if(!getActive() || cScriptName.isEmpty())
      return;

   Entity* cEntity = static_cast<ComponentScript*>(cOwner)->getOwner();

   if(!bInitialized)
   {
      cEnv->setVariable<Entity*>("entity", cEntity);
      cEnv->setVariable<ScriptInstance*>("this", this);

      if(cEnv->isFunctionDefined(cInitFunctionName))
      {
         cEnv->runFunction<void>(cInitFunctionName);
      }

      if(cEnv->isFunctionDefined(cActivateFunctionName))
      {
         cEnv->runFunction<void>(cActivateFunctionName);
      }

      bInitialized = true;
   }

   if(cEnv->isFunctionDefined(cUpdateFunctionName))
   {
      cEnv->setVariable<float>("deltaTime", Time::getClock(cEntity->getClockIndex()).getDelta());
      cEnv->runFunction<void>(cUpdateFunctionName);
      cEnv->collectGarbage();
   }
}

bool ScriptInstance::inputMouse(const Vector2& Point)
{
   if(!getActive())
      return false;

   if(!cScriptName.isEmpty() && cEnv->isFunctionDefined(cInputMouseFunctionName))
   {
      if(cEnv->runFunction<bool>(cInputMouseFunctionName, Point))
         return true;
   }

   return false;
}

bool ScriptInstance::inputTouchBegin(int ID, const Vector2& Point)
{
   if(!getActive())
      return false;

   if(!cScriptName.isEmpty() && cEnv->isFunctionDefined(cInputTouchBeginFunctionName))
   {
      if(cEnv->runFunction<bool>(cInputTouchBeginFunctionName, ID, Point))
         return true;
   }

   return false;
}

bool ScriptInstance::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   if(!getActive())
      return false;

   if(!cScriptName.isEmpty() && cEnv->isFunctionDefined(cInputTouchMoveFunctionName))
   {
      if(cEnv->runFunction<bool>(cInputTouchMoveFunctionName, ID, PreviousPoint, CurrentPoint))
         return true;
   }

   return false;
}

bool ScriptInstance::inputTouchEnd(int ID, const Vector2& Point)
{
   if(!getActive())
      return false;

   if(!cScriptName.isEmpty() && cEnv->isFunctionDefined(cInputTouchEndFunctionName))
   {
      if(cEnv->runFunction<bool>(cInputTouchEndFunctionName, ID, Point))
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
   if(Time::getClock(cOwner->getClockIndex()).getDelta() < GE_EPSILON)
      return;

   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      getScriptInstance(i)->update();
   }
}

bool ComponentScript::inputMouse(const Vector2& Point)
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      if(getScriptInstance(i)->inputMouse(Point))
         return true;
   }

   return false;
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

ScriptInstance* ComponentScript::getScriptInstanceByName(const ObjectName& ScriptName)
{
   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      ScriptInstance* cScriptInstance = getScriptInstance(i);

      if(cScriptInstance->getScriptName() == ScriptName)
         return cScriptInstance;
   }

   return 0;
}
