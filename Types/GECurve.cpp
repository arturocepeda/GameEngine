
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
CurveKey::CurveKey()
   : SerializableArrayElement("CurveKey")
   , fTimePosition(0.0f)
   , fValue(0.0f)
{
   GERegisterProperty(Float, TimePosition);
   GERegisterProperty(Float, Value);
}

CurveKey::~CurveKey()
{
}

float CurveKey::getTimePosition() const
{
   return fTimePosition;
}

float CurveKey::getValue() const
{
   return fValue;
}

void CurveKey::setTimePosition(float TimePosition)
{
   fTimePosition = TimePosition;
}

void CurveKey::setValue(float Value)
{
   fValue = Value;
}


//
//  Curve
//
const ObjectName Curve::TypeName = ObjectName("Curve");
const char* Curve::SubDir = "Data";
const char* Curve::Extension = "curves";

Curve::Curve(const ObjectName& Name, const ObjectName& GroupName)
   : Resource(Name, GroupName, TypeName)
   , eInterpolationMode(InterpolationMode::Linear)
   , eValueType(CurveValueType::Default)
{
   GERegisterPropertyEnum(InterpolationMode, InterpolationMode);
   GERegisterPropertyEnum(CurveValueType, ValueType);
   GERegisterPropertyReadonly(Float, Length);
   GERegisterPropertyArray(CurveKey);
}

Curve::~Curve()
{
   GEReleasePropertyArray(CurveKey);
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

float Curve::getValue(float TimePosition)
{
   const uint iKeysCount = getCurveKeyCount();

   if(iKeysCount == 0)
   {
      return 0.0f;
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

         float fValueA = cKeyAValue.getAsFloat();
         float fValueB = cKeyBValue.getAsFloat();

         if(eValueType != CurveValueType::Default)
         {
            const float fMaxDiff = eValueType == CurveValueType::EulerAngleInRadians
               ? GE_PI
               : 180.0f;

            if(fabsf(fValueB - fValueA) > fMaxDiff)
            {
               if(fValueB > fValueA)
               {
                  fValueA += fMaxDiff * 2.0f;
               }
               else
               {
                  fValueB += fMaxDiff * 2.0f;
               }
            }
         }

         return Math::getInterpolatedValue(fValueA, fValueB, t);
      }
   }

   return 0.0f;
}
