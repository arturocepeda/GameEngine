
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

Property& Serializable::registerProperty(const ObjectName& PropertyName, ValueType Type,
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
   return vProperties.back();
}

void Serializable::removeProperty(uint PropertyIndex)
{
   GEAssert(PropertyIndex < (uint)vProperties.size());
   vProperties.erase(vProperties.begin() + PropertyIndex);
}

PropertyArray& Serializable::registerPropertyArray(const ObjectName& PropertyArrayName,
   PropertyArrayEntries* PropertyArrayEntries,
   const PropertyArrayAdd& Add, const PropertyArrayRemove& Remove)
{
   PropertyArray sPropertyArray;
   sPropertyArray.Name = PropertyArrayName;
   sPropertyArray.Entries = PropertyArrayEntries;
   sPropertyArray.Add = Add;
   sPropertyArray.Remove = Remove;
   vPropertyArrays.push_back(sPropertyArray);
   return vPropertyArrays.back();
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

      GEAssert(cProperty);
      GEAssert(cProperty->Setter);

      Value cPropertyValue = Value(cProperty->Type, sValue);
      cProperty->Setter(cPropertyValue);
   }
}

void Serializable::saveToXml(pugi::xml_node& XmlNode) const
{
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
   for(uint i = 0; i < vProperties.size(); i++)
   {
      const Property& sProperty = vProperties[i];

      if(sProperty.Setter)
      {
         bool bPropertySet;
         Stream.read(reinterpret_cast<char*>(&bPropertySet), 1);

         if(bPropertySet)
         {
            Value cPropertyValue = Value::fromStream(sProperty.Type, Stream);
            sProperty.Setter(cPropertyValue);
         }
      }
   }
}

void Serializable::xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream)
{
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
