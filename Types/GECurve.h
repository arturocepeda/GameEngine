
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


   class Curve : public Content::SerializableResource
   {
   private:
      InterpolationMode eInterpolationMode;

   public:
      Curve(const Core::ObjectName& Name, const Core::ObjectName& GroupName);
      ~Curve();

      InterpolationMode getInterpolationMode() const;
      void setInterpolationMode(InterpolationMode Mode);

      float getLength() const;
      float getValue(float TimePosition);

      GEPropertyArray(CurveKey, CurveKey)
   };
}
