
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GETypeDefinitions.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#define GEHasFlag(Value, Flag)  ((Value & (int)Flag) > 0)
#define GESetFlag(Value, Flag)  (Value |= (int)Flag)
#define GEResetFlag(Value, Flag)  (Value &= ~((int)Flag))

#define GESerializableEnum(EnumType) \
   extern const char* str##EnumType[]; \
   enum class EnumType : unsigned char

namespace GE
{
   typedef unsigned char byte;
   typedef unsigned int uint;
   typedef unsigned short ushort;
   typedef unsigned char tristate;


   struct Point
   {
      int X;
      int Y;

      Point()
         : X(0)
         , Y(0)
      {
      }

      Point(int pX, int pY)
         : X(pX)
         , Y(pY)
      {
      }
   };


   GESerializableEnum(InterpolationMode)
   {
      Linear,
      Quadratic,
      QuadraticInverse,
      Cubic,
      CubicInverse,
      Quartic,
      QuarticInverse,
      Quintic,
      QuinticInverse,
      Logarithmic,

      Count
   };
}
