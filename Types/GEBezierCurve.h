
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GEBezierCurve.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Content/GEResource.h"

namespace GE
{
   class BezierCurveRefPoint : public Core::SerializableArrayElement
   {
   private:
      Vector3 vValue;

   public:
      BezierCurveRefPoint();

      void setValue(const Vector3& Value);
      const Vector3& getValue() const;

      GEProperty(Vector3, Value)
   };


   class BezierCurve : public Content::SerializableResource
   {
   private:
      int getBinomialCoefficient(int n, int k);

   public:
      BezierCurve(const Core::ObjectName& Name, const Core::ObjectName& GroupName);
      ~BezierCurve();

      Vector3 getPoint(float T);

      GEPropertyArray(BezierCurveRefPoint, RefPoint)
   };
}
