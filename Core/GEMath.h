
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
   };
}}
