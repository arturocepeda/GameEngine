
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


//
//  DataContainerVariable
//
ObjectName ValuePropertyName = ObjectName("Value");
uint ValuePropertyIndex = 2;

DataContainerVariable::DataContainerVariable()
   : SerializableArrayElement("DataContainerVariable")
   , cValue(0.0f)
{
   GERegisterProperty(ObjectName, Name);
   GERegisterPropertyEnum(ValueType, Type);

   setType(ValueType::Float);
}

DataContainerVariable::~DataContainerVariable()
{
}

const ObjectName& DataContainerVariable::getName() const
{
   return cName;
}

ValueType DataContainerVariable::getType() const
{
   return cValue.getType();
}

const Value& DataContainerVariable::getValue() const
{
   return cValue;
}

void DataContainerVariable::setName(const Core::ObjectName& Name)
{
   cName = Name;
}

void DataContainerVariable::setType(Core::ValueType Type)
{
   if(getPropertiesCount() > ValuePropertyIndex)
   {
      removeProperty(ValuePropertyIndex);
   }

   cValue = Value::getDefaultValue(Type);

   registerProperty(ValuePropertyName, Type,
      [this](const GE::Core::Value& v) { cValue = v; },
      [this]()->GE::Core::Value { return cValue; });

   if(cOwner)
   {
      Entity* cEntity = static_cast<ComponentDataContainer*>(cOwner)->getOwner();
      cEntity->triggerEvent(Events::PropertiesUpdated);
   }
}

void DataContainerVariable::setValue(const Value& Val)
{
   if(cValue.getType() != Val.getType())
   {
      setType(Val.getType());
   }

   cValue = Val;
}


//
//  ComponentDataContainer
//
ComponentDataContainer::ComponentDataContainer(Entity* Owner)
   : Component(Owner)
{
   cClassName = ObjectName("DataContainer");

   GERegisterPropertyArray(DataContainerVariable);
}

ComponentDataContainer::~ComponentDataContainer()
{
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
   }

   cVariableToSet->setValue(Val);
}
