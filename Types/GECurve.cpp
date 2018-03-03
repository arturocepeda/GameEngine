
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GECurve.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GECurve.h"
#include "GETypes.h"
#include "Core/GEUtils.h"
#include "Core/GEEvents.h"

using namespace GE;
using namespace GE::Core;


//
//  CurveKey
//
const ObjectName ValuePropertyName = ObjectName("Value");
const uint ValuePropertyIndex = 1;
const ValueType DefaultValueType = ValueType::Vector3;

CurveKey::CurveKey()
   : SerializableArrayElement("CurveKey")
   , cValue(Value::getDefaultValue(DefaultValueType))
   , fTimePosition(0.0f)
{
   GERegisterProperty(Float, TimePosition);
}

CurveKey::~CurveKey()
{
}

void CurveKey::setOwner(Serializable* Owner)
{
   SerializableArrayElement::setOwner(Owner);

   Curve* cCurve = static_cast<Curve*>(Owner);
   ValueType eValueType = cCurve->getValueType();
   bool bValueTypeChanged = eValueType != cValue.getType();
   setValue(Value::getDefaultValue(eValueType));

   if(!bValueTypeChanged)
   {
      refreshValueProperty();
   }
}

float CurveKey::getTimePosition() const
{
   return fTimePosition;
}

const Value& CurveKey::getValue() const
{
   return cValue;
}

void CurveKey::setTimePosition(float TimePosition)
{
   fTimePosition = TimePosition;
}

void CurveKey::setValue(const Value& Val)
{
   bool bValueTypeChanged = Val.getType() != cValue.getType();

   cValue = Val;

   if(bValueTypeChanged)
   {
      refreshValueProperty();
   }
}

void CurveKey::refreshValueProperty()
{
   if(getPropertiesCount() > ValuePropertyIndex)
   {
      removeProperty(ValuePropertyIndex);
   }

   registerProperty(ValuePropertyName, cValue.getType(),
      [this](const GE::Core::Value& v) { cValue = v; },
      [this]()->GE::Core::Value { return cValue; });

   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated);
}



//
//  Curve
//
Curve::Curve(const ObjectName& Name, const ObjectName& GroupName)
   : SerializableResource(Name, GroupName, "Curve")
   , eValueType(ValueType::Vector3)
   , eInterpolationMode(InterpolationMode::Linear)
{
   GERegisterPropertyEnum(ValueType, ValueType);
   GERegisterPropertyEnum(InterpolationMode, InterpolationMode);
   GERegisterPropertyReadonly(Float, Length);
   GERegisterPropertyArray(CurveKey);
}

Curve::~Curve()
{
   GEReleasePropertyArray(CurveKey);
}

ValueType Curve::getValueType() const
{
   return eValueType;
}

InterpolationMode Curve::getInterpolationMode() const
{
   return eInterpolationMode;
}

void Curve::setValueType(ValueType Type)
{
   if(eValueType == Type)
      return;

   eValueType = Type;

   if(getCurveKeyCount() > 0)
   {
      for(uint i = 0; i < getCurveKeyCount(); i++)
      {
         getCurveKey(i)->setValue(Value::getDefaultValue(eValueType));
      }
   }
}

void Curve::setInterpolationMode(InterpolationMode Mode)
{
   eInterpolationMode = Mode;
}

float Curve::getLength() const
{
   const uint iKeysCount = getCurveKeyCount();

   if(iKeysCount > 0)
   {
      return getCurveKeyConst(iKeysCount - 1)->getTimePosition();
   }

   return 0.0f;
}

Value Curve::getValue(float TimePosition)
{
   const uint iKeysCount = getCurveKeyCount();

   if(iKeysCount == 0)
   {
      return Value::getDefaultValue(eValueType);
   }

   if(TimePosition <= getCurveKey(0)->getTimePosition())
   {
      return getCurveKey(0)->getValue();
   }

   if(TimePosition >= getCurveKey(iKeysCount - 1)->getTimePosition())
   {
      return getCurveKey(iKeysCount - 1)->getValue();
   }

   for(uint i = 1; i < iKeysCount; i++)
   {
      CurveKey* cKeyA = getCurveKey(i - 1);
      float fKeyATimePosition = cKeyA->getTimePosition();

      CurveKey* cKeyB = getCurveKey(i);
      float fKeyBTimePosition = cKeyB->getTimePosition();

      if(TimePosition >= fKeyATimePosition && TimePosition <= fKeyBTimePosition)
      {
         Scaler cScaler = Scaler(fKeyATimePosition, fKeyBTimePosition, 0.0f, 1.0f);
         float t = Math::getInterpolationFactor(cScaler.y(TimePosition), eInterpolationMode);

         const Value& cKeyAValue = cKeyA->getValue();
         const Value& cKeyBValue = cKeyB->getValue();

         switch(eValueType)
         {
         case ValueType::Float:
            return Value(Math::getInterpolatedValue(cKeyAValue.getAsFloat(), cKeyBValue.getAsFloat(), t));
         case ValueType::Vector2:
            return Value(Math::getInterpolatedValue(cKeyAValue.getAsVector2(), cKeyBValue.getAsVector2(), t));
         case ValueType::Vector3:
            return Value(Math::getInterpolatedValue(cKeyAValue.getAsVector3(), cKeyBValue.getAsVector3(), t));
         case ValueType::Color:
            return Value(Math::getInterpolatedValue(cKeyAValue.getAsColor(), cKeyBValue.getAsColor(), t));
         default:
            return Value::getDefaultValue(eValueType);
         }
      }
   }

   return Value::getDefaultValue(eValueType);
}
