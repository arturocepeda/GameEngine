
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GEMatrix.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include <cmath>
#include "Core/GEPlatform.h"
#include "GEVector.h"

namespace GE
{
   struct Matrix3
   {
      float m[9];
   };
   
   struct Matrix4
   {
      float m[16];
   };


   //
   //  Matrix convention
   //
#if defined (GE_RENDERING_API_DIRECTX)

# define GE_M3_1_1    0
# define GE_M3_2_1    3
# define GE_M3_3_1    6
# define GE_M3_1_2    1
# define GE_M3_2_2    4
# define GE_M3_3_2    7
# define GE_M3_1_3    2
# define GE_M3_2_3    5
# define GE_M3_3_3    8

# define GE_M4_1_1    0
# define GE_M4_2_1    4
# define GE_M4_3_1    8
# define GE_M4_4_1   12
# define GE_M4_1_2    1
# define GE_M4_2_2    5
# define GE_M4_3_2    9
# define GE_M4_4_2   13
# define GE_M4_1_3    2
# define GE_M4_2_3    6
# define GE_M4_3_3   10
# define GE_M4_4_3   14
# define GE_M4_1_4    3
# define GE_M4_2_4    7
# define GE_M4_3_4   11
# define GE_M4_4_4   15

#elif defined (GE_RENDERING_API_OPENGL)

# define GE_M3_1_1    0
# define GE_M3_2_1    1
# define GE_M3_3_1    2
# define GE_M3_1_2    3
# define GE_M3_2_2    4
# define GE_M3_3_2    5
# define GE_M3_1_3    6
# define GE_M3_2_3    7
# define GE_M3_3_3    8

# define GE_M4_1_1    0
# define GE_M4_2_1    1
# define GE_M4_3_1    2
# define GE_M4_4_1    3
# define GE_M4_1_2    4
# define GE_M4_2_2    5
# define GE_M4_3_2    6
# define GE_M4_4_2    7
# define GE_M4_1_3    8
# define GE_M4_2_3    9
# define GE_M4_3_3   10
# define GE_M4_4_3   11
# define GE_M4_1_4   12
# define GE_M4_2_4   13
# define GE_M4_3_4   14
# define GE_M4_4_4   15

#endif


   //
   //  Matrix generation
   //
   inline void Matrix4GetMatrix3(const Matrix4& M, Matrix3* Out)
   {
      Out->m[GE_M3_1_1] = M.m[GE_M4_1_1];
      Out->m[GE_M3_2_1] = M.m[GE_M4_2_1];
      Out->m[GE_M3_3_1] = M.m[GE_M4_3_1];
   
      Out->m[GE_M3_1_2] = M.m[GE_M4_1_2];
      Out->m[GE_M3_2_2] = M.m[GE_M4_2_2];
      Out->m[GE_M3_3_2] = M.m[GE_M4_3_2];
   
      Out->m[GE_M3_1_3] = M.m[GE_M4_1_3];
      Out->m[GE_M3_2_3] = M.m[GE_M4_2_3];
      Out->m[GE_M3_3_3] = M.m[GE_M4_3_3];
   }

   inline void Matrix4Multiply(const Matrix4& M1, const Matrix4& M2, Matrix4* Out)
   {
      Out->m[GE_M4_1_1] = M1.m[GE_M4_1_1] * M2.m[GE_M4_1_1] + M1.m[GE_M4_1_2] * M2.m[GE_M4_2_1] + M1.m[GE_M4_1_3] * M2.m[GE_M4_3_1] + M1.m[GE_M4_1_4] * M2.m[GE_M4_4_1];
      Out->m[GE_M4_1_2] = M1.m[GE_M4_1_1] * M2.m[GE_M4_1_2] + M1.m[GE_M4_1_2] * M2.m[GE_M4_2_2] + M1.m[GE_M4_1_3] * M2.m[GE_M4_3_2] + M1.m[GE_M4_1_4] * M2.m[GE_M4_4_2];
      Out->m[GE_M4_1_3] = M1.m[GE_M4_1_1] * M2.m[GE_M4_1_3] + M1.m[GE_M4_1_2] * M2.m[GE_M4_2_3] + M1.m[GE_M4_1_3] * M2.m[GE_M4_3_3] + M1.m[GE_M4_1_4] * M2.m[GE_M4_4_3];
      Out->m[GE_M4_1_4] = M1.m[GE_M4_1_1] * M2.m[GE_M4_1_4] + M1.m[GE_M4_1_2] * M2.m[GE_M4_2_4] + M1.m[GE_M4_1_3] * M2.m[GE_M4_3_4] + M1.m[GE_M4_1_4] * M2.m[GE_M4_4_4];
   
      Out->m[GE_M4_2_1] = M1.m[GE_M4_2_1] * M2.m[GE_M4_1_1] + M1.m[GE_M4_2_2] * M2.m[GE_M4_2_1] + M1.m[GE_M4_2_3] * M2.m[GE_M4_3_1] + M1.m[GE_M4_2_4] * M2.m[GE_M4_4_1];
      Out->m[GE_M4_2_2] = M1.m[GE_M4_2_1] * M2.m[GE_M4_1_2] + M1.m[GE_M4_2_2] * M2.m[GE_M4_2_2] + M1.m[GE_M4_2_3] * M2.m[GE_M4_3_2] + M1.m[GE_M4_2_4] * M2.m[GE_M4_4_2];
      Out->m[GE_M4_2_3] = M1.m[GE_M4_2_1] * M2.m[GE_M4_1_3] + M1.m[GE_M4_2_2] * M2.m[GE_M4_2_3] + M1.m[GE_M4_2_3] * M2.m[GE_M4_3_3] + M1.m[GE_M4_2_4] * M2.m[GE_M4_4_3];
      Out->m[GE_M4_2_4] = M1.m[GE_M4_2_1] * M2.m[GE_M4_1_4] + M1.m[GE_M4_2_2] * M2.m[GE_M4_2_4] + M1.m[GE_M4_2_3] * M2.m[GE_M4_3_4] + M1.m[GE_M4_2_4] * M2.m[GE_M4_4_4];
   
      Out->m[GE_M4_3_1] = M1.m[GE_M4_3_1] * M2.m[GE_M4_1_1] + M1.m[GE_M4_3_2] * M2.m[GE_M4_2_1] + M1.m[GE_M4_3_3] * M2.m[GE_M4_3_1] + M1.m[GE_M4_3_4] * M2.m[GE_M4_4_1];
      Out->m[GE_M4_3_2] = M1.m[GE_M4_3_1] * M2.m[GE_M4_1_2] + M1.m[GE_M4_3_2] * M2.m[GE_M4_2_2] + M1.m[GE_M4_3_3] * M2.m[GE_M4_3_2] + M1.m[GE_M4_3_4] * M2.m[GE_M4_4_2];
      Out->m[GE_M4_3_3] = M1.m[GE_M4_3_1] * M2.m[GE_M4_1_3] + M1.m[GE_M4_3_2] * M2.m[GE_M4_2_3] + M1.m[GE_M4_3_3] * M2.m[GE_M4_3_3] + M1.m[GE_M4_3_4] * M2.m[GE_M4_4_3];
      Out->m[GE_M4_3_4] = M1.m[GE_M4_3_1] * M2.m[GE_M4_1_4] + M1.m[GE_M4_3_2] * M2.m[GE_M4_2_4] + M1.m[GE_M4_3_3] * M2.m[GE_M4_3_4] + M1.m[GE_M4_3_4] * M2.m[GE_M4_4_4];
   
      Out->m[GE_M4_4_1] = M1.m[GE_M4_4_1] * M2.m[GE_M4_1_1] + M1.m[GE_M4_4_2] * M2.m[GE_M4_2_1] + M1.m[GE_M4_4_3] * M2.m[GE_M4_3_1] + M1.m[GE_M4_4_4] * M2.m[GE_M4_4_1];
      Out->m[GE_M4_4_2] = M1.m[GE_M4_4_1] * M2.m[GE_M4_1_2] + M1.m[GE_M4_4_2] * M2.m[GE_M4_2_2] + M1.m[GE_M4_4_3] * M2.m[GE_M4_3_2] + M1.m[GE_M4_4_4] * M2.m[GE_M4_4_2];
      Out->m[GE_M4_4_3] = M1.m[GE_M4_4_1] * M2.m[GE_M4_1_3] + M1.m[GE_M4_4_2] * M2.m[GE_M4_2_3] + M1.m[GE_M4_4_3] * M2.m[GE_M4_3_3] + M1.m[GE_M4_4_4] * M2.m[GE_M4_4_3];
      Out->m[GE_M4_4_4] = M1.m[GE_M4_4_1] * M2.m[GE_M4_1_4] + M1.m[GE_M4_4_2] * M2.m[GE_M4_2_4] + M1.m[GE_M4_4_3] * M2.m[GE_M4_3_4] + M1.m[GE_M4_4_4] * M2.m[GE_M4_4_4];
   }

   inline void Matrix4Multiply(const Matrix4& M, float Factor, Matrix4* Out)
   {
      Out->m[ 0] = M.m[ 0] * Factor;
      Out->m[ 1] = M.m[ 1] * Factor;
      Out->m[ 2] = M.m[ 2] * Factor;
      Out->m[ 3] = M.m[ 3] * Factor;
      Out->m[ 4] = M.m[ 4] * Factor;
      Out->m[ 5] = M.m[ 5] * Factor;
      Out->m[ 6] = M.m[ 6] * Factor;
      Out->m[ 7] = M.m[ 7] * Factor;
      Out->m[ 8] = M.m[ 8] * Factor;
      Out->m[ 9] = M.m[ 9] * Factor;
      Out->m[10] = M.m[10] * Factor;
      Out->m[11] = M.m[11] * Factor;
      Out->m[12] = M.m[12] * Factor;
      Out->m[13] = M.m[13] * Factor;
      Out->m[14] = M.m[14] * Factor;
      Out->m[15] = M.m[15] * Factor;
   }

   inline void Matrix4Transform(const Matrix4& Matrix, const Vector3& Vector, Vector3* Out)
   {
      float fInvW = 1.0f / (Matrix.m[GE_M4_4_1] * Vector.X + Matrix.m[GE_M4_4_2] * Vector.Y + Matrix.m[GE_M4_4_3] * Vector.Z + Matrix.m[GE_M4_4_4]);

      Out->X = (Matrix.m[GE_M4_1_1] * Vector.X + Matrix.m[GE_M4_1_2] * Vector.Y + Matrix.m[GE_M4_1_3] * Vector.Z + Matrix.m[GE_M4_1_4]) * fInvW;
      Out->Y = (Matrix.m[GE_M4_2_1] * Vector.X + Matrix.m[GE_M4_2_2] * Vector.Y + Matrix.m[GE_M4_2_3] * Vector.Z + Matrix.m[GE_M4_2_4]) * fInvW;
      Out->Z = (Matrix.m[GE_M4_3_1] * Vector.X + Matrix.m[GE_M4_3_2] * Vector.Y + Matrix.m[GE_M4_3_3] * Vector.Z + Matrix.m[GE_M4_3_4]) * fInvW;
   }

   inline void Matrix4Transform(const Matrix4& Matrix, Vector3* VectorToTransform)
   {
      Vector3 vCopy = *VectorToTransform;
      float fInvW = 1.0f / (Matrix.m[GE_M4_4_1] * vCopy.X + Matrix.m[GE_M4_4_2] * vCopy.Y + Matrix.m[GE_M4_4_3] * vCopy.Z + Matrix.m[GE_M4_4_4]);

      VectorToTransform->X = (Matrix.m[GE_M4_1_1] * vCopy.X + Matrix.m[GE_M4_1_2] * vCopy.Y + Matrix.m[GE_M4_1_3] * vCopy.Z + Matrix.m[GE_M4_1_4]) * fInvW;
      VectorToTransform->Y = (Matrix.m[GE_M4_2_1] * vCopy.X + Matrix.m[GE_M4_2_2] * vCopy.Y + Matrix.m[GE_M4_2_3] * vCopy.Z + Matrix.m[GE_M4_2_4]) * fInvW;
      VectorToTransform->Z = (Matrix.m[GE_M4_3_1] * vCopy.X + Matrix.m[GE_M4_3_2] * vCopy.Y + Matrix.m[GE_M4_3_3] * vCopy.Z + Matrix.m[GE_M4_3_4]) * fInvW;
   }

   inline void Matrix4MakeIdentity(Matrix4* Out)
   {
      Out->m[GE_M4_1_1] = 1.0f;
      Out->m[GE_M4_2_1] = 0.0f;
      Out->m[GE_M4_3_1] = 0.0f;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = 0.0f;
      Out->m[GE_M4_2_2] = 1.0f;
      Out->m[GE_M4_3_2] = 0.0f;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = 0.0f;
      Out->m[GE_M4_2_3] = 0.0f;
      Out->m[GE_M4_3_3] = 1.0f;
      Out->m[GE_M4_4_3] = 0.0f;
   
      Out->m[GE_M4_1_4] = 0.0f;
      Out->m[GE_M4_2_4] = 0.0f;
      Out->m[GE_M4_3_4] = 0.0f;
      Out->m[GE_M4_4_4] = 1.0f;
   }

   inline void Matrix4MakeTranslation(const Vector3& Vector, Matrix4* Out)
   {
      Out->m[GE_M4_1_1] = 1.0f;
      Out->m[GE_M4_2_1] = 0.0f;
      Out->m[GE_M4_3_1] = 0.0f;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = 0.0f;
      Out->m[GE_M4_2_2] = 1.0f;
      Out->m[GE_M4_3_2] = 0.0f;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = 0.0f;
      Out->m[GE_M4_2_3] = 0.0f;
      Out->m[GE_M4_3_3] = 1.0f;
      Out->m[GE_M4_4_3] = 0.0f;
   
      Out->m[GE_M4_1_4] = Vector.X;
      Out->m[GE_M4_2_4] = Vector.Y;
      Out->m[GE_M4_3_4] = Vector.Z;
      Out->m[GE_M4_4_4] = 1.0f;
   }

   inline void Matrix4MakeXRotation(float AngleInRadians, Matrix4* Out)
   {
      float cos = cosf(AngleInRadians);
      float sin = sinf(AngleInRadians);
   
      Out->m[GE_M4_1_1] = 1.0f;
      Out->m[GE_M4_2_1] = 0.0f;
      Out->m[GE_M4_3_1] = 0.0f;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = 0.0f;
      Out->m[GE_M4_2_2] = cos;
      Out->m[GE_M4_3_2] = sin;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = 0.0f;
      Out->m[GE_M4_2_3] = -sin;
      Out->m[GE_M4_3_3] = cos;
      Out->m[GE_M4_4_3] = 0.0f;
   
      Out->m[GE_M4_1_4] = 0.0f;
      Out->m[GE_M4_2_4] = 0.0f;
      Out->m[GE_M4_3_4] = 0.0f;
      Out->m[GE_M4_4_4] = 1.0f;
   }

   inline void Matrix4MakeYRotation(float AngleInRadians, Matrix4* Out)
   {
      float cos = cosf(AngleInRadians);
      float sin = sinf(AngleInRadians);
   
      Out->m[GE_M4_1_1] = cos;
      Out->m[GE_M4_2_1] = 0.0f;
      Out->m[GE_M4_3_1] = -sin;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = 0.0f;
      Out->m[GE_M4_2_2] = 1.0f;
      Out->m[GE_M4_3_2] = 0.0f;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = sin;
      Out->m[GE_M4_2_3] = 0.0f;
      Out->m[GE_M4_3_3] = cos;
      Out->m[GE_M4_4_3] = 0.0f;
   
      Out->m[GE_M4_1_4] = 0.0f;
      Out->m[GE_M4_2_4] = 0.0f;
      Out->m[GE_M4_3_4] = 0.0f;
      Out->m[GE_M4_4_4] = 1.0f;
   }

   inline void Matrix4MakeZRotation(float AngleInRadians, Matrix4* Out)
   {
      float cos = cosf(AngleInRadians);
      float sin = sinf(AngleInRadians);
   
      Out->m[GE_M4_1_1] = cos;
      Out->m[GE_M4_2_1] = sin;
      Out->m[GE_M4_3_1] = 0.0f;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = -sin;
      Out->m[GE_M4_2_2] = cos;
      Out->m[GE_M4_3_2] = 0.0f;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = 0.0f;
      Out->m[GE_M4_2_3] = 0.0f;
      Out->m[GE_M4_3_3] = 1.0f;
      Out->m[GE_M4_4_3] = 0.0f;
   
      Out->m[GE_M4_1_4] = 0.0f;
      Out->m[GE_M4_2_4] = 0.0f;
      Out->m[GE_M4_3_4] = 0.0f;
      Out->m[GE_M4_4_4] = 1.0f;
   }

   inline void Matrix4MakeRotation(const Vector3& Axis, float AngleInRadians, Matrix4* Out)
   {
      Vector3 v = Vector3Normalize(Axis);
      float cos = cosf(AngleInRadians);
      float cosp = 1.0f - cos;
      float sin = sinf(AngleInRadians);
   
      Out->m[GE_M4_1_1] = cos + cosp * v.X * v.X;
      Out->m[GE_M4_2_1] = cosp * v.X * v.Y + v.Z * sin;
      Out->m[GE_M4_3_1] = cosp * v.X * v.Z - v.Y * sin;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = cosp * v.X * v.Y - v.Z * sin;
      Out->m[GE_M4_2_2] = cos + cosp * v.Y * v.Y;
      Out->m[GE_M4_3_2] = cosp * v.Y * v.Z + v.X * sin;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = cosp * v.X * v.Z + v.Y * sin;
      Out->m[GE_M4_2_3] = cosp * v.Y * v.Z - v.X * sin;
      Out->m[GE_M4_3_3] = cos + cosp * v.Z * v.Z;
      Out->m[GE_M4_4_3] = 0.0f;
   
      Out->m[GE_M4_1_4] = 0.0f;
      Out->m[GE_M4_2_4] = 0.0f;
      Out->m[GE_M4_3_4] = 0.0f;
      Out->m[GE_M4_4_4] = 1.0f;
   }

   inline void Matrix4MakeLookAt(const Vector3& Eye, const Vector3& Point, const Vector3& Up, Matrix4* Out)
   {
      Vector3 n = Vector3Normalize(Vector3Add(Eye, Vector3Negate(Point)));
      Vector3 u = Vector3Normalize(Vector3CrossProduct(Up, n));
      Vector3 v = Vector3CrossProduct(n, u);
   
      Out->m[GE_M4_1_1] = u.X;
      Out->m[GE_M4_2_1] = v.X;
      Out->m[GE_M4_3_1] = n.X;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = u.Y;
      Out->m[GE_M4_2_2] = v.Y;
      Out->m[GE_M4_3_2] = n.Y;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = u.Z;
      Out->m[GE_M4_2_3] = v.Z;
      Out->m[GE_M4_3_3] = n.Z;
      Out->m[GE_M4_4_3] = 0.0f;
   
      Out->m[GE_M4_1_4] = Vector3DotProduct(Vector3Negate(u), Eye);
      Out->m[GE_M4_2_4] = Vector3DotProduct(Vector3Negate(v), Eye);
      Out->m[GE_M4_3_4] = Vector3DotProduct(Vector3Negate(n), Eye);
      Out->m[GE_M4_4_4] = 1.0f;
   }

   inline void Matrix4MakeOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far, Matrix4* Out)
   {
      float ral = Right + Left;
      float rsl = Right - Left;
      float tab = Top + Bottom;
      float tsb = Top - Bottom;
      float fan = Far + Near;
      float fsn = Far - Near;
   
      Out->m[GE_M4_1_1] = 2.0f / rsl;
      Out->m[GE_M4_2_1] = 0.0f;
      Out->m[GE_M4_3_1] = 0.0f;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = 0.0f;
      Out->m[GE_M4_2_2] = 2.0f / tsb;
      Out->m[GE_M4_3_2] = 0.0f;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = 0.0f;
      Out->m[GE_M4_2_3] = 0.0f;
      Out->m[GE_M4_3_3] = -2.0f / fsn;
      Out->m[GE_M4_4_3] = 0.0f;
   
      Out->m[GE_M4_1_4] = -ral / rsl;
      Out->m[GE_M4_2_4] = -tab / tsb;
      Out->m[GE_M4_3_4] = -fan / fsn;
      Out->m[GE_M4_4_4] = 1.0f;
   }

   inline void Matrix4MakePerspective(float FOVyRadians, float Aspect, float NearZ, float FarZ, Matrix4* Out)
   {
      float cotan = 1.0f / tanf(FOVyRadians * 0.5f);
   
      Out->m[GE_M4_1_1] = cotan / Aspect;
      Out->m[GE_M4_2_1] = 0.0f;
      Out->m[GE_M4_3_1] = 0.0f;
      Out->m[GE_M4_4_1] = 0.0f;
   
      Out->m[GE_M4_1_2] = 0.0f;
      Out->m[GE_M4_2_2] = cotan;
      Out->m[GE_M4_3_2] = 0.0f;
      Out->m[GE_M4_4_2] = 0.0f;
   
      Out->m[GE_M4_1_3] = 0.0f;
      Out->m[GE_M4_2_3] = 0.0f;
      Out->m[GE_M4_3_3] = (FarZ + NearZ) / (NearZ - FarZ);
      Out->m[GE_M4_4_3] = -1.0f;
   
      Out->m[GE_M4_1_4] = 0.0f;
      Out->m[GE_M4_2_4] = 0.0f;
      Out->m[GE_M4_3_4] = (2.0f * FarZ * NearZ) / (NearZ - FarZ);
      Out->m[GE_M4_4_4] = 0.0f;
   }


   //
   //  Matrix transformation
   //
   inline void Matrix4Translate(Matrix4* Matrix, const Vector3& Translation)
   {
      Matrix->m[GE_M4_1_4] += Translation.X;
      Matrix->m[GE_M4_2_4] += Translation.Y;
      Matrix->m[GE_M4_3_4] += Translation.Z;
   }

   inline void Matrix4Scale(Matrix4* Matrix, const Vector3& Scale)
   {
      Matrix->m[GE_M4_1_1] *= Scale.X;
      Matrix->m[GE_M4_2_1] *= Scale.X;
      Matrix->m[GE_M4_3_1] *= Scale.X;
      Matrix->m[GE_M4_4_1] *= Scale.X;
   
      Matrix->m[GE_M4_1_2] *= Scale.Y;
      Matrix->m[GE_M4_2_2] *= Scale.Y;
      Matrix->m[GE_M4_3_2] *= Scale.Y;
      Matrix->m[GE_M4_4_2] *= Scale.Y;
   
      Matrix->m[GE_M4_1_3] *= Scale.Z;
      Matrix->m[GE_M4_2_3] *= Scale.Z;
      Matrix->m[GE_M4_3_3] *= Scale.Z;
      Matrix->m[GE_M4_4_3] *= Scale.Z;
   }

   inline void Matrix4RotateX(Matrix4* Matrix, float AngleInRadians)
   {
      Matrix4 matRotation;
      Matrix4MakeXRotation(AngleInRadians, &matRotation);
      Matrix4 matCopy(*Matrix);
      Matrix4Multiply(matCopy, matRotation, Matrix);
   }

   inline void Matrix4RotateY(Matrix4* Matrix, float AngleInRadians)
   {
      Matrix4 matRotation;
      Matrix4MakeYRotation(AngleInRadians, &matRotation);
      Matrix4 matCopy(*Matrix);
      Matrix4Multiply(matCopy, matRotation, Matrix);
   }

   inline void Matrix4RotateZ(Matrix4* Matrix, float AngleInRadians)
   {
      Matrix4 matRotation;
      Matrix4MakeZRotation(AngleInRadians, &matRotation);
      Matrix4 matCopy(*Matrix);
      Matrix4Multiply(matCopy, matRotation, Matrix);
   }

   inline void Matrix4Transpose(Matrix4* Matrix)
   {
      Matrix4 matCopy(*Matrix);
   
      Matrix->m[GE_M4_2_1] = matCopy.m[GE_M4_1_2];
      Matrix->m[GE_M4_3_1] = matCopy.m[GE_M4_1_3];
      Matrix->m[GE_M4_4_1] = matCopy.m[GE_M4_1_4];
   
      Matrix->m[GE_M4_1_2] = matCopy.m[GE_M4_2_1];
      Matrix->m[GE_M4_3_2] = matCopy.m[GE_M4_2_3];
      Matrix->m[GE_M4_4_2] = matCopy.m[GE_M4_2_4];

      Matrix->m[GE_M4_1_3] = matCopy.m[GE_M4_3_1];
      Matrix->m[GE_M4_2_3] = matCopy.m[GE_M4_3_2];
      Matrix->m[GE_M4_4_3] = matCopy.m[GE_M4_3_4];

      Matrix->m[GE_M4_1_4] = matCopy.m[GE_M4_4_1];
      Matrix->m[GE_M4_2_4] = matCopy.m[GE_M4_4_2];
      Matrix->m[GE_M4_3_4] = matCopy.m[GE_M4_4_3];
   }

   inline void Matrix4Invert(Matrix4* Matrix)
   {
      float m00 = Matrix->m[GE_M4_1_1], m01 = Matrix->m[GE_M4_1_2], m02 = Matrix->m[GE_M4_1_3], m03 = Matrix->m[GE_M4_1_4];
      float m10 = Matrix->m[GE_M4_2_1], m11 = Matrix->m[GE_M4_2_2], m12 = Matrix->m[GE_M4_2_3], m13 = Matrix->m[GE_M4_2_4];
      float m20 = Matrix->m[GE_M4_3_1], m21 = Matrix->m[GE_M4_3_2], m22 = Matrix->m[GE_M4_3_3], m23 = Matrix->m[GE_M4_3_4];
      float m30 = Matrix->m[GE_M4_4_1], m31 = Matrix->m[GE_M4_4_2], m32 = Matrix->m[GE_M4_4_3], m33 = Matrix->m[GE_M4_4_4];

      float v0 = m20 * m31 - m21 * m30;
      float v1 = m20 * m32 - m22 * m30;
      float v2 = m20 * m33 - m23 * m30;
      float v3 = m21 * m32 - m22 * m31;
      float v4 = m21 * m33 - m23 * m31;
      float v5 = m22 * m33 - m23 * m32;

      float t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
      float t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
      float t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
      float t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

      float fInvDet = 1.0f / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

      Matrix->m[GE_M4_1_1] = t00 * fInvDet;
      Matrix->m[GE_M4_2_1] = t10 * fInvDet;
      Matrix->m[GE_M4_3_1] = t20 * fInvDet;
      Matrix->m[GE_M4_4_1] = t30 * fInvDet;

      Matrix->m[GE_M4_1_2] = -(v5 * m01 - v4 * m02 + v3 * m03) * fInvDet;
      Matrix->m[GE_M4_2_2] = +(v5 * m00 - v2 * m02 + v1 * m03) * fInvDet;
      Matrix->m[GE_M4_3_2] = -(v4 * m00 - v2 * m01 + v0 * m03) * fInvDet;
      Matrix->m[GE_M4_4_2] = +(v3 * m00 - v1 * m01 + v0 * m02) * fInvDet;

      v0 = m10 * m31 - m11 * m30;
      v1 = m10 * m32 - m12 * m30;
      v2 = m10 * m33 - m13 * m30;
      v3 = m11 * m32 - m12 * m31;
      v4 = m11 * m33 - m13 * m31;
      v5 = m12 * m33 - m13 * m32;

      Matrix->m[GE_M4_1_3] = + (v5 * m01 - v4 * m02 + v3 * m03) * fInvDet;
      Matrix->m[GE_M4_2_3] = - (v5 * m00 - v2 * m02 + v1 * m03) * fInvDet;
      Matrix->m[GE_M4_3_3] = + (v4 * m00 - v2 * m01 + v0 * m03) * fInvDet;
      Matrix->m[GE_M4_4_3] = - (v3 * m00 - v1 * m01 + v0 * m02) * fInvDet;

      v0 = m21 * m10 - m20 * m11;
      v1 = m22 * m10 - m20 * m12;
      v2 = m23 * m10 - m20 * m13;
      v3 = m22 * m11 - m21 * m12;
      v4 = m23 * m11 - m21 * m13;
      v5 = m23 * m12 - m22 * m13;

      Matrix->m[GE_M4_1_4] = -(v5 * m01 - v4 * m02 + v3 * m03) * fInvDet;
      Matrix->m[GE_M4_2_4] = +(v5 * m00 - v2 * m02 + v1 * m03) * fInvDet;
      Matrix->m[GE_M4_3_4] = -(v4 * m00 - v2 * m01 + v0 * m03) * fInvDet;
      Matrix->m[GE_M4_4_4] = +(v3 * m00 - v1 * m01 + v0 * m02) * fInvDet;
   }

   inline void Matrix3Transpose(Matrix3* Matrix)
   {
      Matrix3 matCopy(*Matrix);
   
      Matrix->m[GE_M3_2_1] = matCopy.m[GE_M3_1_2];
      Matrix->m[GE_M3_3_1] = matCopy.m[GE_M3_1_3];
   
      Matrix->m[GE_M3_1_2] = matCopy.m[GE_M3_2_1];
      Matrix->m[GE_M3_3_2] = matCopy.m[GE_M3_2_3];
   
      Matrix->m[GE_M3_1_3] = matCopy.m[GE_M3_3_1];
      Matrix->m[GE_M3_2_3] = matCopy.m[GE_M3_3_2];
   }

   inline void Matrix3Invert(Matrix3* Matrix)
   {
      Matrix3 matCopy(*Matrix);
   
      Matrix->m[GE_M3_1_1] = matCopy.m[GE_M3_2_2] * matCopy.m[GE_M3_3_3] - matCopy.m[GE_M3_3_2] * matCopy.m[GE_M3_2_3];
      Matrix->m[GE_M3_2_1] = matCopy.m[GE_M3_3_1] * matCopy.m[GE_M3_2_3] - matCopy.m[GE_M3_2_1] * matCopy.m[GE_M3_3_3];
      Matrix->m[GE_M3_3_1] = matCopy.m[GE_M3_2_1] * matCopy.m[GE_M3_3_2] - matCopy.m[GE_M3_3_1] * matCopy.m[GE_M3_2_2];
   
      Matrix->m[GE_M3_1_2] = matCopy.m[GE_M3_3_2] * matCopy.m[GE_M3_1_3] - matCopy.m[GE_M3_1_2] * matCopy.m[GE_M3_3_3];
      Matrix->m[GE_M3_2_2] = matCopy.m[GE_M3_1_1] * matCopy.m[GE_M3_3_3] - matCopy.m[GE_M3_3_1] * matCopy.m[GE_M3_1_3];
      Matrix->m[GE_M3_3_2] = matCopy.m[GE_M3_3_1] * matCopy.m[GE_M3_1_2] - matCopy.m[GE_M3_1_1] * matCopy.m[GE_M3_3_2];
   
      Matrix->m[GE_M3_1_3] = matCopy.m[GE_M3_1_2] * matCopy.m[GE_M3_2_3] - matCopy.m[GE_M3_2_2] * matCopy.m[GE_M3_1_3];
      Matrix->m[GE_M3_2_3] = matCopy.m[GE_M3_2_1] * matCopy.m[GE_M3_1_3] - matCopy.m[GE_M3_1_1] * matCopy.m[GE_M3_2_3];
      Matrix->m[GE_M3_3_3] = matCopy.m[GE_M3_1_1] * matCopy.m[GE_M3_2_2] - matCopy.m[GE_M3_2_1] * matCopy.m[GE_M3_1_2];
   
      float fDet =
         matCopy.m[GE_M3_1_1] * Matrix->m[GE_M3_1_1] +
         matCopy.m[GE_M3_2_1] * Matrix->m[GE_M3_1_2] +
         matCopy.m[GE_M3_3_1] * Matrix->m[GE_M3_1_3];
   
      if(fabs(fDet) <= 0.01f)
         return;
   
      float fInvDet = 1.0f / fDet;
   
      Matrix->m[GE_M3_1_1] *= fInvDet;
      Matrix->m[GE_M3_2_1] *= fInvDet;
      Matrix->m[GE_M3_3_1] *= fInvDet;
      Matrix->m[GE_M3_1_2] *= fInvDet;
      Matrix->m[GE_M3_2_2] *= fInvDet;
      Matrix->m[GE_M3_3_2] *= fInvDet;
      Matrix->m[GE_M3_1_3] *= fInvDet;
      Matrix->m[GE_M3_2_3] *= fInvDet;
      Matrix->m[GE_M3_3_3] *= fInvDet;
   }
}
