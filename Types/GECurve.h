
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GECurve.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Content/GEResource.h"

namespace GE
{
   class CurveKey : public Core::SerializableArrayElement
   {
   private:
      float fTimePosition;
      float fValue;

   public:
      CurveKey();
      ~CurveKey();

      float getTimePosition() const;
      float getValue() const;

      void setTimePosition(float TimePosition);
      void setValue(float Value);
   };


   GESerializableEnum(CurveValueType)
   {
      Default,
      EulerAngleInRadians,
      EulerAngleInDegrees,

      Count
   };


   class Curve : public Content::Resource
   {
   private:
      InterpolationMode eInterpolationMode;
      CurveValueType eValueType;

   public:
      static const Core::ObjectName TypeName;

      Curve(const Core::ObjectName& Name, const Core::ObjectName& GroupName);
      ~Curve();

      InterpolationMode getInterpolationMode() const { return eInterpolationMode; }
      void setInterpolationMode(InterpolationMode Mode) { eInterpolationMode = Mode; }

      CurveValueType getValueType() const { return eValueType; }
      void setValueType(CurveValueType Type) { eValueType = Type; }

      float getLength() const;
      float getValue(float TimePosition);

      GEPropertyArray(CurveKey, CurveKey)
   };
}
