
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEValue.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEValue.h"
#include "GEObject.h"
#include "GEParser.h"
#include <cstring>

using namespace GE;
using namespace GE::Core;

const Value Value::DefaultValues[(uint)ValueType::Count] =
{
   Value((int)0),
   Value((uint)0),
   Value(0.0f),
   Value(false),
   Value((GE::byte)0),
   Value((short)0),
   Value(""),
   Value(ObjectName::Empty),
   Value(Vector2()),
   Value(Vector3()),
   Value(Color())
};

Value Value::getDefaultValue(ValueType Type)
{
   return DefaultValues[(uint)Type];
}

ValueType Value::getValueType(const char* TypeAsString)
{
   int iValueTypeIndex = -1;

   for(int i = 0; i < (int)ValueType::Count; i++)
   {
      if(strcmp(TypeAsString, strValueType[i]) == 0)
      {
         iValueTypeIndex = i;
         break;
      }
   }

   GEAssert(iValueTypeIndex >= 0);
   return (ValueType)iValueTypeIndex;
}


Value::Value()
   : Value(0)
{
}

Value::Value(ValueType Type)
   : eType(Type)
{
}

Value::Value(int IntValue)
   : eType(ValueType::Int)
{
   setAsInt(IntValue);
}

Value::Value(uint UIntValue)
   : eType(ValueType::UInt)
{
   setAsUInt(UIntValue);
}

Value::Value(float FloatValue)
   : eType(ValueType::Float)
{
   setAsFloat(FloatValue);
}

Value::Value(bool BoolValue)
   : eType(ValueType::Bool)
{
   setAsBool(BoolValue);
}

Value::Value(GE::byte ByteValue)
   : eType(ValueType::Byte)
{
   setAsByte(ByteValue);
}

Value::Value(short ShortValue)
   : eType(ValueType::Short)
{
   setAsShort(ShortValue);
}

Value::Value(const char* StringValue)
   : eType(ValueType::String)
{
   setAsString(StringValue);
}

Value::Value(const ObjectName& ObjectNameValue)
   : eType(ValueType::ObjectName)
{
   setAsObjectName(ObjectNameValue);
}

Value::Value(const GE::Vector2& Vector2Value)
   : eType(ValueType::Vector2)
{
   setAsVector2(Vector2Value);
}

Value::Value(const GE::Vector3& Vector3Value)
   : eType(ValueType::Vector3)
{
   setAsVector3(Vector3Value);
}

Value::Value(const GE::Color& ColorValue)
   : eType(ValueType::Color)
{
   setAsColor(ColorValue);
}

Value::Value(const Value& Other)
   : eType(Other.eType)
{
   memcpy(sBuffer, &Other.sBuffer, BufferSize);
}

Value::Value(ValueType Type, const char* ValueAsString)
   : eType(Type)
{
   switch(eType)
   {
   case ValueType::Int:
      setAsInt(Parser::parseInt(ValueAsString));
      break;
   case ValueType::UInt:
      setAsUInt(Parser::parseUInt(ValueAsString));
      break;
   case ValueType::Float:
      setAsFloat(Parser::parseFloat(ValueAsString));
      break;
   case ValueType::Bool:
      setAsBool(Parser::parseBool(ValueAsString));
      break;
   case ValueType::Byte:
      setAsByte((byte)Parser::parseUInt(ValueAsString));
      break;
   case ValueType::Short:
      setAsShort((short)Parser::parseInt(ValueAsString));
      break;
   case ValueType::String:
      setAsString(ValueAsString);
      break;
   case ValueType::ObjectName:
      setAsObjectName(ObjectName(ValueAsString));
      break;
   case ValueType::Vector2:
      setAsVector2(Parser::parseVector2(ValueAsString));
      break;
   case ValueType::Vector3:
      setAsVector3(Parser::parseVector3(ValueAsString));
      break;
   case ValueType::Color:
      setAsColor(Parser::parseColor(ValueAsString));
      break;
   default:
      break;
   }
}

void Value::setAsInt(int IntValue)
{
   memcpy(sBuffer, &IntValue, sizeof(int));
}

void Value::setAsUInt(uint UIntValue)
{
   memcpy(sBuffer, &UIntValue, sizeof(uint));
}

void Value::setAsFloat(float FloatValue)
{
   memcpy(sBuffer, &FloatValue, sizeof(float));
}

void Value::setAsBool(bool BoolValue)
{
   memcpy(sBuffer, &BoolValue, sizeof(bool));
}

void Value::setAsByte(GE::byte ByteValue)
{
   sBuffer[0] = ByteValue;
}

void Value::setAsShort(short ShortValue)
{
   memcpy(sBuffer, &ShortValue, sizeof(short));
}

void Value::setAsString(const char* StringValue)
{
   size_t iSize = 0;

   if(StringValue)
   {
      size_t iLength = strlen(StringValue);
      iSize = iLength < BufferSize ? iLength : BufferSize - 1;
      memcpy(sBuffer, StringValue, iSize);
   }

   sBuffer[iSize] = '\0';
}

void Value::setAsObjectName(const ObjectName& ObjectNameValue)
{
   size_t iStringSize = strlen(ObjectNameValue.getString());
   GEAssert(iStringSize < BufferSize);
   memcpy(sBuffer, ObjectNameValue.getString(), iStringSize);
   sBuffer[iStringSize] = '\0';
}

void Value::setAsVector2(const GE::Vector2& Vector2Value)
{
   memcpy(sBuffer, &Vector2Value, sizeof(GE::Vector2));
}

void Value::setAsVector3(const GE::Vector3& Vector3Value)
{
   memcpy(sBuffer, &Vector3Value, sizeof(GE::Vector3));
}

void Value::setAsColor(const GE::Color& ColorValue)
{
   memcpy(sBuffer, &ColorValue, sizeof(GE::Color));
}

ValueType Value::getType() const
{
   return eType;
}

uint Value::getSize() const
{
   switch(eType)
   {
   case ValueType::Int:
      return (uint)sizeof(int);
   case ValueType::UInt:
      return (uint)sizeof(uint);
   case ValueType::Float:
      return (uint)sizeof(float);
   case ValueType::Bool:
      return (uint)sizeof(bool);
   case ValueType::Byte:
      return (uint)sizeof(byte);
   case ValueType::Short:
      return (uint)sizeof(short);
   case ValueType::String:
   case ValueType::ObjectName:
      return (uint)strlen(sBuffer);
   case ValueType::Vector2:
      return (uint)sizeof(Vector2);
   case ValueType::Vector3:
      return (uint)sizeof(Vector3);
   case ValueType::Color:
      return (uint)sizeof(Color);
   }

   return 0;
}

const char* Value::getRawData() const
{
   return sBuffer;
}

int Value::getAsInt() const
{
   GEAssert(eType == ValueType::Int);
   return *(reinterpret_cast<const int*>(sBuffer));
}

uint Value::getAsUInt() const
{
   GEAssert(eType == ValueType::UInt);
   return *(reinterpret_cast<const uint*>(sBuffer));
}

float Value::getAsFloat() const
{
   GEAssert(eType == ValueType::Float);
   return *(reinterpret_cast<const float*>(sBuffer));
}

bool Value::getAsBool() const
{
   GEAssert(eType == ValueType::Bool);
   return *(reinterpret_cast<const bool*>(sBuffer));
}

GE::byte Value::getAsByte() const
{
   GEAssert(eType == ValueType::Byte);
   return (GE::byte)sBuffer[0];
}

short Value::getAsShort() const
{
   GEAssert(eType == ValueType::Short);
   return *(reinterpret_cast<const short*>(sBuffer));
}

const char* Value::getAsString() const
{
   GEAssert(eType == ValueType::String);
   return sBuffer;
}

ObjectName Value::getAsObjectName() const
{
   GEAssert(eType == ValueType::ObjectName);
   return ObjectName(sBuffer);
}

Vector2 Value::getAsVector2() const
{
   GEAssert(eType == ValueType::Vector2);
   Vector2 vVector;
   memcpy(&vVector, sBuffer, sizeof(Vector2));
   return vVector;
}

Vector3 Value::getAsVector3() const
{
   GEAssert(eType == ValueType::Vector3);
   Vector3 vVector;
   memcpy(&vVector, sBuffer, sizeof(Vector3));
   return vVector;
}

Color Value::getAsColor() const
{
   GEAssert(eType == ValueType::Color);
   Color sColor;
   memcpy(&sColor, sBuffer, sizeof(Color));
   return sColor;
}

void Value::toString(char* Buffer) const
{
   switch(eType)
   {
   case ValueType::Int:
      Parser::writeInt(getAsInt(), Buffer);
      break;
   case ValueType::UInt:
      Parser::writeUInt(getAsUInt(), Buffer);
      break;
   case ValueType::Float:
      Parser::writeFloat(getAsFloat(), Buffer);
      break;
   case ValueType::Bool:
      Parser::writeBool(getAsBool(), Buffer);
      break;
   case ValueType::Byte:
      Parser::writeByte(getAsByte(), Buffer);
      break;
   case ValueType::String:
      strcpy(Buffer, getAsString());
      break;
   case ValueType::ObjectName:
      strcpy(Buffer, getAsObjectName().getString());
      break;
   case ValueType::Vector2:
      Parser::writeVector2(getAsVector2(), Buffer);
      break;
   case ValueType::Vector3:
      Parser::writeVector3(getAsVector3(), Buffer);
      break;
   case ValueType::Color:
      Parser::writeColor(getAsColor(), Buffer);
      break;
   default:
      break;
   }
}

bool Value::operator==(const Value& Other) const
{
   if(eType != Other.eType)
      return false;

   uint iLengthToCheck = 0;

   switch(eType)
   {
   case ValueType::Int:
      iLengthToCheck = sizeof(int);
      break;
   case ValueType::UInt:
      iLengthToCheck = sizeof(uint);
      break;
   case ValueType::Float:
      iLengthToCheck = sizeof(float);
      break;
   case ValueType::Bool:
      iLengthToCheck = sizeof(bool);
      break;
   case ValueType::Byte:
      iLengthToCheck = sizeof(byte);
      break;
   case ValueType::Short:
      iLengthToCheck = sizeof(short);
      break;
   case ValueType::String:
   case ValueType::ObjectName:
      iLengthToCheck = (uint)strlen(sBuffer);
      break;
   case ValueType::Vector2:
      iLengthToCheck = sizeof(Vector2);
      break;
   case ValueType::Vector3:
      iLengthToCheck = sizeof(Vector3);
      break;
   case ValueType::Color:
      iLengthToCheck = sizeof(Color);
      break;
   default:
      break;
   }

   if(eType == ValueType::String || eType == ValueType::ObjectName)
   {
      if(strlen(Other.sBuffer) != iLengthToCheck)
         return false;
   }

   return memcmp(sBuffer, Other.sBuffer, iLengthToCheck) == 0;
}

bool Value::operator!=(const Value& Other) const
{
   return !operator==(Other);
}

void Value::writeToStream(std::ostream& Stream) const
{
   switch(eType)
   {
   case ValueType::Int:
      {
         Stream.write(sBuffer, sizeof(int));
      }
      break;
   case ValueType::UInt:
      {
         Stream.write(sBuffer, sizeof(uint));
      }
      break;
   case ValueType::Float:
      {
         Stream.write(sBuffer, sizeof(float));
      }
      break;
   case ValueType::Bool:
      {
         Stream.write(sBuffer, sizeof(bool));
      }
      break;
   case ValueType::Byte:
      {
         Stream.write(sBuffer, sizeof(byte));
      }
      break;
   case ValueType::Short:
      {
         Stream.write(sBuffer, sizeof(short));
      }
      break;
   case ValueType::String:
   case ValueType::ObjectName:
      {
         byte iStringLength = (byte)strlen(sBuffer);
         Stream.write(reinterpret_cast<char*>(&iStringLength), sizeof(byte));

         for(uint i = 0; i < (uint)iStringLength; i++)
         {
            byte bEncodedValue = *(sBuffer + i) + 128;
            Stream.write(reinterpret_cast<char*>(&bEncodedValue), sizeof(byte));
         }
      }
      break;
   case ValueType::Vector2:
      {
         Stream.write(sBuffer, sizeof(Vector2));
      }
      break;
   case ValueType::Vector3:
      {
         Stream.write(sBuffer, sizeof(Vector3));
      }
      break;
   case ValueType::Color:
      {
         Stream.write(sBuffer, sizeof(Color));
      }
      break;
   default:
      break;
   }
}

Value Value::fromStream(ValueType Type, std::istream& Stream)
{
   Value cValue(Type);

   switch(Type)
   {
   case ValueType::Int:
      {
         Stream.read(cValue.sBuffer, sizeof(int));
      }
      break;
   case ValueType::UInt:
      {
         Stream.read(cValue.sBuffer, sizeof(uint));
      }
      break;
   case ValueType::Float:
      {
         Stream.read(cValue.sBuffer, sizeof(float));
      }
      break;
   case ValueType::Bool:
      {
         Stream.read(cValue.sBuffer, sizeof(bool));
      }
      break;
   case ValueType::Byte:
      {
         Stream.read(cValue.sBuffer, sizeof(byte));
      }
      break;
   case ValueType::Short:
      {
         Stream.read(cValue.sBuffer, sizeof(short));
      }
      break;
   case ValueType::String:
   case ValueType::ObjectName:
      {
         byte iStringLength;
         Stream.read(reinterpret_cast<char*>(&iStringLength), sizeof(byte));

         for(uint i = 0; i < (uint)iStringLength; i++)
         {
            byte bEncodedValue;
            Stream.read(reinterpret_cast<char*>(&bEncodedValue), sizeof(byte));
            cValue.sBuffer[i] = bEncodedValue + 128;
         }

         cValue.sBuffer[iStringLength] = '\0';
      }
      break;
   case ValueType::Vector2:
      {
         Stream.read(cValue.sBuffer, sizeof(Vector2));
      }
      break;
   case ValueType::Vector3:
      {
         Stream.read(cValue.sBuffer, sizeof(Vector3));
      }
      break;
   case ValueType::Color:
      {
         Stream.read(cValue.sBuffer, sizeof(Color));
      }
      break;
   default:
      break;
   }

   return cValue;
}

Value Value::fromRawData(ValueType Type, const char* Data, uint DataSize)
{
   Value cValue(Type);

   memcpy(cValue.sBuffer, Data, DataSize);

   return cValue;
}
