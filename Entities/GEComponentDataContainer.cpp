
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentDataContainer.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentDataContainer.h"
#include "Core/GEEvents.h"
#include "Entities/GEEntity.h"

using namespace GE;
using namespace GE::Entities;
using namespace GE::Core;

ComponentDataContainer::ComponentDataContainer(Entity* Owner)
   : Component(Owner)
{
   mClassNames.push_back(ObjectName("DataContainer"));

   GERegisterPropertyArray(DataContainerVariable);
}

ComponentDataContainer::~ComponentDataContainer()
{
   GEReleasePropertyArray(DataContainerVariable);
}

const Value* ComponentDataContainer::getVariable(const ObjectName& VariableName)
{
   for(uint i = 0; i < getDataContainerVariableCount(); i++)
   {
      DataContainerVariable* cVariable = getDataContainerVariable(i);

      if(cVariable->getName() == VariableName)
      {
         return &cVariable->getValue();
      }
   }

   return 0;
}

bool ComponentDataContainer::getVariable(const Core::ObjectName& VariableName, Core::Value* OutValue)
{
   for(uint i = 0; i < getDataContainerVariableCount(); i++)
   {
      DataContainerVariable* cVariable = getDataContainerVariable(i);

      if(cVariable->getName() == VariableName)
      {
         *OutValue = cVariable->getValue();
         return true;
      }
   }

   return false;
}

void ComponentDataContainer::setVariable(const ObjectName& VariableName, const Value& Val)
{
   DataContainerVariable* cVariableToSet = 0;

   for(uint i = 0; i < getDataContainerVariableCount(); i++)
   {
      DataContainerVariable* cExistingVariable = getDataContainerVariable(i);

      if(cExistingVariable->getName() == VariableName)
      {
         cVariableToSet = cExistingVariable;
         break;
      }
   }

   if(!cVariableToSet)
   {
      cVariableToSet = addDataContainerVariable();
      cVariableToSet->setName(VariableName);
   }

   cVariableToSet->setValue(Val);
}
