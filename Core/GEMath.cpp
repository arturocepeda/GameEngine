
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEMath.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEMath.h"
#include "GEConstants.h"
#include <cmath>

using namespace GE;
using namespace GE::Core;

float Math::lerp(float ValueA, float ValueB, float T)
{
   float fDiff = ValueB - ValueA;
   return ValueA + (fDiff * T);
}

float Math::clamp(float Value, float Min, float Max)
{
   if(Value < Min)
      return Min;

   if(Value > Max)
      return Max;

   return Value;
}

float Math::clamp01(float Value)
{
   if(Value < 0.0f)
      return 0.0f;

   if(Value > 1.0f)
      return 1.0f;

   return Value;
}

float Math::pow(float Base, uint Exp)
{
   float fValue = 1.0f;

   for(uint i = 1; i <= Exp; i++)
      fValue *= Base;

   return fValue;
}

float Math::getSimplifiedAngle(float AngleInRadians)
{
   while(AngleInRadians > GE_PI)
      AngleInRadians -= GE_DOUBLEPI;

   while(AngleInRadians < -GE_PI)
      AngleInRadians += GE_DOUBLEPI;

   return AngleInRadians;
}
