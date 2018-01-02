
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

namespace GE { namespace Entities
{
   class DataContainerVariable : public Core::GenericVariable
   {
   public:
      DataContainerVariable() : Core::GenericVariable("DataContainerVariable") {}
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
