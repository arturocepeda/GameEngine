
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
   };


   class BezierCurve : public Content::Resource
   {
   private:
      int getBinomialCoefficient(int n, int k);

   public:
      static const Core::ObjectName TypeName;

      BezierCurve(const Core::ObjectName& Name, const Core::ObjectName& GroupName);
      ~BezierCurve();

      Vector3 getPoint(float T);

      GEPropertyArray(BezierCurveRefPoint, RefPoint)
   };
}
