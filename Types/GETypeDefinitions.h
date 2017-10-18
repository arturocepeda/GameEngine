
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
   enum class EnumType

namespace GE
{
   typedef unsigned char byte;
   typedef unsigned int uint;
   typedef unsigned short ushort;
   typedef unsigned char tristate;


   GESerializableEnum(InterpolationMode)
   {
      Linear,
      Quadratic,
      QuadraticInverse,
      Logarithmic,

      Count
   };
}
