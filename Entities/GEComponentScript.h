
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

      Count = 2
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
      Scripting::Environment* getEnvironment() const;

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

      void setDebugBreakpointLine(uint pLine);
      uint getDebugBreakpointLine() const;

      void setActive(bool pValue);
      bool getActive() const;
      bool getThreadSafe() const;

      void update();

      bool inputKeyPress(char pKey);
      bool inputKeyRelease(char pKey);
      bool inputKeyText(uint16_t pUnicode);

      bool inputMouse(const Vector2& pPoint);
      bool inputMouseWheel(int pDelta);

      bool inputTouchBegin(int pID, const Vector2& pPoint);
      bool inputTouchMove(int pID, const Vector2& pPreviousPoint, const Vector2& pCurrentPoint);
      bool inputTouchEnd(int pID, const Vector2& pPoint);
   };


   class ComponentScript : public Component, public Input::InputListener
   {
   private:
      uint32_t mJobIndex;

   public:
      static ComponentType getType() { return ComponentType::Script; }

      ComponentScript(Entity* Owner);
      ~ComponentScript();

      uint8_t getInputPriority() const { return mInputPriority; }
      uint32_t getJobIndex() const { return mJobIndex; }

      void setInputPriority(uint8_t pValue);

      void update();

      virtual bool inputKeyPress(char pKey) override;
      virtual bool inputKeyRelease(char pKey) override;
      virtual bool inputKeyText(uint16_t pUnicode) override;

      virtual bool inputMouse(const Vector2& pPoint) override;
      virtual bool inputMouseWheel(int pDelta) override;

      virtual bool inputTouchBegin(int pID, const Vector2& pPoint) override;
      virtual bool inputTouchMove(int pID, const Vector2& pPreviousPoint, const Vector2& pCurrentPoint) override;
      virtual bool inputTouchEnd(int pID, const Vector2& pPoint) override;

      ScriptInstance* getScriptInstanceByName(const Core::ObjectName& pScriptName);

      GEPropertyArray(ScriptInstance, ScriptInstance)
   };
}}
