
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentGenericData.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponent.h"
#include "GEComponentType.h"
#include "Core/GESerializable.h"
#include "Core/GEValue.h"
#include "Core/GEObjectManager.h"

#include <map>

namespace GE { namespace Entities
{
   class GenericVariable : public Core::Object
   {
   private:
      Core::Value cValue;

   public:
      GenericVariable(const Core::ObjectName& Name);
      ~GenericVariable();

      const Core::Value& getValue() const;
      void setValue(const Core::Value& Val);
   };


   class ComponentGenericData : public Component
   {
   private:
      Core::ObjectManager<GenericVariable> mVariables;

   public:
      static ComponentType getType() { return ComponentType::GenericData; }

      ComponentGenericData(Entity* Owner);
      ~ComponentGenericData();

      bool hasVariable(const Core::ObjectName& Name);
      void setVariable(const Core::ObjectName& Name, const Core::Value& Val);
      const Core::Value& getVariable(const Core::ObjectName& Name);

      virtual void loadFromXml(const pugi::xml_node& XmlNode) override;
      virtual void saveToXml(pugi::xml_node& XmlNode) const override;
   };
}}
