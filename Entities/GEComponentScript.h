
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentScript.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponent.h"
#include "GEComponentType.h"

namespace GE { namespace Scripting
{
   class ScriptingEnvironment;
}}

namespace GE { namespace Entities
{
   GESerializableEnum(ScriptSettingsBitMask)
   {
      Active             = 1 << 0,
      ThreadSafe         = 1 << 1,
      SharedEnvironment  = 1 << 2,

      Count = 3
   };


   class ScriptInstance : public Core::SerializableArrayElement
   {
   private:
      Scripting::ScriptingEnvironment* cEnv;
      Core::ObjectName cScriptName;
      uint8_t eScriptSettings;

      uint iBasePropertiesCount;
      uint iBaseActionsCount;

      bool bInitialized;

      struct CachedPropertyValue
      {
         Core::ObjectName PropertyName;
         Core::Value PropertyValue;

         CachedPropertyValue() : PropertyValue(0) {}
      };
      GESTLVector(CachedPropertyValue) vInstancePropertyValues;

#if defined (GE_EDITOR_SUPPORT)
      GESTLVector(CachedPropertyValue) vCachedPropertyValues;
      uint iDebugBreakpointLine;
#endif

      void registerScriptProperties();
      void registerScriptActions();

      void setScriptProperty(const Core::ObjectName& pName, const Core::Value& pValue);
      Core::Value getScriptProperty(const Core::ObjectName& pName, Core::ValueType pType);

      void updateEnvironmentContext();
      void cachePropertyValues();

   public:
      ScriptInstance();
      ~ScriptInstance();

      void setScriptName(const Core::ObjectName& Name);
      const Core::ObjectName& getScriptName() const;

      void setScriptSettings(uint8_t BitMask);
      uint8_t getScriptSettings() const;

#if defined (GE_EDITOR_SUPPORT)
      void setDebugBreakpointLine(uint Line);
      uint getDebugBreakpointLine() const;
#endif

      void setActive(bool Value);
      bool getActive() const;
      bool getThreadSafe() const;

      void update();

      bool inputMouse(const Vector2& Point);

      bool inputTouchBegin(int ID, const Vector2& Point);
      bool inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint);
      bool inputTouchEnd(int ID, const Vector2& Point);
   };


   class ComponentScript : public Component
   {
   public:
      static ComponentType getType() { return ComponentType::Script; }

      ComponentScript(Entity* Owner);
      ~ComponentScript();

      void update();

      bool inputMouse(const Vector2& Point);

      bool inputTouchBegin(int ID, const Vector2& Point);
      bool inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint);
      bool inputTouchEnd(int ID, const Vector2& Point);

      ScriptInstance* getScriptInstanceByName(const Core::ObjectName& ScriptName);

      GEPropertyArray(ScriptInstance, ScriptInstance)
   };
}}
