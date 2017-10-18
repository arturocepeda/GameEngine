
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

namespace GE
{
   GESerializableEnum(Alignment)
   {
      None,
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
