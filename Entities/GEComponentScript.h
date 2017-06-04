
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
#include "Core/GEScript.h"

namespace GE { namespace Entities
{
   class ScriptInstance : public Core::SerializableArrayElement
   {
   private:
      Core::Script* cScript;
      GESTLString sScriptName;

      uint iBasePropertiesCount;
      bool bInitialized;

      void registerScriptProperties();

   public:
      ScriptInstance();
      ~ScriptInstance();

      void setScriptName(const char* FileName);
      const char* getScriptName() const;

      void update();

      bool inputTouchBegin(int ID, const Vector2& Point);
      bool inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint);
      bool inputTouchEnd(int ID, const Vector2& Point);

      GEProperty(String, ScriptName);
   };


   class ComponentScript : public Component
   {
   public:
      static ComponentType getType() { return ComponentType::Script; }

      ComponentScript(Entity* Owner);
      ~ComponentScript();

      void update();

      bool inputTouchBegin(int ID, const Vector2& Point);
      bool inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint);
      bool inputTouchEnd(int ID, const Vector2& Point);

      GEPropertyArray(ScriptInstance, ScriptInstance)
   };
}}
