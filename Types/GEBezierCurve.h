
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

#include "Core/GESerializable.h"

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


   class BezierCurve : public Core::Serializable
   {
   private:
      int getBinomialCoefficient(int n, int k);

   public:
      BezierCurve();
      ~BezierCurve();

      void loadFromFile(const char* FileName);

      Vector3 getPoint(float T);

      GEPropertyArray(BezierCurveRefPoint, RefPoint)
   };
}
