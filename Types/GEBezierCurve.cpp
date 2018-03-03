
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
#include "Core/GEDevice.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;


//
//  BezierCurveRefPoint
//
BezierCurveRefPoint::BezierCurveRefPoint()
   : SerializableArrayElement("BeizerCurveRefPoint")
{
   GERegisterProperty(Vector3, Value);
}

void BezierCurveRefPoint::setValue(const Vector3& Value)
{
   vValue = Value;
}

const Vector3& BezierCurveRefPoint::getValue() const
{
   return vValue;
}


//
//  BezierCurve
//
BezierCurve::BezierCurve(const ObjectName& Name, const ObjectName& GroupName)
   : SerializableResource(Name, GroupName, "BezierCurve")
{
   GERegisterPropertyArray(RefPoint);
}

BezierCurve::~BezierCurve()
{
   GEReleasePropertyArray(RefPoint);
}

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

Vector3 BezierCurve::getPoint(float T)
{
   uint iRefPointsCount = getRefPointCount();
   GEAssert(iRefPointsCount > 1);

   T = Core::Math::clamp01(T);
   Vector3 vPoint;

   for(uint i = 0; i < iRefPointsCount; i++)
   {
      float fFactor = 
         (float)getBinomialCoefficient(iRefPointsCount - 1, i) *
         Core::Math::pow(1.0f - T, iRefPointsCount - 1 - i) *
         Core::Math::pow(T, i);
      vPoint += getRefPoint(i)->getValue() * fFactor;
   }

   return vPoint;
}
