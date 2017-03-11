
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GETypes.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GETypeDefinitions.h"
#include "GEVector.h"
#include "GEMatrix.h"
#include "GEQuaternion.h"
#include "GEColor.h"
#include "GERotation.h"

#include <cassert>

#define GEAssert(Expr)  assert(Expr)

#define GEHasFlag(Value, Flag)  ((Value & Flag) > 0)
#define GESetFlag(Value, Flag)  (Value |= Flag)
#define GEResetFlag(Value, Flag)  (Value &= ~Flag)

#define GESerializableEnum(EnumType) \
   extern const char* str##EnumType[]; \
   enum class EnumType

namespace GE
{
   GESerializableEnum(Alignment)
   {
      TopLeft,
      TopCenter,
      TopRight,
      MiddleLeft,
      MiddleCenter,
      MiddleRight,
      BottomLeft,
      BottomCenter,
      BottomRight,

      Count
   };
}
