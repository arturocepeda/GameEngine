
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GESerializable.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GESerializable.h"
#include "GEEvents.h"
#include "GELog.h"

using namespace GE;
using namespace GE::Core;


//
//  Serializable
//
Serializable::Serializable(const ObjectName& ClassName)
{
   mClassNames.push_back(ClassName);
}

Serializable::~Serializable()
{
}

Property* Serializable::registerProperty(const ObjectName& PropertyName, ValueType Type,
   const PropertySetter& Setter, const PropertyGetter& Getter,
   PropertyEditor Editor, uint8_t Flags, void* PropertyDataPtr,
   uint PropertyDataUInt)
{
   Property sProperty =
   {
      PropertyName,
      Type,
      Flags,
      Getter,
      Setter,
#if defined (GE_EDITOR_SUPPORT)
      mClassNames.back(),
      Editor,
      Getter(),
      PropertyDataPtr,
      PropertyDataUInt,
#endif
   };
   vProperties.push_back(sProperty);

   return &vProperties.back();
}

void Serializable::removeProperty(uint PropertyIndex)
{
   GEAssert(PropertyIndex < (uint)vProperties.size());
   vProperties.erase(vProperties.begin() + PropertyIndex);
}

PropertyArray* Serializable::registerPropertyArray(const ObjectName& PropertyArrayName,
   PropertyArrayEntries* PropertyArrayEntries,
   const PropertyArrayAdd& Add, const PropertyArrayRemove& Remove,
   const PropertyArraySwap& Swap, const PropertyArrayXmlToStream& XmlToStream)
{
   PropertyArray sPropertyArray;
   sPropertyArray.Name = PropertyArrayName;
   sPropertyArray.Entries = PropertyArrayEntries;
   sPropertyArray.Add = Add;
#if defined (GE_EDITOR_SUPPORT)
   sPropertyArray.Remove = Remove;
   sPropertyArray.Swap = Swap;
   sPropertyArray.XmlToStream = XmlToStream;
#endif
   vPropertyArrays.push_back(sPropertyArray);

   return &vPropertyArrays.back();
}

Action* Serializable::registerAction(const ObjectName& ActionName, const ActionFunction& Function)
{
   Action sAction;
   sAction.Name = ActionName;
   sAction.Function = Function;
   GEAssert(sAction.Function != nullptr);
   vActions.push_back(sAction);

   return &vActions.back();
}

void Serializable::removeAction(uint ActionIndex)
{
   GEAssert(ActionIndex < (uint)vActions.size());
   vActions.erase(vActions.begin() + ActionIndex);
}

uint Serializable::getPropertiesCount() const
{
   return (uint)vProperties.size();
}

const Property& Serializable::getProperty(uint PropertyIndex) const
{
   GEAssert(PropertyIndex < (uint)vProperties.size());
   return vProperties[PropertyIndex];
}

const Property* Serializable::getProperty(const ObjectName& PropertyName) const
{
   for(uint i = 0; i < vProperties.size(); i++)
   {
      if(vProperties[i].Name == PropertyName)
         return &vProperties[i];
   }

   return 0;
}

uint Serializable::getPropertyArraysCount() const
{
   return (uint)vPropertyArrays.size();
}

const PropertyArray& Serializable::getPropertyArray(uint PropertyArrayIndex) const
{
   GEAssert(PropertyArrayIndex < (uint)vPropertyArrays.size());
   return vPropertyArrays[PropertyArrayIndex];
}

const PropertyArray* Serializable::getPropertyArray(const ObjectName& PropertyArrayName) const
{
   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      if(vPropertyArrays[i].Name == PropertyArrayName)
         return &vPropertyArrays[i];
   }

   return 0;
}

uint Serializable::getActionsCount() const
{
   return (uint)vActions.size();
}

const Action& Serializable::getAction(uint ActionIndex) const
{
   GEAssert(ActionIndex < (uint)vActions.size());
   return vActions[ActionIndex];
}

bool Serializable::is(const ObjectName& ClassName) const
{
   for(int i = (int)mClassNames.size() - 1; i >= 0; i--)
   {
      if(mClassNames[i] == ClassName)
      {
         return true;
      }
   }

   return false;
}

bool Serializable::has(const ObjectName& PropertyName) const
{
   for(uint i = 0; i < vProperties.size(); i++)
   {
      if(vProperties[i].Name == PropertyName)
      {
         return true;
      }
   }

   return false;
}

Value Serializable::get(const ObjectName& PropertyName)
{
   Property* cProperty = 0;

   for(uint i = 0; i < vProperties.size(); i++)
   {
      if(vProperties[i].Name == PropertyName)
      {
         cProperty = &vProperties[i];
         break;
      }
   }

   GEAssert(cProperty);
   GEAssert(cProperty->Getter);
   return cProperty->Getter();
}

void Serializable::set(const ObjectName& PropertyName, const Value& PropertyValue)
{
   Property* cProperty = 0;

   for(uint i = 0; i < vProperties.size(); i++)
   {
      if(vProperties[i].Name == PropertyName)
      {
         cProperty = &vProperties[i];
         break;
      }
   }

   GEAssert(cProperty);
   GEAssert(cProperty->Setter);
   cProperty->Setter(PropertyValue);
}

void Serializable::executeAction(const ObjectName& ActionName)
{
   for(uint i = 0; i < vActions.size(); i++)
   {
      if(vActions[i].Name == ActionName)
      {
         vActions[i].Function();
         break;
      }
   }
}

void Serializable::copy(Serializable* cSource)
{
   GEAssert(cSource->is(getClassName()));

   for(uint i = 0; i < cSource->getPropertyArraysCount(); i++)
   {
      const PropertyArray& sSourcePropertyArray = cSource->getPropertyArray(i);
      const PropertyArray* sTargetPropertyArray = getPropertyArray(sSourcePropertyArray.Name);

      if(sTargetPropertyArray)
      {
         for(uint j = 0; j < sSourcePropertyArray.Entries->size(); j++)
         {
            if(sTargetPropertyArray->Entries->size() <= j)
               sTargetPropertyArray->Add();

            sTargetPropertyArray->Entries->at(j)->copy(sSourcePropertyArray.Entries->at(j));
         }
      }
   }

   for(uint i = 0; i < cSource->getPropertiesCount(); i++)
   {
      const Property& sSourceProperty = cSource->getProperty(i);

      if(!sSourceProperty.Setter)
         continue;

      const Property* sTargetProperty = getProperty(sSourceProperty.Name);

      GEAssert(sSourceProperty.Type == sTargetProperty->Type);

      Value cSourcePropertyValue = sSourceProperty.Getter();
      sTargetProperty->Setter(cSourcePropertyValue);
   }
}

void Serializable::loadFromXml(const pugi::xml_node& XmlNode)
{
   for(const pugi::xml_node& xmlProperty : XmlNode.children("Property"))
   {
      const char* sName = xmlProperty.attribute("name").value();
      const char* sValue = xmlProperty.attribute("value").value();
      GEAssert(strlen(sValue) < Value::BufferSize);

      ObjectName cPropertyName = ObjectName(sName);
      Property* cProperty = 0;

      for(uint i = 0; i < vProperties.size(); i++)
      {
         if(vProperties[i].Name == cPropertyName)
         {
            cProperty = &vProperties[i];
            break;
         }
      }

      if(!cProperty)
      {
         Log::log(LogType::Error, "The '%s' property does not exist", cPropertyName.getString());
         continue;
      }

      if(!cProperty->Setter)
      {
         Log::log(LogType::Error, "The '%s' property is read-only", cPropertyName.getString());
         continue;
      }

      Value cPropertyValue = Value(cProperty->Type, sValue);
      cProperty->Setter(cPropertyValue);
   }

   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      const char* sPropertyArrayElementName = sPropertyArray.Name.getString();

      sPropertyArray.Entries->clear();
      sPropertyArray.Entries->shrink_to_fit();

      uint iPropertyArrayElementsCount = 0;

      for(const pugi::xml_node& xmlPropertyArrayElement : XmlNode.children(sPropertyArrayElementName))
      {
         iPropertyArrayElementsCount++;
      }

      sPropertyArray.Entries->reserve(iPropertyArrayElementsCount);

      uint iPropertyArrayElementIndex = 0;

      for(const pugi::xml_node& xmlPropertyArrayElement : XmlNode.children(sPropertyArrayElementName))
      {
         sPropertyArray.Add();
         SerializableArrayElement* cPropertyArrayElement =
            sPropertyArray.Entries->at(iPropertyArrayElementIndex++);
         cPropertyArrayElement->loadFromXml(xmlPropertyArrayElement);
      }
   }
}

void Serializable::saveToXml(pugi::xml_node& XmlNode) const
{
   for(uint i = 0; i < vProperties.size(); i++)
   {
      const Property& sProperty = vProperties[i];

      if(!sProperty.Setter)
         continue;

#if defined (GE_EDITOR_SUPPORT)
      if(sProperty.Getter() == sProperty.DefaultValue)
         continue;
      if(GEHasFlag(sProperty.Flags, PropertyFlags::Runtime))
         continue;
#endif

      char sValueBuffer[Value::BufferSize];

      sProperty.Getter().toString(sValueBuffer);

      if(sValueBuffer[0] == '\0')
         continue;

      pugi::xml_node xmlProperty = XmlNode.append_child("Property");
      xmlProperty.append_attribute("name").set_value(sProperty.Name.getString());
      xmlProperty.append_attribute("value").set_value(sValueBuffer);
   }

   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      const char* sPropertyArrayElementName = sPropertyArray.Name.getString();

      for(uint j = 0; j < sPropertyArray.Entries->size(); j++)
      {
         SerializableArrayElement* cPropertyArrayElement = sPropertyArray.Entries->at(j);
         pugi::xml_node xmlPropertyArrayElement = XmlNode.append_child(sPropertyArrayElementName);
         cPropertyArrayElement->saveToXml(xmlPropertyArrayElement);
      }
   }
}

void Serializable::loadFromStream(std::istream& Stream)
{
   for(uint i = 0; i < vProperties.size(); i++)
   {
      const Property& sProperty = vProperties[i];

      if(sProperty.Setter && !GEHasFlag(sProperty.Flags, PropertyFlags::Runtime))
      {
         Value cPropertyValue = Value::fromStream(sProperty.Type, Stream);
         sProperty.Setter(cPropertyValue);
      }
   }

   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      const uint iPropertyArrayElementsCount =
         (uint)Value::fromStream(ValueType::Byte, Stream).getAsByte();

      sPropertyArray.Entries->clear();
      sPropertyArray.Entries->shrink_to_fit();
      sPropertyArray.Entries->reserve(iPropertyArrayElementsCount);

      for(uint j = 0; j < iPropertyArrayElementsCount; j++)
      {
         sPropertyArray.Add();
         SerializableArrayElement* cPropertyArrayElement = sPropertyArray.Entries->at(j);
         cPropertyArrayElement->loadFromStream(Stream);
      }
   }
}

void Serializable::saveToStream(std::ostream& Stream) const
{
   for(uint i = 0; i < vProperties.size(); i++)
   {
      const Property& sProperty = vProperties[i];

      if(sProperty.Setter && !GEHasFlag(sProperty.Flags, PropertyFlags::Runtime))
      {
         sProperty.Getter().writeToStream(Stream);
      }
   }

   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      Value((GE::byte)sPropertyArray.Entries->size()).writeToStream(Stream);

      for(uint j = 0; j < sPropertyArray.Entries->size(); j++)
      {
         SerializableArrayElement* cArrayElement = sPropertyArray.Entries->at(j);
         cArrayElement->saveToStream(Stream);
      }
   }
}

void Serializable::xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream)
{
#if defined (GE_EDITOR_SUPPORT)
   for(uint i = 0; i < vProperties.size(); i++)
   {
      const Property& sProperty = vProperties[i];

      if(!sProperty.Setter)
         continue;

      bool bPropertySet = false;

      for(const pugi::xml_node& xmlProperty : XmlNode.children("Property"))
      {
         const char* sName = xmlProperty.attribute("name").value();
         ObjectName cPropertyName = ObjectName(sName);

         if(cPropertyName == sProperty.Name)
         {
            const char* sValue = xmlProperty.attribute("value").value();
            GEAssert(strlen(sValue) < Value::BufferSize);

            Value cValue = Value(sProperty.Type, sValue);
            cValue.writeToStream(Stream);
            bPropertySet = true;

            break;
         }
      }

      if(!bPropertySet)
      {
         sProperty.DefaultValue.writeToStream(Stream);
      }
   }

   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      const char* sPropertyArrayElementName = sPropertyArray.Name.getString();

      pugi::xml_object_range<pugi::xml_named_node_iterator> xmlPropertyArrayElement = XmlNode.children(sPropertyArrayElementName);
      GE::byte iPropertyArrayElementsCount = 0;

      for(const pugi::xml_node& xmlPropertyArrayElement : xmlPropertyArrayElement)
      {
         iPropertyArrayElementsCount++;
      }

      Value(iPropertyArrayElementsCount).writeToStream(Stream);

      for(const pugi::xml_node& xmlPropertyArrayElement : xmlPropertyArrayElement)
      {
         sPropertyArray.XmlToStream(xmlPropertyArrayElement, Stream);
      }
   }
#endif
}

void Serializable::mergeXmlDescription(pugi::xml_node& pXmlBase, const pugi::xml_node& pXmlDerived)
{
   for(const pugi::xml_node& xmlDerivedNode : pXmlDerived.children())
   {
      // Property
      if(strcmp(xmlDerivedNode.name(), "Property") == 0)
      {
         const char* propertyName = xmlDerivedNode.attribute("name").value();
         pugi::xml_node xmlBaseProperty = pXmlBase.find_child_by_attribute("Property", "name", propertyName);

         if(xmlBaseProperty.empty())
         {
            pXmlBase.append_copy(xmlDerivedNode);
         }
         else
         {
            xmlBaseProperty.remove_attribute("value");
            const char* value = xmlDerivedNode.attribute("value").value();
            xmlBaseProperty.append_attribute("value").set_value(value);
         }
      }
      // Property array element
      else if(xmlDerivedNode.first_attribute().empty())
      {
         pXmlBase.append_copy(xmlDerivedNode);
      }
   }
}


//
//  SerializableArrayElement
//
SerializableArrayElement::SerializableArrayElement(const ObjectName& ClassName)
   : Serializable(ClassName)
   , cOwner(0)
{
}

SerializableArrayElement::~SerializableArrayElement()
{
}

void SerializableArrayElement::setOwner(Serializable* Owner)
{
   GEAssert(Owner);
   cOwner = Owner;
}


//
//  GenericVariable
//
const ObjectName kValuePropertyName("Value");
const uint32_t kValuePropertyIndex = 2u;

GenericVariable::GenericVariable(const ObjectName& ClassName)
   : SerializableArrayElement(ClassName)
   , cValue(0.0f)
{
   GERegisterProperty(ObjectName, Name);
   GERegisterPropertyEnum(ValueType, Type);

   setType(ValueType::Float);
}

GenericVariable::~GenericVariable()
{
}

const ObjectName& GenericVariable::getName() const
{
   return cName;
}

ValueType GenericVariable::getType() const
{
   return cValue.getType();
}

const Value& GenericVariable::getValue() const
{
   return cValue;
}

void GenericVariable::setName(const Core::ObjectName& Name)
{
   cName = Name;
}

void GenericVariable::setType(Core::ValueType Type)
{
   cValue = Value::getDefaultValue(Type);

   if(getPropertiesCount() == kValuePropertyIndex)
   {
      registerProperty(kValuePropertyName, Type,
         [this](const GE::Core::Value& v) { cValue = v; },
         [this]()->GE::Core::Value { return cValue; });
   }
   else
   {
      Property& valueProperty = const_cast<Property&>(getProperty(kValuePropertyIndex));
      valueProperty.Type = Type;
#if defined (GE_EDITOR_SUPPORT)
      valueProperty.DefaultValue = cValue;
#endif
   }

#if defined (GE_EDITOR_SUPPORT)
   if(cOwner)
   {
      EventArgs sArgs;
      sArgs.Data = cOwner;
      EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
   }
#endif
}

void GenericVariable::setValue(const Value& Val)
{
   if(cValue.getType() != Val.getType())
   {
      setType(Val.getType());
   }

   cValue = Val;
}
