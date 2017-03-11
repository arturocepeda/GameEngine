
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

#include "GESTLTypes.h"

namespace GE
{
   class BezierCurve
   {
   private:
      GESTLVector(Vector3) vRefPoints;

      int getBinomialCoefficient(int n, int k);

   public:
      void addRefPoint(const Vector3& RefPoint);
      Vector3 getPoint(float T);
   };
}
