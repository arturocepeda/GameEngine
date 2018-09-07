
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GEQuaternion.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEConstants.h"

#include <cmath>

namespace GE
{
   struct Quaternion
   {
      float X;
      float Y;
      float Z;
      float W;

      Quaternion()
         : X(0.0f)
         , Y(0.0f)
         , Z(0.0f)
         , W(1.0f)
      {
      }

      Quaternion(float qX, float qY, float qZ, float qW)
         : X(qX)
         , Y(qY)
         , Z(qZ)
         , W(qW)
      {
      }
      
      void normalize()
      {
         float fNorm = sqrt(X * X + Y * Y + Z * Z + W * W);

         X /= fNorm;
         Y /= fNorm;
         Z /= fNorm;
         W /= fNorm;
      }

      float norm() const
      {
         return sqrt(X * X + Y * Y + Z * Z + W * W);
      }

      Quaternion conjugate() const
      {
         return Quaternion(-X, -Y, -Z, W);
      }

      Quaternion inverse() const
      {
         Quaternion qInverse = conjugate();
         qInverse.normalize();
         return qInverse;
      }

      bool isIdentity() const
      {
         return
            fabsf(X) < GE_EPSILON &&
            fabsf(Y) < GE_EPSILON &&
            fabsf(Z) < GE_EPSILON &&
            W > (1.0f - GE_EPSILON) &&
            W < (1.0f + GE_EPSILON);
      }

      bool equals(const Quaternion& other) const
      {
         return
            fabsf(other.X - X) < GE_EPSILON &&
            fabsf(other.Y - Y) < GE_EPSILON &&
            fabsf(other.Z - Z) < GE_EPSILON &&
            fabsf(other.W - W) < GE_EPSILON;
      }

      Quaternion operator*(const Quaternion& Other) const
      {
         return Quaternion(
            X * Other.W + Y * Other.Z - Z * Other.Y + W * Other.X,
            -X * Other.Z + Y * Other.W + Z * Other.X + W * Other.Y,
            X * Other.Y - Y * Other.X + Z * Other.W + W * Other.Z,
            -X * Other.X - Y * Other.Y - Z * Other.Z + W * Other.W);
      }

      Quaternion& operator*=(const Quaternion& Other)
      {
         X = X * Other.W + Y * Other.Z - Z * Other.Y + W * Other.X;
         Y = -X * Other.Z + Y * Other.W + Z * Other.X + W * Other.Y;
         Z = X * Other.Y - Y * Other.X + Z * Other.W + W * Other.Z;
         W = -X * Other.X - Y * Other.Y - Z * Other.Z + W * Other.W;
         return *this;
      }

      static Quaternion slerp(const Quaternion& Q1, const Quaternion& Q2, float fFactor)
      {
         Quaternion qResult;
         float fCosHalfTheta = Q1.W * Q2.W + Q1.X * Q2.X + Q1.Y * Q2.Y + Q1.Z * Q2.Z;

         if(fabsf(fCosHalfTheta) >= 1.0f)
            return Q1;

         Quaternion qQ1N = Q1;

         if(fCosHalfTheta < 0.0f)
         {
            qQ1N.X = -Q1.X;
            qQ1N.Y = -Q1.Y;
            qQ1N.Z = -Q1.Z;
            qQ1N.W = -Q1.W;
         }

         float fSinHalfTheta = sqrtf(1.0f - fCosHalfTheta*fCosHalfTheta);

         if(fabsf(fSinHalfTheta) < 0.001f)
         {
            qResult.W = (qQ1N.W * 0.5f + Q2.W * 0.5f);
            qResult.X = (qQ1N.X * 0.5f + Q2.X * 0.5f);
            qResult.Y = (qQ1N.Y * 0.5f + Q2.Y * 0.5f);
            qResult.Z = (qQ1N.Z * 0.5f + Q2.Z * 0.5f);

            return qResult;
         }

         float fHalfTheta = acosf(fCosHalfTheta);

         float fRatioA = sin((1.0f - fFactor) * fHalfTheta) / fSinHalfTheta;
         float fRatioB = sin(fFactor * fHalfTheta) / fSinHalfTheta; 

         qResult.W = (qQ1N.W * fRatioA + Q2.W * fRatioB);
         qResult.X = (qQ1N.X * fRatioA + Q2.X * fRatioB);
         qResult.Y = (qQ1N.Y * fRatioA + Q2.Y * fRatioB);
         qResult.Z = (qQ1N.Z * fRatioA + Q2.Z * fRatioB);

         return qResult;
      }
   };
}
