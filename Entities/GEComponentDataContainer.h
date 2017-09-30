
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentDataContainer.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponent.h"
#include "GEComponentType.h"
#include "Core/GESerializable.h"
#include "Core/GEValue.h"

namespace GE { namespace Entities
{
   class DataContainerVariable : public Core::SerializableArrayElement
   {
   private:
      Core::ObjectName cName;
      Core::Value cValue;

   public:
      DataContainerVariable();
      ~DataContainerVariable();

      const Core::ObjectName& getName() const;
      Core::ValueType getType() const;
      const Core::Value& getValue() const;

      void setName(const Core::ObjectName& Name);
      void setType(Core::ValueType Type);
      void setValue(const Core::Value& Val);

      GEProperty(ObjectName, Name)
      GEPropertyEnum(Core::ValueType, Type)
   };


   class ComponentDataContainer : public Component
   {
   public:
      static ComponentType getType() { return ComponentType::DataContainer; }

      ComponentDataContainer(Entity* Owner);
      ~ComponentDataContainer();

      const Core::Value* getVariable(const Core::ObjectName& VariableName);
      bool getVariable(const Core::ObjectName& VariableName, Core::Value* OutValue);
      void setVariable(const Core::ObjectName& VariableName, const Core::Value& Val);

      GEPropertyArray(DataContainerVariable, DataContainerVariable)
   };
}}
