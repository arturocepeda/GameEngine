
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
   class ScriptInstance : public Core::SerializableArrayElement
   {
   private:
      Scripting::ScriptingEnvironment* cEnv;
      Core::ObjectName cScriptName;

      uint iBasePropertiesCount;
      uint iBaseActionsCount;
      bool bActive;
      bool bInitialized;

      struct CachedPropertyValue
      {
         uint PropertyNameHash;
         Core::Value PropertyValue;

         CachedPropertyValue() : PropertyNameHash(0), PropertyValue(0) {}
      };
      GESTLVector(CachedPropertyValue) vCachedPropertyValues;

#if defined (GE_EDITOR_SUPPORT)
      uint iDebugBreakpointLine;
#endif

      void registerScriptProperties();
      void registerScriptActions();

   public:
      ScriptInstance();
      ~ScriptInstance();

      void setScriptName(const Core::ObjectName& Name);
      const Core::ObjectName& getScriptName() const;

      void setActive(bool Value);
      bool getActive() const;

#if defined (GE_EDITOR_SUPPORT)
      void setDebugBreakpointLine(uint Line);
      uint getDebugBreakpointLine() const;
#endif

      void update();

      bool inputMouse(const Vector2& Point);

      bool inputTouchBegin(int ID, const Vector2& Point);
      bool inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint);
      bool inputTouchEnd(int ID, const Vector2& Point);

      GEProperty(ObjectName, ScriptName);
      GEProperty(Bool, Active);
#if defined (GE_EDITOR_SUPPORT)
      GEProperty(UInt, DebugBreakpointLine);
#endif
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

      GEPropertyArray(ScriptInstance, ScriptInstance)
   };
}}
