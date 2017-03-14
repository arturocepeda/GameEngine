
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
      GESTLString sScriptName;

      bool bInitialized;

   public:
      static ComponentType getType() { return ComponentType::Script; }

      ComponentScript(Entity* Owner);
      ~ComponentScript();

      void update();

      void setScriptName(const char* FileName);
      const char* getScriptName() const;

      GEProperty(String, ScriptName);
   };
}}
