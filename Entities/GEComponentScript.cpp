
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

const ObjectName cRestartActionName = ObjectName("Restart");
const ObjectName cReloadActionName = ObjectName("Reload");
const ObjectName cDebugActionName = ObjectName("Debug");

const ObjectName cThisVariableName = ObjectName("this");
const ObjectName cEntityVariableName = ObjectName("entity");
const ObjectName cDeltaTimeVariableName = ObjectName("deltaTime");

const ObjectName cInitFunctionName = ObjectName("init");
const ObjectName cUpdateFunctionName = ObjectName("update");
const ObjectName cInputMouseFunctionName = ObjectName("inputMouse");
const ObjectName cInputTouchBeginFunctionName = ObjectName("inputTouchBegin");
const ObjectName cInputTouchMoveFunctionName = ObjectName("inputTouchMove");
const ObjectName cInputTouchEndFunctionName = ObjectName("inputTouchEnd");

const ObjectName* cInternalFunctionNames[] =
{
   &cInitFunctionName,
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
   , cEnv(0)
   , eScriptSettings((uint8_t)ScriptSettingsBitMask::Active)
   , bInitialized(false)
#if defined (GE_EDITOR_SUPPORT)
   , iDebugBreakpointLine(0)
#endif
{
   GERegisterPropertyBitMask(ScriptSettingsBitMask, ScriptSettings);
   GERegisterProperty(ObjectName, ScriptName);

   registerAction(cRestartActionName, [this]
   {
      bInitialized = false;
   });

#if defined (GE_EDITOR_SUPPORT)
   registerAction(cReloadActionName, [this]
   {
      cachePropertyValues();
      setScriptName(getScriptName());
   });

   GERegisterProperty(UInt, DebugBreakpointLine);

   registerAction(cDebugActionName, [this]
   {
      cEnv->enableDebugger();
   });
#endif

   iBasePropertiesCount = getPropertiesCount();
   iBaseActionsCount = getActionsCount();
}

ScriptInstance::~ScriptInstance()
{
   if(cEnv)
   {
      if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
      {
         ScriptingEnvironment::leaveSharedEnvironment(cScriptName);
      }
      else
      {
         GEInvokeDtor(ScriptingEnvironment, cEnv);
         Allocator::free(cEnv);
      }

      cEnv = 0;
   }
}

void ScriptInstance::setScriptName(const ObjectName& Name)
{
   if(Name.isEmpty())
      return;

   bInitialized = false;

   if(cEnv)
   {
      if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
      {
         ScriptingEnvironment::leaveSharedEnvironment(cScriptName);
         cEnv = 0;
      }
      else
      {
         cEnv->reset();

         if(!cEnv->loadFromFile(Name.getString()))
            return;
      }
   }
   
   if(!cEnv)
   {
      if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
      {
         cEnv = ScriptingEnvironment::requestSharedEnvironment(Name);

         if(cEnv->isReset())
         {
            if(!cEnv->loadFromFile(Name.getString()))
            {
               ScriptingEnvironment::leaveSharedEnvironment(Name);
               return;
            }
         }
      }
      else
      {
         cEnv = Allocator::alloc<ScriptingEnvironment>();
         GEInvokeCtor(ScriptingEnvironment, cEnv);

         if(!cEnv->loadFromFile(Name.getString()))
            return;
      }
   }

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

   const bool bSharedEnvironmentBefore = GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment);

   eScriptSettings = BitMask;

   if(!cEnv)
      return;

   const bool bSharedEnvironmentNow = GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment);

   if(bSharedEnvironmentBefore != bSharedEnvironmentNow)
   {
      cachePropertyValues();

      if(bSharedEnvironmentBefore)
      {
         ScriptingEnvironment::leaveSharedEnvironment(cScriptName);
         cEnv = 0;
      }
      else
      {
         GEInvokeDtor(ScriptingEnvironment, cEnv);
         Allocator::free(cEnv);
         cEnv = 0;
      }

      setScriptName(cScriptName);
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

void ScriptInstance::setActive(bool Value)
{
   if(Value)
   {
      setScriptSettings(eScriptSettings | (uint8_t)ScriptSettingsBitMask::Active);
   }
   else
   {
      setScriptSettings(eScriptSettings & ~((uint8_t)ScriptSettingsBitMask::Active));
   }
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
   vInstancePropertyValues.clear();

   const GESTLVector(ObjectName)& vGlobalVariableNames = cEnv->getGlobalVariableNames();

   for(uint i = 0; i < vGlobalVariableNames.size(); i++)
   {
      const ObjectName& cGlobalVariableName = vGlobalVariableNames[i];
      const char* sGlobalVariableName = cGlobalVariableName.getString();
      const ValueType ePropertyType = cEnv->getVariableType(sGlobalVariableName);

      if(ePropertyType == ValueType::Count)
         continue;

      PropertySetter setter = nullptr;
      PropertyGetter getter = nullptr;

      if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
      {
         const size_t propertyIndex = vInstancePropertyValues.size();

         setter = [this, propertyIndex](const Value& cValue)
         {
            vInstancePropertyValues[propertyIndex].PropertyValue = cValue;
         };
         getter = [this, propertyIndex]() -> Value
         {
            return vInstancePropertyValues[propertyIndex].PropertyValue;
         };

         CachedPropertyValue cachedPropertyValue;
         cachedPropertyValue.PropertyName = cGlobalVariableName;
         cachedPropertyValue.PropertyValue = getScriptProperty(cGlobalVariableName, ePropertyType);
         vInstancePropertyValues.push_back(cachedPropertyValue);
      }
      else
      {
         setter = [this, cGlobalVariableName](const Value& cValue)
         {
            setScriptProperty(cGlobalVariableName, cValue);

            char sOnPropertySetFuncionNameStr[64];
            sprintf(sOnPropertySetFuncionNameStr, "on%sSet", cGlobalVariableName.getString());
            ObjectName cOnPropertySetFuncionName = ObjectName(sOnPropertySetFuncionNameStr);

            if(cEnv->isFunctionDefined(cOnPropertySetFuncionName))
            {
               cEnv->runFunction<void>(cOnPropertySetFuncionName, cValue);
            }
         };
         getter = [this, cGlobalVariableName, ePropertyType]() -> Value
         {
            return getScriptProperty(cGlobalVariableName, ePropertyType);
         };
      }

      Property* cProperty = registerProperty(cGlobalVariableName, ePropertyType, setter, getter);

#if defined (GE_EDITOR_SUPPORT)
      if(!isupper(sGlobalVariableName[0]))
      {
         GESetFlag(cProperty->Flags, PropertyFlags::Internal);
      }
#endif
   }

   vInstancePropertyValues.shrink_to_fit();

   EventArgs sArgs;
   sArgs.Data = cOwner;
   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
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

      const char* sGlobalFunctionName = cGlobalFunctionName.getString();

      registerAction(sGlobalFunctionName, [this, cGlobalFunctionName]
      {
         if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
         {
            cEnv->lock();

            updateEnvironmentContext();
         }

         cEnv->runFunction<void>(cGlobalFunctionName);

         if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
         {
            updateInstancePropertyValues();

            cEnv->unlock();
         }
      });
   }
}

void ScriptInstance::setScriptProperty(const ObjectName& pName, const Value& pValue)
{
   switch(pValue.getType())
   {
   case ValueType::Bool:
      cEnv->setVariable<bool>(pName, pValue.getAsBool());
      break;
   case ValueType::Int:
      cEnv->setVariable<int>(pName, pValue.getAsInt());
      break;
   case ValueType::Float:
      cEnv->setVariable<float>(pName, pValue.getAsFloat());
      break;
   case ValueType::String:
      cEnv->setVariable<const char*>(pName, pValue.getAsString());
      break;
   case ValueType::Vector3:
      cEnv->setVariable<Vector3>(pName, pValue.getAsVector3());
      break;
   case ValueType::Vector2:
      cEnv->setVariable<Vector2>(pName, pValue.getAsVector2());
      break;
   case ValueType::Color:
      cEnv->setVariable<Color>(pName, pValue.getAsColor());
      break;
   }
}

Value ScriptInstance::getScriptProperty(const ObjectName& pName, ValueType pType)
{
   switch(pType)
   {
   case ValueType::Bool:
      return Value(cEnv->getVariable<bool>(pName));
   case ValueType::Int:
      return Value(cEnv->getVariable<int>(pName));
   case ValueType::Float:
      return Value(cEnv->getVariable<float>(pName));
   case ValueType::Vector3:
      return Value(cEnv->getVariable<Vector3>(pName));
   case ValueType::Vector2:
      return Value(cEnv->getVariable<Vector2>(pName));
   case ValueType::Color:
      return Value(cEnv->getVariable<Color>(pName));
   default:
      return Value(pType, cEnv->getVariable<const char*>(pName));
   }
}

void ScriptInstance::updateEnvironmentContext()
{
   cEnv->setVariable<ScriptInstance*>(cThisVariableName, this);

   Entity* cEntity = static_cast<ComponentScript*>(cOwner)->getOwner();
   cEnv->setVariable<Entity*>(cEntityVariableName, cEntity);

   if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
   {
      for(uint i = 0; i < vInstancePropertyValues.size(); i++)
      {
         setScriptProperty(vInstancePropertyValues[i].PropertyName, vInstancePropertyValues[i].PropertyValue);
      }
   }
}

void ScriptInstance::updateInstancePropertyValues()
{
   for(size_t i = 0; i < vInstancePropertyValues.size(); i++)
   {
      const ObjectName& propertyName = vInstancePropertyValues[i].PropertyName;
      ValueType propertyType = vInstancePropertyValues[i].PropertyValue.getType();
      vInstancePropertyValues[i].PropertyValue = getScriptProperty(propertyName, propertyType);
   }
}

void ScriptInstance::cachePropertyValues()
{
#if defined (GE_EDITOR_SUPPORT)
   const uint iNumProperties = getPropertiesCount() - iBasePropertiesCount;
   vCachedPropertyValues.resize(iNumProperties);

   for(uint i = 0; i < iNumProperties; i++)
   {
      const Property& sProperty = getProperty(i + iBasePropertiesCount);
      vCachedPropertyValues[i].PropertyName = sProperty.Name;
      vCachedPropertyValues[i].PropertyValue = sProperty.Getter();
   }
#endif
}

void ScriptInstance::update()
{
   GEProfilerMarker("ScriptInstance::update()");

#if defined (GE_EDITOR_SUPPORT)
   if(!vCachedPropertyValues.empty())
   {
      for(uint i = 0; i < vCachedPropertyValues.size(); i++)
      {
         const Property* cProperty = getProperty(vCachedPropertyValues[i].PropertyName);

         if(cProperty)
         {
            cProperty->Setter(vCachedPropertyValues[i].PropertyValue);
         }
      }

      vCachedPropertyValues.clear();
   }
#endif

   if(!getActive() || cScriptName.isEmpty())
      return;

   if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
   {
      cEnv->lock();
   }

   updateEnvironmentContext();

   if(!bInitialized)
   {
      if(cEnv->isFunctionDefined(cInitFunctionName))
      {
         cEnv->runFunction<void>(cInitFunctionName);
      }

      bInitialized = true;
   }

   if(cEnv->isFunctionDefined(cUpdateFunctionName))
   {
      Entity* cEntity = static_cast<ComponentScript*>(cOwner)->getOwner();
      cEnv->setVariable<float>(cDeltaTimeVariableName, cEntity->getClock()->getDelta());
      cEnv->runFunction<void>(cUpdateFunctionName);
      cEnv->collectGarbage();
   }

   if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
   {
      updateInstancePropertyValues();

      cEnv->unlock();
   }
}

bool ScriptInstance::inputMouse(const Vector2& Point)
{
   if(!getActive())
      return false;

   if(!cScriptName.isEmpty() && cEnv->isFunctionDefined(cInputMouseFunctionName))
   {
      if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
         updateEnvironmentContext();

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
      if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
         updateEnvironmentContext();

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
      if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
         updateEnvironmentContext();

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
      if(GEHasFlag(eScriptSettings, ScriptSettingsBitMask::SharedEnvironment))
         updateEnvironmentContext();

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
   if(cOwner->getClock()->getDelta() < GE_EPSILON)
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
