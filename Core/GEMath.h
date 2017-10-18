
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEMath.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypeDefinitions.h"

namespace GE
{
   struct Vector2;
   struct Vector3;
   struct Quaternion;
   struct Color;
}

namespace GE { namespace Core
{
   class Math
   {
   public:
      static float lerp(float ValueA, float ValueB, float T);
      static float clamp(float Value, float Min, float Max);
      static float clamp01(float Value);
      static float pow(float Base, uint Exp);

      static float getSimplifiedAngle(float AngleInRadians);

      static float getInterpolationFactor(float T, InterpolationMode Mode);

      static float getInterpolatedValue(float fStartValue, float fEndValue, float fFactor);
      static Vector2 getInterpolatedValue(const Vector2& vStartValue, const Vector2& vEndValue, float fFactor);
      static Vector3 getInterpolatedValue(const Vector3& vStartValue, const Vector3& vEndValue, float fFactor);
      static Color getInterpolatedValue(const Color& sStartValue, const Color& sEndValue, float fFactor);
      static Quaternion getInterpolatedValue(const Quaternion& qStartValue, const Quaternion& qEndValue, float fFactor);
   };
}}
