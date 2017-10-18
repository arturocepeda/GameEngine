
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
      Core::Value cValue;

      void refreshValueProperty();

   public:
      CurveKey();
      ~CurveKey();

      virtual void setOwner(Core::Serializable* Owner) override;

      float getTimePosition() const;
      const Core::Value& getValue() const;

      void setTimePosition(float TimePosition);
      void setValue(const Core::Value& Val);

      GEProperty(Float, TimePosition)
   };


   class Curve : public Content::SerializableResource
   {
   private:
      Core::ValueType eValueType;
      InterpolationMode eInterpolationMode;

   public:
      Curve(const Core::ObjectName& Name, const Core::ObjectName& GroupName);
      ~Curve();

      Core::ValueType getValueType() const;
      InterpolationMode getInterpolationMode() const;

      void setValueType(Core::ValueType Type);
      void setInterpolationMode(InterpolationMode Mode);

      float getLength();
      Core::Value getValue(float TimePosition);

      GEPropertyEnum(Core::ValueType, ValueType)
      GEPropertyEnum(InterpolationMode, InterpolationMode)

      GEPropertyArray(CurveKey, CurveKey)
   };
}
