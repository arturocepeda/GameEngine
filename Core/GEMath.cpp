
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

#include "Types/GEVector.h"
#include "Types/GEQuaternion.h"
#include "Types/GEColor.h"

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

float Math::getInterpolationFactor(float T, InterpolationMode Mode)
{
   switch(Mode)
   {
      case InterpolationMode::Linear:
      {
         return T;
      }
      case InterpolationMode::Quadratic:
      {
         return T * T;
      }
      case InterpolationMode::QuadraticInverse:
      {
         const float tMinus1 = T - 1.0f;
         return 1.0f - (tMinus1 * tMinus1);
      }
      case InterpolationMode::Cubic:
      {
         return T * T * T;
      }
      case InterpolationMode::CubicInverse:
      {
         const float tMinus1 = T - 1.0f;
         return 1.0f - fabsf(tMinus1 * tMinus1 * tMinus1);
      }
      case InterpolationMode::Quartic:
      {
         return T * T * T * T;
      }
      case InterpolationMode::QuarticInverse:
      {
         const float tMinus1 = T - 1.0f;
         return 1.0f - (tMinus1 * tMinus1 * tMinus1 * tMinus1);
      }
      case InterpolationMode::Quintic:
      {
         return T * T * T * T * T;
      }
      case InterpolationMode::QuinticInverse:
      {
         const float tMinus1 = T - 1.0f;
         return 1.0f - fabsf(tMinus1 * tMinus1 * tMinus1 * tMinus1 * tMinus1);
      }
      case InterpolationMode::Logarithmic:
      {
         //
         //  log(1) = 0 and log(e) = 1, so we need to map [0, 1] to [1, e] and then calculate the logarithm
         //
         float fMappedT = 1.0f + T * (GE_E - 1.0f);
         return log(fMappedT);
      }
   }

   return T;
}

float Math::getInterpolatedValue(float fStartValue, float fEndValue, float fFactor)
{
   return Math::lerp(fStartValue, fEndValue, fFactor);
}

Vector2 Math::getInterpolatedValue(const Vector2& vStartValue, const Vector2& vEndValue, float fFactor)
{
   return Vector2::lerp(vStartValue, vEndValue, fFactor);
}

Vector3 Math::getInterpolatedValue(const Vector3& vStartValue, const Vector3& vEndValue, float fFactor)
{
   return Vector3::lerp(vStartValue, vEndValue, fFactor);
}

Color Math::getInterpolatedValue(const Color& sStartValue, const Color& sEndValue, float fFactor)
{
   return Color::lerp(sStartValue, sEndValue, fFactor);
}

Quaternion Math::getInterpolatedValue(const Quaternion& qStartValue, const Quaternion& qEndValue, float fFactor)
{
   return Quaternion::slerp(qStartValue, qEndValue, fFactor);
}
