
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEValue.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEObject.h"
#include "Types/GETypes.h"
#include <iostream>

namespace GE { namespace Core
{
   GESerializableEnum(ValueType)
   {
      Int,
      UInt,
      Float,
      Bool,
      Byte,
      Short,
      String,
      ObjectName,
      Vector2,
      Vector3,
      Color,

      Count
   };


   class Value
   {
   public:
      static const int BufferSize = 44;

      static Value getDefaultValue(ValueType Type);
      static ValueType getValueType(const char* TypeAsString);

   private:
      ValueType eType;
      char sBuffer[BufferSize];

      static const Value DefaultValues[(uint)ValueType::Count];

      Value(ValueType Type);

      void setAsInt(int Val);
      void setAsUInt(uint Val);
      void setAsFloat(float Val);
      void setAsBool(bool Val);
      void setAsByte(byte Val);
      void setAsShort(short Val);
      void setAsString(const char* Val);
      void setAsObjectName(const ObjectName& Val);
      void setAsVector2(const GE::Vector2& Val);
      void setAsVector3(const GE::Vector3& Val);
      void setAsColor(const GE::Color& Val);

   public:
      Value(int IntValue);
      Value(uint UIntValue);
      Value(float FloatValue);
      Value(bool BoolValue);
      Value(byte ByteValue);
      Value(short ShortValue);
      Value(const char* StringValue);
      Value(const ObjectName& ObjectNameValue);
      Value(const GE::Vector2& Vector2Value);
      Value(const GE::Vector3& Vector3Value);
      Value(const GE::Color& ColorValue);

      Value(const Value& Other);
      Value(ValueType Type, const char* ValueAsString);

      ValueType getType() const;
      uint getSize() const;
      const char* getRawData() const;

      int getAsInt() const;
      uint getAsUInt() const;
      float getAsFloat() const;
      bool getAsBool() const;
      byte getAsByte() const;
      short getAsShort() const;
      const char* getAsString() const;
      ObjectName getAsObjectName() const;
      const GE::Vector2& getAsVector2() const;
      const GE::Vector3& getAsVector3() const;
      const GE::Color& getAsColor() const;

      template<typename T> T getAs() const { return (T)0; }
      template<> int getAs<int>() const { return getAsInt(); }
      template<> float getAs<float>() const { return getAsFloat(); }
      template<> bool getAs<bool>() const { return getAsBool(); }
      template<> byte getAs<byte>() const { return getAsByte(); }
      template<> short getAs<short>() const { return getAsShort(); }
      template<> const char* getAs<const char*>() const { return getAsString(); }
      template<> ObjectName getAs<ObjectName>() const { return getAsObjectName(); }
      template<> GE::Vector2 getAs<GE::Vector2>() const { return getAsVector2(); }
      template<> GE::Vector3 getAs<GE::Vector3>() const { return getAsVector3(); }
      template<> GE::Color getAs<GE::Color>() const { return getAsColor(); }

      void toString(char* Buffer) const;

      bool operator==(const Value& Other) const;
      bool operator!=(const Value& Other) const;

      void writeToStream(std::ostream& Stream) const;

      static Value fromStream(ValueType Type, std::istream& Stream);
      static Value fromRawData(ValueType Type, const char* Data, uint DataSize);
   };
}}
