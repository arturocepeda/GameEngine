
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

      uint iBasePropertiesCount;
      bool bInitialized;

      void registerScriptProperties();

   public:
      static ComponentType getType() { return ComponentType::Script; }

      ComponentScript(Entity* Owner);
      ~ComponentScript();

      void setScriptName(const char* FileName);
      const char* getScriptName() const;

      void update();

      void inputTouchBegin(int ID, const Vector2& Point);
      void inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint);
      void inputTouchEnd(int ID, const Vector2& Point);

      virtual void loadFromStream(std::istream& Stream) override;
      virtual void xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream) override;

      GEProperty(String, ScriptName);
   };
}}
