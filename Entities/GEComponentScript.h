
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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
#include "Input/GEInputSystem.h"

namespace GE { namespace Scripting
{
   class Namespace;
   class Environment;
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
      Core::ObjectName mNamespaceName;
      Scripting::Namespace* mNamespace;
      Core::ObjectName mScriptName;
      uint8_t mScriptSettings;

      uint mBasePropertiesCount;
      uint mBaseActionsCount;

      bool mInitialized;

#if defined (GE_EDITOR_SUPPORT)
      struct CachedPropertyValue
      {
         Core::ObjectName PropertyName;
         Core::Value PropertyValue;

         CachedPropertyValue() : PropertyValue(0) {}
      };
      GESTLVector(CachedPropertyValue) mCachedPropertyValues;
      uint mDebugBreakpointLine;
#endif

      void registerScriptProperties();
      void registerScriptActions();

      void setScriptProperty(const Core::ObjectName& pName, const Core::Value& pValue);
      Core::Value getScriptProperty(const Core::ObjectName& pName, Core::ValueType pType);

      void cachePropertyValues();

   public:
      ScriptInstance();
      ~ScriptInstance();

      void setScriptName(const Core::ObjectName& pName);
      const Core::ObjectName& getScriptName() const;

      void setScriptSettings(uint8_t pBitMask);
      uint8_t getScriptSettings() const;

#if defined (GE_EDITOR_SUPPORT)
      void setDebugBreakpointLine(uint pLine);
      uint getDebugBreakpointLine() const;
#endif

      void setActive(bool pValue);
      bool getActive() const;
      bool getThreadSafe() const;

      void update();

      bool inputMouse(const Vector2& pPoint);

      bool inputTouchBegin(int pID, const Vector2& pPoint);
      bool inputTouchMove(int pID, const Vector2& pPreviousPoint, const Vector2& pCurrentPoint);
      bool inputTouchEnd(int pID, const Vector2& pPoint);
   };


   class ComponentScript : public Component, public Input::InputListener
   {
   public:
      static ComponentType getType() { return ComponentType::Script; }

      ComponentScript(Entity* Owner);
      ~ComponentScript();

      void update();

      virtual bool inputMouse(const Vector2& Point) override;

      virtual bool inputTouchBegin(int ID, const Vector2& Point) override;
      virtual bool inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint) override;
      virtual bool inputTouchEnd(int ID, const Vector2& Point) override;

      ScriptInstance* getScriptInstanceByName(const Core::ObjectName& ScriptName);

      GEPropertyArray(ScriptInstance, ScriptInstance)
   };
}}
