
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

   typedef std::function<Value()> PropertyGetter;
   typedef std::function<void(Value&)> PropertySetter;

   struct Property
   {
      ObjectName Name;
      ValueType Type;
      PropertyGetter Getter;
      PropertySetter Setter;

#if defined (GE_EDITOR_SUPPORT)
      ObjectName Class;
      void* DataPtr;
      uint DataUInt;
#endif
   };

   class Serializable
   {
   private:
      typedef GESTLVector(Property) PropertiesList;
      PropertiesList vProperties;

   protected:
      ObjectName cClassName;

      Serializable(const ObjectName& ClassName);

      void registerProperty(const ObjectName& PropertyName, ValueType Type,
         const PropertySetter& Setter, const PropertyGetter& Getter,
         void* PropertyDataPtr = 0, uint PropertyDataUInt = 0);
      void removeProperty(uint PropertyIndex);

   public:
      const ObjectName& getClassName() const;

      uint getPropertiesCount() const;
      const Property& getProperty(uint PropertyIndex) const;
      const Property* getProperty(const ObjectName& PropertyName) const;

      Value get(const ObjectName& PropertyName);
      void set(const ObjectName& PropertyName, Value& PropertyValue);

      virtual void loadFromXml(const pugi::xml_node& XmlNode);
      virtual void saveToXml(pugi::xml_node& XmlNode) const;

      virtual void loadFromStream(std::istream& Stream);

      virtual void xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream);
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
//  Register generic properties
//
#define GERegisterProperty(ClassName, PropertyType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this))

#define GERegisterPropertyReadonly(ClassName, PropertyType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      nullptr, \
      std::bind(&ClassName::P_get##PropertyName, this))

#define GERegisterPropertyObjectManager(ClassName, PropertyType, PropertyName, ObjectType) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this), \
      (void*)GE::Core::ObjectManagers::getInstance()->getObjectRegistry(#ObjectType))


//
//  Register enum properties
//
#define GERegisterPropertyEnum(ClassName, EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::String, \
      std::bind(&ClassName::P_set##PropertyName, this, std::placeholders::_1), \
      std::bind(&ClassName::P_get##PropertyName, this), \
      (void*)str##EnumType, (uint)EnumType::Count)

#define GERegisterPropertyEnumReadonly(ClassName, EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::String, \
      nullptr, \
      std::bind(&ClassName::P_get##PropertyName, this), \
      (void*)str##EnumType, (uint)EnumType::Count)
