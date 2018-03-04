
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
Curve::Curve(const ObjectName& Name, const ObjectName& GroupName)
   : SerializableResource(Name, GroupName, "Curve")
   , eInterpolationMode(InterpolationMode::Linear)
{
   GERegisterPropertyEnum(InterpolationMode, InterpolationMode);
   GERegisterPropertyReadonly(Float, Length);
   GERegisterPropertyArray(CurveKey);
}

Curve::~Curve()
{
   GEReleasePropertyArray(CurveKey);
}

InterpolationMode Curve::getInterpolationMode() const
{
   return eInterpolationMode;
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

         return Math::getInterpolatedValue(cKeyAValue.getAsFloat(), cKeyBValue.getAsFloat(), t);
      }
   }

   return 0.0f;
}
