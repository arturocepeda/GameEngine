
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
   class ComponentScript : public Component
   {
   private:
      Core::Script* cScript;

      GESTLString sScriptInit;
      GESTLString sScriptUpdate;

      bool bInitialized;

   public:
      static ComponentType getType() { return ComponentType::Script; }

      ComponentScript(Entity* Owner);
      ~ComponentScript();

      void update();

      void setScriptInit(const char* FileName);
      const char* getScriptInit() const;

      void setScriptUpdate(const char* FileName);
      const char* getScriptUpdate() const;

      GEProperty(String, ScriptInit);
      GEProperty(String, ScriptUpdate);
   };
}}
