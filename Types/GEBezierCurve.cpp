
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GEBezierCurve.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEBezierCurve.h"
#include "GETypes.h"

using namespace GE;

int BezierCurve::getBinomialCoefficient(int n, int k)
{
   int iResult = 1;

   for(int i = 1; i <= k; i++)
   {
      iResult *= n - (k - i);
      iResult /= i;
   }

   return iResult;
}

void BezierCurve::addRefPoint(const Vector3& RefPoint)
{
   vRefPoints.push_back(RefPoint);
}

Vector3 BezierCurve::getPoint(float T)
{
   uint iRefPointsCount = (uint)vRefPoints.size();
   GEAssert(iRefPointsCount > 1);

   T = Core::Math::clamp01(T);
   Vector3 vPoint;

   for(uint i = 0; i < iRefPointsCount; i++)
   {
      float fFactor = 
         (float)getBinomialCoefficient(iRefPointsCount - 1, i) *
         Core::Math::pow(1.0f - T, iRefPointsCount - 1 - i) *
         Core::Math::pow(T, i);
      vPoint += vRefPoints[i] * fFactor;
   }

   return vPoint;
}
