
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEGeometry.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GERotation.h"
#include "Types/GEMatrix.h"

namespace GE { namespace Core
{
   class Geometry
   {
   public:
      static void createTRSMatrix(const Vector3& T, const Rotation& R, const Vector3& S, Matrix4* Out);
      static void extractTRSFromMatrix(const Matrix4& M, Vector3* OutT, Rotation* OutR, Vector3* OutS);
   };
}}
