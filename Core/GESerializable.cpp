
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

using namespace GE;
using namespace GE::Core;


//
//  Serializable
//
const ObjectName Serializable::EventPropertiesUpdated = ObjectName("EventPropertiesUpdated");

Serializable::Serializable(const ObjectName& ClassName)
   : cClassName(ClassName)
{
}

Serializable::~Serializable()
{
}

void Serializable::registerProperty(const ObjectName& PropertyName, ValueType Type,
   const PropertySetter& Setter, const PropertyGetter& Getter,
   PropertyEditor Editor, void* PropertyDataPtr, uint PropertyDataUInt,
   float MinValue, float MaxValue)
{
   Property sProperty;
   sProperty.Name = PropertyName;
   sProperty.Type = Type;
   sProperty.Setter = Setter;
   sProperty.Getter = Getter;
#if defined (GE_EDITOR_SUPPORT)
   sProperty.Class = cClassName;
   sProperty.Editor = Editor;
   sProperty.DataPtr = PropertyDataPtr;
   sProperty.DataUInt = PropertyDataUInt;
   sProperty.MinValue = MinValue;
   sProperty.MaxValue = MaxValue;
#endif
   vProperties.push_back(sProperty);
}

void Serializable::removeProperty(uint PropertyIndex)
{
   GEAssert(PropertyIndex < (uint)vProperties.size());
   vProperties.erase(vProperties.begin() + PropertyIndex);
}

void Serializable::registerPropertyArray(const ObjectName& PropertyArrayName,
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
}

void Serializable::registerAction(const ObjectName& ActionName, const ActionFunction& Function)
{
   Action sAction;
   sAction.Name = ActionName;
   sAction.Function = Function;
   GEAssert(sAction.Function != nullptr);
   vActions.push_back(sAction);
}

void Serializable::registerEditorAction(const ObjectName& ActionName, const ActionFunction& Function)
{
#if defined (GE_EDITOR_SUPPORT)
   registerAction(ActionName, Function);
#endif
}

const ObjectName& Serializable::getClassName() const
{
   return cClassName;
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

void Serializable::set(const ObjectName& PropertyName, Value& PropertyValue)
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
#if defined (GE_EDITOR_SUPPORT)
   for(uint i = 0; i < vActions.size(); i++)
   {
      if(vActions[i].Name == ActionName)
      {
         vActions[i].Function();
         break;
      }
   }
#endif
}

void Serializable::copy(Serializable* cSource)
{
   GEAssert(cSource->cClassName == cClassName);

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
   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      const char* sPropertyArrayElementName = sPropertyArray.Name.getString().c_str();
      uint iPropertyArrayElementsCount = 0;

      for(const pugi::xml_node& xmlPropertyArrayElement : XmlNode.children(sPropertyArrayElementName))
      {
         sPropertyArray.Add();
         SerializableArrayElement* cPropertyArrayElement = sPropertyArray.Entries->at(iPropertyArrayElementsCount);
         cPropertyArrayElement->loadFromXml(xmlPropertyArrayElement);
         iPropertyArrayElementsCount++;
      }
   }

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

      GEAssert(cProperty);
      GEAssert(cProperty->Setter);

      Value cPropertyValue = Value(cProperty->Type, sValue);
      cProperty->Setter(cPropertyValue);
   }
}

void Serializable::saveToXml(pugi::xml_node& XmlNode) const
{
   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      const char* sPropertyArrayElementName = sPropertyArray.Name.getString().c_str();
      
      for(uint j = 0; j < sPropertyArray.Entries->size(); j++)
      {
         SerializableArrayElement* cPropertyArrayElement = sPropertyArray.Entries->at(j);
         pugi::xml_node xmlPropertyArrayElement = XmlNode.append_child(sPropertyArrayElementName);
         cPropertyArrayElement->saveToXml(xmlPropertyArrayElement);
      }
   }

   for(uint i = 0; i < vProperties.size(); i++)
   {
      const Property& sProperty = vProperties[i];

      if(!sProperty.Setter)
         continue;

      char sValueBuffer[Value::BufferSize];

      sProperty.Getter().toString(sValueBuffer);

      if(sValueBuffer[0] == '\0')
         continue;

      pugi::xml_node xmlProperty = XmlNode.append_child("Property");
      xmlProperty.append_attribute("name").set_value(sProperty.Name.getString().c_str());
      xmlProperty.append_attribute("value").set_value(sValueBuffer);
   }
}

void Serializable::loadFromStream(std::istream& Stream)
{
   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      uint iPropertyArrayElementsCount = (uint)Value::fromStream(ValueType::Byte, Stream).getAsByte();

      for(uint j = 0; j < iPropertyArrayElementsCount; j++)
      {
         sPropertyArray.Add();
         SerializableArrayElement* cPropertyArrayElement = sPropertyArray.Entries->at(j);
         cPropertyArrayElement->loadFromStream(Stream);
      }
   }

   for(uint i = 0; i < vProperties.size(); i++)
   {
      const Property& sProperty = vProperties[i];

      if(sProperty.Setter)
      {
         Value cPropertyValue = Value::fromStream(sProperty.Type, Stream);
         sProperty.Setter(cPropertyValue);
      }
   }
}

void Serializable::saveToStream(std::ostream& Stream)
{
   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];

      for(uint j = 0; j < sPropertyArray.Entries->size(); j++)
      {
         SerializableArrayElement* cArrayElement = sPropertyArray.Entries->at(j);
         cArrayElement->saveToStream(Stream);
      }
   }

   for(uint i = 0; i < vProperties.size(); i++)
   {
      const Property& sProperty = vProperties[i];

      if(sProperty.Setter)
      {
         sProperty.Getter().writeToStream(Stream);
      }
   }
}

void Serializable::xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream)
{
#if defined (GE_EDITOR_SUPPORT)
   for(uint i = 0; i < vPropertyArrays.size(); i++)
   {
      const PropertyArray& sPropertyArray = vPropertyArrays[i];
      const char* sPropertyArrayElementName = sPropertyArray.Name.getString().c_str();

      pugi::xml_object_range<pugi::xml_named_node_iterator> xmlPropertyArrayElement = XmlNode.children(sPropertyArrayElementName);
      GE::byte iPropertyArrayElementsCount = 0;

      for(const pugi::xml_node& xmlPropertyArrayElement : xmlPropertyArrayElement)
         iPropertyArrayElementsCount++;

      Value(iPropertyArrayElementsCount).writeToStream(Stream);

      for(const pugi::xml_node& xmlPropertyArrayElement : xmlPropertyArrayElement)
         sPropertyArray.XmlToStream(xmlPropertyArrayElement, Stream);
   }

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

            bPropertySet = true;
            Stream.write(reinterpret_cast<const char*>(&bPropertySet), 1);

            Value cValue = Value(sProperty.Type, sValue);
            cValue.writeToStream(Stream);
            break;
         }
      }

      if(!bPropertySet)
      {
         Stream.write(reinterpret_cast<const char*>(&bPropertySet), 1);
      }
   }
#endif
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
