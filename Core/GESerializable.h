
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GESerializable.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEValue.h"
#include "GEObject.h"
#include "GEParser.h"
#include "GEObjectManager.h"
#include "Externals/pugixml/pugixml.hpp"

#include <vector>
#include <cstdlib>
#include <functional>

namespace GE { namespace Core
{
   class ObjectName;
   class SerializableArrayElement;

   typedef std::function<Value()> PropertyGetter;
   typedef std::function<void(Value&)> PropertySetter;

   typedef GESTLVector(SerializableArrayElement*) PropertyArrayEntries;
   typedef std::function<void()> PropertyArrayAdd;
   typedef std::function<void(uint)> PropertyArrayRemove;
   typedef std::function<void(const pugi::xml_node&, std::ostream&)> PropertyArrayXmlToStream;

   enum class PropertyEditor
   {
      Default,
      Rotation,
      Enum,
      BitMask
   };

   struct Property
   {
      ObjectName Name;
      ValueType Type;
      PropertyGetter Getter;
      PropertySetter Setter;

#if defined (GE_EDITOR_SUPPORT)
      ObjectName Class;
      PropertyEditor Editor;
      void* DataPtr;
      uint DataUInt;
      float MinValue;
      float MaxValue;
#endif
   };

   struct PropertyArray
   {
      ObjectName Name;
      PropertyArrayEntries* Entries;
      PropertyArrayAdd Add;

#if defined (GE_EDITOR_SUPPORT)
      PropertyArrayRemove Remove;
      PropertyArrayXmlToStream XmlToStream;
#endif
   };

   class Serializable
   {
   private:
      typedef GESTLVector(Property) PropertiesList;
      typedef GESTLVector(PropertyArray) PropertyArraysList;

      PropertiesList vProperties;
      PropertyArraysList vPropertyArrays;

   protected:
      ObjectName cClassName;

      Serializable(const ObjectName& ClassName);
      virtual ~Serializable();

      Property& registerProperty(const ObjectName& PropertyName, ValueType Type,
         const PropertySetter& Setter, const PropertyGetter& Getter,
         PropertyEditor Editor = PropertyEditor::Default,
         void* PropertyDataPtr = 0, uint PropertyDataUInt = 0,
         float MinValue = 0.0f, float MaxValue = 0.0f);
      void removeProperty(uint PropertyIndex);

      PropertyArray& registerPropertyArray(const ObjectName& PropertyArrayName,
         PropertyArrayEntries* PropertyArrayEntries,
         const PropertyArrayAdd& Add, const PropertyArrayRemove& Remove,
         const PropertyArrayXmlToStream& XmlToStream);

   public:
      static const ObjectName EventPropertiesUpdated;

      const ObjectName& getClassName() const;

      uint getPropertiesCount() const;
      const Property& getProperty(uint PropertyIndex) const;
      const Property* getProperty(const ObjectName& PropertyName) const;

      uint getPropertyArraysCount() const;
      const PropertyArray& getPropertyArray(uint PropertyArrayIndex) const;

      Value get(const ObjectName& PropertyName);
      void set(const ObjectName& PropertyName, Value& PropertyValue);

      virtual void copy(Serializable* cSource);

      virtual void loadFromXml(const pugi::xml_node& XmlNode);
      virtual void saveToXml(pugi::xml_node& XmlNode) const;

      virtual void loadFromStream(std::istream& Stream);

      virtual void xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream);
   };


   class SerializableArrayElement : public Serializable
   {
   protected:
      Serializable* cOwner;

      SerializableArrayElement(const ObjectName& ClassName);

   public:
      virtual ~SerializableArrayElement();

      void setOwner(Serializable* Owner);
   };
}}


//
//  Definition of generic properties
//
#define GEPropertyReadonly(PropertyType, PropertyName) \
   GE::Core::Value P_get##PropertyName() { return GE::Core::Value(get##PropertyName()); }

#define GEPropertyWriteonly(PropertyType, PropertyName) \
   void P_set##PropertyName(GE::Core::Value& v) { set##PropertyName(v.getAs##PropertyType()); }

#define GEProperty(PropertyType, PropertyName) \
   GEPropertyReadonly(PropertyType, PropertyName) \
   GEPropertyWriteonly(PropertyType, PropertyName)


//
//  Definition of enum properties
//
#define GEPropertyEnumReadonly(EnumType, PropertyName) \
   GE::Core::Value P_get##PropertyName() { return GE::Core::Value(str##EnumType[(GE::uint)get##PropertyName()]); }

#define GEPropertyEnumWriteonly(EnumType, PropertyName) \
   void P_set##PropertyName(GE::Core::Value& v) \
   { \
      for(GE::uint i = 0; i < (GE::uint)EnumType::Count; i++) \
      { \
         if(strcmp(v.getAsString(), str##EnumType[i]) == 0) \
         { \
            set##PropertyName((EnumType)i); \
            return; \
         } \
      } \
      GEAssert(false); \
   }

#define GEPropertyEnum(EnumType, PropertyName) \
   GEPropertyEnumReadonly(EnumType, PropertyName) \
   GEPropertyEnumWriteonly(EnumType, PropertyName)


//
//  Definition of bit mask properties
//
#define GEPropertyBitMaskReadonly(EnumType, PropertyName) \
   GE::Core::Value P_get##PropertyName() \
   { \
      GE::uint iPropertyValue = get##PropertyName(); \
      char sBuffer[GE::Core::Value::BufferSize]; \
      sBuffer[0] = '\0'; \
      for(GE::uint i = 0; i < (GE::uint)EnumType::Count; i++) \
      { \
         if(GEHasFlag(iPropertyValue, 1 << i)) \
         { \
            sprintf(sBuffer, "%s%s|", sBuffer, str##EnumType[i]); \
         } \
      } \
      GE::uint iBitMaskLength = (GE::uint)strlen(sBuffer); \
      if(iBitMaskLength > 0) sBuffer[iBitMaskLength - 1] = '\0'; \
      return GE::Core::Value(sBuffer); \
   }

#define GEPropertyBitMaskWriteonly(EnumType, PropertyName) \
   void P_set##PropertyName(GE::Core::Value& v) \
   { \
      GE::uint iPropertyValue = 0; \
      const char* sBitMask = v.getAsString(); \
      GE::uint iBitMaskLength = (GE::uint)strlen(sBitMask); \
      if(iBitMaskLength > 0) \
      { \
         char sBuffer[GE::Core::Value::BufferSize]; \
         GE::uint iBufferPosition = 0; \
         for(GE::uint i = 0; i <= iBitMaskLength; i++) \
         { \
            sBuffer[iBufferPosition++] = sBitMask[i]; \
            if(sBitMask[i] != '|' && sBitMask[i] != '\0') continue; \
            sBuffer[iBufferPosition - 1] = '\0'; \
            for(GE::uint j = 0; j < (GE::uint)EnumType::Count; j++) \
            { \
               if(strcmp(sBuffer, str##EnumType[j]) == 0) \
               { \
                  GESetFlag(iPropertyValue, 1 << j); \
                  iBufferPosition = 0; \
                  break; \
               } \
            } \
            GEAssert(iBufferPosition == 0); \
         } \
      } \
      set##PropertyName(iPropertyValue); \
   }

#define GEPropertyBitMask(EnumType, PropertyName) \
   GEPropertyBitMaskReadonly(EnumType, PropertyName) \
   GEPropertyBitMaskWriteonly(EnumType, PropertyName)


//
//  Definition of property arrays
//
#define GEPropertyArray(Class, ClassName) \
   GE::Core::PropertyArrayEntries v##ClassName##List; \
   Class* add##ClassName() \
   { \
      Class* cEntry = GE::Core::Allocator::alloc<Class>(); \
      GEInvokeCtor(Class, cEntry)(); \
      cEntry->setOwner(this); \
      v##ClassName##List.push_back(cEntry); \
      return cEntry; \
   } \
   GE::uint get##ClassName##Count() const \
   { \
      return (GE::uint)v##ClassName##List.size(); \
   } \
   Class* get##ClassName(uint Index) \
   { \
      GEAssert(Index < v##ClassName##List.size()); \
      return static_cast<Class*>(v##ClassName##List[Index]); \
   } \
   void remove##ClassName(uint Index) \
   { \
      GEAssert(Index < v##ClassName##List.size()); \
      Class* cEntry = static_cast<Class*>(v##ClassName##List[Index]); \
      GEInvokeDtor(Class, cEntry); \
      GE::Core::Allocator::free(cEntry); \
      v##ClassName##List.erase(v##ClassName##List.begin() + Index); \
   } \
   void clear##ClassName##List() \
   { \
      for(GE::uint i = 0; i < v##ClassName##List.size(); i++) \
      { \
         Class* cEntry = static_cast<Class*>(v##ClassName##List[i]); \
         GEInvokeDtor(Class, cEntry); \
         GE::Core::Allocator::free(cEntry); \
      } \
      v##ClassName##List.clear(); \
   } \
   void xmlToStream##ClassName(const pugi::xml_node& XmlNode, std::ostream& Stream) \
   { \
      Class cEntry; \
      cEntry.loadFromXml(XmlNode); \
      cEntry.xmlToStream(XmlNode, Stream); \
   }



//
//  Register generic properties
//
#define GERegisterProperty(ClassName, PropertyType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this))

#define GERegisterPropertyMinMax(ClassName, PropertyType, PropertyName, MinValue, MaxValue) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this), \
      PropertyEditor::Default, 0, 0, \
      MinValue, MaxValue)

#define GERegisterPropertyReadonly(ClassName, PropertyType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      nullptr, \
      std::bind(&ClassName::P_get##PropertyName, this))

#define GERegisterPropertyObjectManager(ClassName, PropertyType, PropertyName, ObjectType) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this), \
      PropertyEditor::Default, \
      (void*)GE::Core::ObjectManagers::getInstance()->getObjectRegistry(#ObjectType))

#define GERegisterPropertySpecialEditor(ClassName, PropertyType, PropertyName, Editor) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this), \
      Editor)


//
//  Register enum properties
//
#define GERegisterPropertyEnum(ClassName, EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::String, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this), \
      PropertyEditor::Enum, (void*)str##EnumType, (uint)EnumType::Count)

#define GERegisterPropertyEnumReadonly(ClassName, EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::String, \
      nullptr, \
      std::bind(&ClassName::P_get##PropertyName, this), \
      PropertyEditor::Enum, (void*)str##EnumType, (uint)EnumType::Count)


//
//  Register bit mask properties
//
#define GERegisterPropertyBitMask(ClassName, EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::String, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this), \
      PropertyEditor::BitMask, (void*)str##EnumType, (uint)EnumType::Count)

#define GERegisterPropertyBitMaskReadonly(ClassName, EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::String, \
      nullptr, \
      std::bind(&ClassName::P_get##PropertyName, this), \
      PropertyEditor::BitMask, (void*)str##EnumType, (uint)EnumType::Count)


//
//  Register property array
//
#define GERegisterPropertyArray(ClassName, ArrayElementName) \
   registerPropertyArray(GE::Core::ObjectName(#ArrayElementName), &v##ArrayElementName##List, \
      std::bind(&ClassName::add##ArrayElementName, this), \
      std::bind(&ClassName::remove##ArrayElementName, this, std::placeholders::_1), \
      std::bind(&ClassName::xmlToStream##ArrayElementName, this, std::placeholders::_1, std::placeholders::_2))

#define GEReleasePropertyArray(ClassName, ArrayElementName) \
   clear##ArrayElementName##List()
