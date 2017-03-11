
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentGenericData.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentGenericData.h"

using namespace GE;
using namespace GE::Entities;
using namespace GE::Core;


//
//  GenericVariable
//
GenericVariable::GenericVariable(const ObjectName& Name)
   : Object(Name)
   , cValue(0)
{
}

GenericVariable::~GenericVariable()
{
}

const Value& GenericVariable::getValue() const
{
   return cValue;
}

void GenericVariable::setValue(const Value& Val)
{
   cValue = Val;
}


//
//  ComponentGenericData
//
ComponentGenericData::ComponentGenericData(Entity* Owner)
   : Component(Owner)
{
   cClassName = ObjectName("GenericData");
}

ComponentGenericData::~ComponentGenericData()
{
   mVariables.clear();
}

bool ComponentGenericData::hasVariable(const Core::ObjectName& Name)
{
   return mVariables.get(Name) != 0;
}

void ComponentGenericData::setVariable(const ObjectName& Name, const Value& Val)
{
   GenericVariable* cVariable = mVariables.get(Name);

   if(!cVariable)
   {
      cVariable = Allocator::alloc<GenericVariable>();
      GEInvokeCtor(GenericVariable, cVariable)(Name);
      mVariables.add(cVariable);

      registerProperty(Name, Val.getType(),
         std::bind(&GenericVariable::setValue, cVariable, std::placeholders::_1),
         std::bind(&GenericVariable::getValue, cVariable));
   }

   cVariable->setValue(Val);
}

const Value& ComponentGenericData::getVariable(const ObjectName& Name)
{
   GenericVariable* cVariable = mVariables.get(Name);
   GEAssert(cVariable);
   return cVariable->getValue();
}

void ComponentGenericData::loadFromXml(const pugi::xml_node& XmlNode)
{
   for(const pugi::xml_node& xmlVariable : XmlNode.children("Variable"))
   {
      const char* sVariableName = xmlVariable.attribute("name").value();
      const char* sVariableType = xmlVariable.attribute("type").value();
      const char* sVariableValue = xmlVariable.attribute("value").value();

      ValueType eValueType = Value::getValueType(sVariableType);
      setVariable(ObjectName(sVariableName), Value(eValueType, sVariableValue));
   }

   Serializable::loadFromXml(XmlNode);
}

void ComponentGenericData::saveToXml(pugi::xml_node& XmlNode) const
{
   mVariables.iterateConst([&XmlNode](const GenericVariable* cVariable) -> bool
   {
      const char* sVariableName = cVariable->getName().getString().c_str();
      const Value& cVariableValue = cVariable->getValue();

      char sVariableValue[Value::BufferSize];
      cVariableValue.toString(sVariableValue);

      pugi::xml_node xmlVariable = XmlNode.append_child("Variable");

      xmlVariable.append_attribute("name").set_value(sVariableName);
      xmlVariable.append_attribute("type").set_value(strValueType[(uint)cVariableValue.getType()]);
      xmlVariable.append_attribute("value").set_value(sVariableValue);

      return true;
   });

   Serializable::saveToXml(XmlNode);
}
