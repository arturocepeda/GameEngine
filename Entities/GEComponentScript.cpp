
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
#include "Core/GEApplication.h"
#include "Core/GETime.h"
#include "Core/GEEvents.h"
#include "Core/GELog.h"
#include "Core/GEProfiler.h"
#include "Entities/GEScene.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Content;
using namespace GE::Input;
using namespace GE::Scripting;

const ObjectName cRestartActionName = ObjectName("Restart");
const ObjectName cReloadActionName = ObjectName("Reload");
const ObjectName cDebugActionName = ObjectName("Debug");

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
   , mNamespace(0)
   , mScriptSettings((uint8_t)ScriptSettingsBitMask::Active)
   , mInitialized(false)
#if defined (GE_EDITOR_SUPPORT)
   , mDebugBreakpointLine(0)
#endif
{
   char namespaceNameBuffer[32];
   sprintf(namespaceNameBuffer, "si%p", this);
   mNamespaceName = ObjectName(namespaceNameBuffer);

   GERegisterPropertyBitMask(ScriptSettingsBitMask, ScriptSettings);
   GERegisterProperty(ObjectName, ScriptName);

   registerAction(cRestartActionName, [this]
   {
      mInitialized = false;
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
      //TODO: cEnv->enableDebugger();
   });
#endif

   mBasePropertiesCount = getPropertiesCount();
   mBaseActionsCount = getActionsCount();
}

ScriptInstance::~ScriptInstance()
{
   if(mNamespace)
   {
      mNamespace->getParent()->removeNamespace(mNamespaceName);
      mNamespace = 0;
   }
}

void ScriptInstance::setScriptName(const ObjectName& pName)
{
   if(pName.isEmpty())
      return;

   mInitialized = false;

   if(mNamespace)
   {
      mNamespace->getParent()->removeNamespace(mNamespaceName);
      mNamespace = 0;
   }

   mScriptName = pName;

   while(getPropertiesCount() > mBasePropertiesCount)
   {
      removeProperty(getPropertiesCount() - 1);
   }

   while(getActionsCount() > mBaseActionsCount)
   {
      removeAction(getActionsCount() - 1);
   }
   
   Environment* env = Application::getScriptingEnvironment(0);
   Namespace* globalNS = env->getGlobalNamespace();
   mNamespace = globalNS->addNamespaceFromModule(mNamespaceName, pName.getString());

   if(mNamespace)
   {
      registerScriptProperties();
      registerScriptActions();
   }
   else
   {
      Log::log(LogType::Warning, "The '%s' script does not return any valid module", pName.getString());
   }
}

const ObjectName& ScriptInstance::getScriptName() const
{
   return mScriptName;
}

void ScriptInstance::setScriptSettings(uint8_t BitMask)
{
   if(mScriptSettings == BitMask)
      return;

   mScriptSettings = BitMask;

   //TODO: process settings change
}

uint8_t ScriptInstance::getScriptSettings() const
{
   return mScriptSettings;
}

#if defined (GE_EDITOR_SUPPORT)
void ScriptInstance::setDebugBreakpointLine(uint Line)
{
   mDebugBreakpointLine = Line;
   //TODO: cEnv->setDebugBreakpointLine(Line);
}

uint ScriptInstance::getDebugBreakpointLine() const
{
   return mDebugBreakpointLine;
}
#endif

void ScriptInstance::setActive(bool Value)
{
   if(Value)
   {
      setScriptSettings(mScriptSettings | (uint8_t)ScriptSettingsBitMask::Active);
   }
   else
   {
      setScriptSettings(mScriptSettings & ~((uint8_t)ScriptSettingsBitMask::Active));
   }
}

bool ScriptInstance::getActive() const
{
   return GEHasFlag(mScriptSettings, ScriptSettingsBitMask::Active);
}

bool ScriptInstance::getThreadSafe() const
{
   return GEHasFlag(mScriptSettings, ScriptSettingsBitMask::ThreadSafe);
}

void ScriptInstance::registerScriptProperties()
{
   const GESTLVector(ObjectName)& vGlobalVariableNames = mNamespace->getGlobalVariableNames();

   for(uint i = 0; i < vGlobalVariableNames.size(); i++)
   {
      const ObjectName& cGlobalVariableName = vGlobalVariableNames[i];
      const char* sGlobalVariableName = cGlobalVariableName.getString();
      const ValueType ePropertyType = mNamespace->getVariableType(sGlobalVariableName);

      if(ePropertyType == ValueType::Count)
         continue;

      PropertySetter setter = [this, cGlobalVariableName](const Value& cValue)
      {
         setScriptProperty(cGlobalVariableName, cValue);

         char sOnPropertySetFuncionNameStr[64];
         sprintf(sOnPropertySetFuncionNameStr, "on%sSet", cGlobalVariableName.getString());
         ObjectName cOnPropertySetFuncionName = ObjectName(sOnPropertySetFuncionNameStr);

         if(mNamespace->isFunctionDefined(cOnPropertySetFuncionName))
         {
            mNamespace->runFunction<void>(cOnPropertySetFuncionName, cValue);
         }
      };
      PropertyGetter getter = [this, cGlobalVariableName, ePropertyType]() -> Value
      {
         return getScriptProperty(cGlobalVariableName, ePropertyType);
      };

      Property* cProperty = registerProperty(cGlobalVariableName, ePropertyType, setter, getter);

#if defined (GE_EDITOR_SUPPORT)
      if(!isupper(sGlobalVariableName[0]))
      {
         GESetFlag(cProperty->Flags, PropertyFlags::Internal);
      }
#endif
   }

   EventArgs sArgs;
   sArgs.Data = cOwner;
   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
}

void ScriptInstance::registerScriptActions()
{
   const GESTLVector(ObjectName)& vGlobalFunctionNames = mNamespace->getGlobalFunctionNames();

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

      if(mNamespace->getFunctionParametersCount(cGlobalFunctionName) > 0)
         continue;

      const char* sGlobalFunctionName = cGlobalFunctionName.getString();

      registerAction(sGlobalFunctionName, [this, cGlobalFunctionName]
      {
         mNamespace->runFunction<void>(cGlobalFunctionName);
      });
   }
}

void ScriptInstance::setScriptProperty(const ObjectName& pName, const Value& pValue)
{
   switch(pValue.getType())
   {
   case ValueType::Bool:
      mNamespace->setVariable<bool>(pName, pValue.getAsBool());
      break;
   case ValueType::UInt:
      mNamespace->setVariable<uint32_t>(pName, pValue.getAsUInt());
      break;
   case ValueType::Int:
      mNamespace->setVariable<int>(pName, pValue.getAsInt());
      break;
   case ValueType::Float:
      mNamespace->setVariable<float>(pName, pValue.getAsFloat());
      break;
   case ValueType::String:
      mNamespace->setVariable<const char*>(pName, pValue.getAsString());
      break;
   case ValueType::Vector3:
      mNamespace->setVariable<Vector3>(pName, pValue.getAsVector3());
      break;
   case ValueType::Vector2:
      mNamespace->setVariable<Vector2>(pName, pValue.getAsVector2());
      break;
   case ValueType::Color:
      mNamespace->setVariable<Color>(pName, pValue.getAsColor());
      break;
   }
}

Value ScriptInstance::getScriptProperty(const ObjectName& pName, ValueType pType)
{
   switch(pType)
   {
   case ValueType::Bool:
      return Value(mNamespace->getVariable<bool>(pName));
   case ValueType::Int:
      return Value(mNamespace->getVariable<int>(pName));
   case ValueType::Float:
      return Value(mNamespace->getVariable<float>(pName));
   case ValueType::Vector3:
      return Value(mNamespace->getVariable<Vector3>(pName));
   case ValueType::Vector2:
      return Value(mNamespace->getVariable<Vector2>(pName));
   case ValueType::Color:
      return Value(mNamespace->getVariable<Color>(pName));
   default:
      return Value(pType, mNamespace->getVariable<const char*>(pName));
   }
}

void ScriptInstance::cachePropertyValues()
{
#if defined (GE_EDITOR_SUPPORT)
   const uint iNumProperties = getPropertiesCount() - mBasePropertiesCount;
   mCachedPropertyValues.resize(iNumProperties);

   for(uint i = 0; i < iNumProperties; i++)
   {
      const Property& sProperty = getProperty(i + mBasePropertiesCount);
      mCachedPropertyValues[i].PropertyName = sProperty.Name;
      mCachedPropertyValues[i].PropertyValue = sProperty.Getter();
   }
#endif
}

void ScriptInstance::update()
{
   GEProfilerMarker("ScriptInstance::update()");

#if defined (GE_EDITOR_SUPPORT)
   if(!mCachedPropertyValues.empty())
   {
      for(uint i = 0; i < mCachedPropertyValues.size(); i++)
      {
         const Property* cProperty = getProperty(mCachedPropertyValues[i].PropertyName);

         if(cProperty)
         {
            cProperty->Setter(mCachedPropertyValues[i].PropertyValue);
         }
      }

      mCachedPropertyValues.clear();
   }
#endif

   if(!getActive() || mScriptName.isEmpty() || !mNamespace)
      return;

   Entity* entity = static_cast<ComponentScript*>(cOwner)->getOwner();

   if(!mInitialized)
   {
      if(mNamespace->isFunctionDefined(cInitFunctionName))
      {
         mNamespace->runFunction<void>(cInitFunctionName, this, entity);
      }

      mInitialized = true;
   }

   if(mNamespace->isFunctionDefined(cUpdateFunctionName))
   {
      const float deltaTime = entity->getClock()->getDelta();
      mNamespace->runFunction<void>(cUpdateFunctionName, this, entity, deltaTime);
   }
}

bool ScriptInstance::inputMouse(const Vector2& Point)
{
   if(!getActive() || !mNamespace)
      return false;

   if(!mScriptName.isEmpty() && mNamespace->isFunctionDefined(cInputMouseFunctionName))
   {
      if(mNamespace->runFunction<bool>(cInputMouseFunctionName, Point))
         return true;
   }

   return false;
}

bool ScriptInstance::inputTouchBegin(int ID, const Vector2& Point)
{
   if(!getActive() || !mNamespace)
      return false;

   if(!mScriptName.isEmpty() && mNamespace->isFunctionDefined(cInputTouchBeginFunctionName))
   {
      if(mNamespace->runFunction<bool>(cInputTouchBeginFunctionName, ID, Point))
         return true;
   }

   return false;
}

bool ScriptInstance::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   if(!getActive() || !mNamespace)
      return false;

   if(!mScriptName.isEmpty() && mNamespace->isFunctionDefined(cInputTouchMoveFunctionName))
   {
      if(mNamespace->runFunction<bool>(cInputTouchMoveFunctionName, ID, PreviousPoint, CurrentPoint))
         return true;
   }

   return false;
}

bool ScriptInstance::inputTouchEnd(int ID, const Vector2& Point)
{
   if(!getActive() || !mNamespace)
      return false;

   if(!mScriptName.isEmpty() && mNamespace->isFunctionDefined(cInputTouchEndFunctionName))
   {
      if(mNamespace->runFunction<bool>(cInputTouchEndFunctionName, ID, Point))
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
   mClassNames.push_back(ObjectName("Script"));

   GERegisterPropertyArray(ScriptInstance);

   InputSystem::getInstance()->addListener(this);
}

ComponentScript::~ComponentScript()
{
   InputSystem::getInstance()->removeListener(this);

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
   if(!cOwner->isActiveInHierarchy())
      return false;

   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      if(getScriptInstance(i)->inputMouse(Point))
         return true;
   }

   return false;
}

bool ComponentScript::inputTouchBegin(int ID, const Vector2& Point)
{
   if(!cOwner->isActiveInHierarchy())
      return false;

   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      if(getScriptInstance(i)->inputTouchBegin(ID, Point))
         return true;
   }

   return false;
}

bool ComponentScript::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   if(!cOwner->isActiveInHierarchy())
      return false;

   for(uint i = 0; i < vScriptInstanceList.size(); i++)
   {
      if(getScriptInstance(i)->inputTouchMove(ID, PreviousPoint, CurrentPoint))
         return true;
   }

   return false;
}

bool ComponentScript::inputTouchEnd(int ID, const Vector2& Point)
{
   if(!cOwner->isActiveInHierarchy())
      return false;

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
