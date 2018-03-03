
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

#include <functional>

namespace GE { namespace Core
{
   class ObjectName;
   class SerializableArrayElement;

   typedef std::function<Value()> PropertyGetter;
   typedef std::function<void(const Value&)> PropertySetter;
   typedef std::function<void()> ActionFunction;

   typedef GESTLVector(SerializableArrayElement*) PropertyArrayEntries;
   typedef std::function<void()> PropertyArrayAdd;
   typedef std::function<void(uint)> PropertyArrayRemove;
   typedef std::function<void(uint, uint)> PropertyArraySwap;
   typedef std::function<void(const pugi::xml_node&, std::ostream&)> PropertyArrayXmlToStream;

   enum class PropertyEditor
   {
      Default,
      Rotation,
      Enum,
      BitMask,
      Script
   };

   enum class PropertyFlags
   {
      EditorOnly = 1 << 0,
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
      Value DefaultValue;
      uint8_t Flags;
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
      PropertyArraySwap Swap;
      PropertyArrayXmlToStream XmlToStream;
#endif
   };

   struct Action
   {
      ObjectName Name;
      ActionFunction Function;
   };

   class Serializable
   {
   private:
      typedef GESTLVector(Property) PropertiesList;
      typedef GESTLVector(PropertyArray) PropertyArraysList;
      typedef GESTLVector(Action) ActionsList;

      PropertiesList vProperties;
      PropertyArraysList vPropertyArrays;
      ActionsList vActions;

   protected:
      ObjectName cClassName;

      Serializable(const ObjectName& ClassName);
      virtual ~Serializable();

      void registerProperty(const ObjectName& PropertyName, ValueType Type,
         const PropertySetter& Setter, const PropertyGetter& Getter,
         PropertyEditor Editor = PropertyEditor::Default, uint8_t Flags = 0,
         void* PropertyDataPtr = 0, uint PropertyDataUInt = 0,
         float MinValue = 0.0f, float MaxValue = 0.0f);
      void removeProperty(uint PropertyIndex);

      void registerPropertyArray(const ObjectName& PropertyArrayName,
         PropertyArrayEntries* PropertyArrayEntries,
         const PropertyArrayAdd& Add, const PropertyArrayRemove& Remove,
         const PropertyArraySwap& Swap, const PropertyArrayXmlToStream& XmlToStream);

      void registerAction(const ObjectName& ActionName, const ActionFunction& Function);
      void removeAction(uint ActionIndex);

   public:
      const ObjectName& getClassName() const;

      uint getPropertiesCount() const;
      const Property& getProperty(uint PropertyIndex) const;
      const Property* getProperty(const ObjectName& PropertyName) const;

      uint getPropertyArraysCount() const;
      const PropertyArray& getPropertyArray(uint PropertyArrayIndex) const;
      const PropertyArray* getPropertyArray(const ObjectName& PropertyArrayName) const;

      uint getActionsCount() const;
      const Action& getAction(uint ActionIndex) const;

      Value get(const ObjectName& PropertyName);
      void set(const ObjectName& PropertyName, const Value& PropertyValue);

      void executeAction(const ObjectName& ActionName);

      virtual void copy(Serializable* cSource);

      virtual void loadFromXml(const pugi::xml_node& XmlNode);
      virtual void saveToXml(pugi::xml_node& XmlNode) const;

      virtual void loadFromStream(std::istream& Stream);
      virtual void saveToStream(std::ostream& Stream) const;

      virtual void xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream);
   };


   class SerializableArrayElement : public Serializable
   {
   protected:
      Serializable* cOwner;

      SerializableArrayElement(const ObjectName& ClassName);

   public:
      virtual ~SerializableArrayElement();

      virtual void setOwner(Serializable* Owner);
   };
}}


//
//  Definition of generic properties
//
#define GEPropertyReadonly(PropertyType, PropertyName) \
   GE::Core::Value P_get##PropertyName() { return GE::Core::Value(get##PropertyName()); }

#define GEPropertyWriteonly(PropertyType, PropertyName) \
   void P_set##PropertyName(const GE::Core::Value& v) { set##PropertyName(v.getAs##PropertyType()); }

#define GEProperty(PropertyType, PropertyName) \
   GEPropertyReadonly(PropertyType, PropertyName) \
   GEPropertyWriteonly(PropertyType, PropertyName)


//
//  Definition of enum properties
//
#define GEPropertyEnumReadonly(EnumType, PropertyName) \
   GE::Core::Value P_get##PropertyName() { return GE::Core::Value((GE::byte)get##PropertyName()); }

#define GEPropertyEnumWriteonly(EnumType, PropertyName) \
   void P_set##PropertyName(const GE::Core::Value& v) \
   { \
      GEAssert(v.getAsByte() < (GE::byte)EnumType::Count); \
      set##PropertyName((EnumType)v.getAsByte()); \
   }

#define GEPropertyEnum(EnumType, PropertyName) \
   GEPropertyEnumReadonly(EnumType, PropertyName) \
   GEPropertyEnumWriteonly(EnumType, PropertyName)


//
//  Definition of bit mask properties
//
#define GEPropertyBitMaskReadonly(EnumType, PropertyName) \
   GEPropertyReadonly(Byte, PropertyName)

#define GEPropertyBitMaskWriteonly(EnumType, PropertyName) \
   GEPropertyWriteonly(Byte, PropertyName)

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
   const Class* get##ClassName##Const(uint Index) const \
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
   void swap##ClassName(uint IndexA, uint IndexB) \
   { \
      GEAssert(IndexA < v##ClassName##List.size()); \
      GEAssert(IndexB < v##ClassName##List.size()); \
      GEAssert(IndexA != IndexB); \
      GE::Core::SerializableArrayElement* cCachedEntry = v##ClassName##List[IndexA]; \
      v##ClassName##List[IndexA] = v##ClassName##List[IndexB]; \
      v##ClassName##List[IndexB] = cCachedEntry; \
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
      cEntry.saveToStream(Stream); \
   }



//
//  Register generic properties
//
#define GERegisterProperty(PropertyType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      [this](const GE::Core::Value& v) { this->P_set##PropertyName(v); }, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); })

#define GERegisterPropertyMinMax(PropertyType, PropertyName, MinValue, MaxValue) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      [this](const GE::Core::Value& v) { this->P_set##PropertyName(v); }, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); }, \
      PropertyEditor::Default, 0, 0, 0, \
      MinValue, MaxValue)

#define GERegisterPropertyReadonly(PropertyType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      nullptr, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); })

#define GERegisterPropertyResource(PropertyType, PropertyName, ObjectType) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      [this](const GE::Core::Value& v) { this->P_set##PropertyName(v); }, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); }, \
      PropertyEditor::Default, 0, \
      (void*)GE::Content::ResourcesManager::getInstance()->getObjectRegistry(#ObjectType))

#define GERegisterPropertySpecialEditor(PropertyType, PropertyName, Editor) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      [this](const GE::Core::Value& v) { this->P_set##PropertyName(v); }, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); }, \
      Editor)

#if defined (GE_EDITOR_SUPPORT)
# define GERegisterPropertyEditorOnly(PropertyType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::PropertyType, \
      [this](const GE::Core::Value& v) { this->P_set##PropertyName(v); }, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); }, \
      PropertyEditor::Default, (uint8_t)PropertyFlags::EditorOnly);
#else
# define GERegisterPropertyEditorOnly(PropertyType, PropertyName)
#endif


//
//  Register enum properties
//
#define GERegisterPropertyEnum(EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::Byte, \
      [this](const GE::Core::Value& v) { this->P_set##PropertyName(v); }, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); }, \
      PropertyEditor::Enum, 0, (void*)str##EnumType, (uint32_t)EnumType::Count)

#define GERegisterPropertyEnumReadonly(EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::Byte, \
      nullptr, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); }, \
      PropertyEditor::Enum, 0, (void*)str##EnumType, (uint32_t)EnumType::Count)


//
//  Register bit mask properties
//
#define GERegisterPropertyBitMask(EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::Byte, \
      [this](const GE::Core::Value& v) { this->P_set##PropertyName(v); }, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); }, \
      PropertyEditor::BitMask, 0, (void*)str##EnumType, (uint)EnumType::Count)

#define GERegisterPropertyBitMaskReadonly(EnumType, PropertyName) \
   registerProperty(GE::Core::ObjectName(#PropertyName), GE::Core::ValueType::Byte, \
      nullptr, \
      [this]()->GE::Core::Value { return this->P_get##PropertyName(); }, \
      PropertyEditor::BitMask, 0, (void*)str##EnumType, (uint)EnumType::Count)


//
//  Register property array
//
#define GERegisterPropertyArray(ArrayElementName) \
   registerPropertyArray(GE::Core::ObjectName(#ArrayElementName), &v##ArrayElementName##List, \
      [this]() { this->add##ArrayElementName(); }, \
      [this](uint i) { this->remove##ArrayElementName(i); }, \
      [this](uint i, uint j) { this->swap##ArrayElementName(i, j); }, \
      [this](const pugi::xml_node& n, std::ostream& o) { this->xmlToStream##ArrayElementName(n, o); })

#define GEReleasePropertyArray(ArrayElementName) \
   clear##ArrayElementName##List()


//
//  Generic array element
//
namespace GE { namespace Core
{
   class GenericVariable : public SerializableArrayElement
   {
   protected:
      ObjectName cName;
      Value cValue;

      GenericVariable(const ObjectName& ClassName);

   public:
      ~GenericVariable();

      const ObjectName& getName() const;
      ValueType getType() const;
      const Value& getValue() const;

      void setName(const ObjectName& Name);
      void setType(ValueType Type);
      void setValue(const Value& Val);

      GEProperty(ObjectName, Name)
      GEPropertyEnum(ValueType, Type)
   };
}}
