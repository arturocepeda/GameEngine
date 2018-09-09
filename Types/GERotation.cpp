
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GERotation.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GERotation.h"
#include "Core/GEConstants.h"
#include <cmath>

using namespace GE;
using namespace GE::Core;

Rotation::Rotation()
{
   reset();
}

Rotation::Rotation(const Vector3& EulerAnglesInRadians)
{
   setFromEulerAngles(EulerAnglesInRadians);
}

Rotation::Rotation(const Vector3& Axis, float AngleInRadians)
{
   setFromAxisAngle(Axis, AngleInRadians);
}

Rotation::Rotation(const Quaternion& Q)
{
   setFromQuaternion(Q);
}

Rotation::Rotation(const Matrix4& RotationMatrix)
{
   mRotationMatrix = RotationMatrix;
   calculateQuaternionFromMatrix();
}

void Rotation::reset()
{
   qQuaternion.X = 0.0f;
   qQuaternion.Y = 0.0f;
   qQuaternion.Z = 0.0f;
   qQuaternion.W = 1.0f;

   Matrix4MakeIdentity(&mRotationMatrix);
}

void Rotation::setFromEulerAngles(const Vector3& EulerAnglesInRadians)
{
   Rotation r =
      Rotation(Vector3::UnitZ, EulerAnglesInRadians.Z) *
      Rotation(Vector3::UnitY, EulerAnglesInRadians.Y) *
      Rotation(Vector3::UnitX, EulerAnglesInRadians.X);
   qQuaternion = r.getQuaternion();
   mRotationMatrix = r.getRotationMatrix();
}

void Rotation::setFromAxisAngle(const Vector3& Axis, float AngleInRadians)
{
   float fHalfAngle = AngleInRadians * 0.5f;
   float fSin = sinf(fHalfAngle);
   float fCos = cosf(fHalfAngle);

   qQuaternion.X = fSin * Axis.X;
   qQuaternion.Y = fSin * Axis.Y;
   qQuaternion.Z = fSin * Axis.Z;
   qQuaternion.W = fCos;
   qQuaternion.normalize();

   calculateMatrixFromQuaternion();
}

void Rotation::setFromQuaternion(const Quaternion& Q)
{
   qQuaternion = Q;
   qQuaternion.normalize();
   calculateMatrixFromQuaternion();
}

void Rotation::calculateMatrixFromQuaternion()
{
   float qx = qQuaternion.X;
   float qy = qQuaternion.Y;
   float qz = qQuaternion.Z;
   float qw = qQuaternion.W;

   float qx2 = qx * qx;
   float qy2 = qy * qy;
   float qz2 = qz * qz;

   mRotationMatrix.m[GE_M4_1_1] = 1.0f - (2.0f * qy2) - (2.0f * qz2);
   mRotationMatrix.m[GE_M4_2_1] = (2.0f * qx * qy) - (2.0f * qz * qw);
   mRotationMatrix.m[GE_M4_3_1] = (2.0f * qx * qz) + (2.0f * qy * qw);
   mRotationMatrix.m[GE_M4_4_1] = 0.0f;

   mRotationMatrix.m[GE_M4_1_2] = (2.0f * qx * qy) + (2.0f * qz * qw);
   mRotationMatrix.m[GE_M4_2_2] = 1.0f - (2.0f * qx2) - (2.0f * qz2);
   mRotationMatrix.m[GE_M4_3_2] = (2.0f * qy * qz) - (2.0f * qx * qw);
   mRotationMatrix.m[GE_M4_4_2] = 0.0f;

   mRotationMatrix.m[GE_M4_1_3] = (2.0f * qx * qz) - (2.0f * qy * qw);
   mRotationMatrix.m[GE_M4_2_3] = (2.0f * qy * qz) + (2.0f * qx * qw);
   mRotationMatrix.m[GE_M4_3_3] = 1.0f - (2.0f * qx2) - (2.0f * qy2);
   mRotationMatrix.m[GE_M4_4_3] = 0.0f;

   mRotationMatrix.m[GE_M4_1_4] = 0.0f;
   mRotationMatrix.m[GE_M4_2_4] = 0.0f;
   mRotationMatrix.m[GE_M4_3_4] = 0.0f;
   mRotationMatrix.m[GE_M4_4_4] = 1.0f;
}

void Rotation::calculateQuaternionFromMatrix()
{
   qQuaternion.W = sqrt(1.0f + mRotationMatrix.m[GE_M4_1_1] + mRotationMatrix.m[GE_M4_2_2] + mRotationMatrix.m[GE_M4_3_3]) * 0.5f;

   const float f4QW = -4.0f * qQuaternion.W;
   const float Epsilon = 0.001f;

   if(fabsf(f4QW) > Epsilon)
   {
      qQuaternion.X = (mRotationMatrix.m[GE_M4_3_2] - mRotationMatrix.m[GE_M4_2_3]) / f4QW;
      qQuaternion.Y = (mRotationMatrix.m[GE_M4_1_3] - mRotationMatrix.m[GE_M4_3_1]) / f4QW;
      qQuaternion.Z = (mRotationMatrix.m[GE_M4_2_1] - mRotationMatrix.m[GE_M4_1_2]) / f4QW;
      qQuaternion.normalize();
   }
   else
   {
      // main diagonal: (1, 1, 1)
      if(fabsf(mRotationMatrix.m[GE_M4_1_1] - 1.0f) < Epsilon &&
         fabsf(mRotationMatrix.m[GE_M4_2_2] - 1.0f) < Epsilon &&
         fabsf(mRotationMatrix.m[GE_M4_3_3] - 1.0f) < Epsilon)
      {
         qQuaternion.X = 0.0f;
         qQuaternion.Y = 0.0f;
         qQuaternion.Z = 0.0f;
         qQuaternion.W = 1.0f;
      }
      // main diagonal: (1, -1, -1)
      else if(fabsf(mRotationMatrix.m[GE_M4_1_1] - 1.0f) < Epsilon &&
         fabsf(mRotationMatrix.m[GE_M4_2_2] - -1.0f) < Epsilon &&
         fabsf(mRotationMatrix.m[GE_M4_3_3] - -1.0f) < Epsilon)
      {
         qQuaternion.X = 1.0f;
         qQuaternion.Y = 0.0f;
         qQuaternion.Z = 0.0f;
         qQuaternion.W = 0.0f;
      }
      // main diagonal: (-1, 1, -1)
      else if(fabsf(mRotationMatrix.m[GE_M4_1_1] - -1.0f) < Epsilon &&
         fabsf(mRotationMatrix.m[GE_M4_2_2] - 1.0f) < Epsilon &&
         fabsf(mRotationMatrix.m[GE_M4_3_3] - -1.0f) < Epsilon)
      {
         qQuaternion.X = 0.0f;
         qQuaternion.Y = 1.0f;
         qQuaternion.Z = 0.0f;
         qQuaternion.W = 0.0f;
      }
      // main diagonal: (-1, -1, 1)
      else
      {
         qQuaternion.X = 0.0f;
         qQuaternion.Y = 0.0f;
         qQuaternion.Z = 1.0f;
         qQuaternion.W = 0.0f;
      }
   }
}

const Quaternion& Rotation::getQuaternion() const
{
   return qQuaternion;
}

const Matrix4& Rotation::getRotationMatrix() const
{
   return mRotationMatrix;
}

Vector3 Rotation::getEulerAngles() const
{
   Vector3 vEuler;

   vEuler.X =
      atan2f(-2.0f * (qQuaternion.Y*qQuaternion.Z - qQuaternion.W*qQuaternion.X),
      qQuaternion.W*qQuaternion.W - qQuaternion.X*qQuaternion.X -qQuaternion.Y*qQuaternion.Y + qQuaternion.Z*qQuaternion.Z);
   vEuler.Y =
      asinf(Math::clamp(2.0f * (qQuaternion.X*qQuaternion.Z + qQuaternion.W*qQuaternion.Y), -1.0f, 1.0f));
   vEuler.Z =
      atan2f(-2.0f * (qQuaternion.X*qQuaternion.Y - qQuaternion.W*qQuaternion.Z),
      qQuaternion.W*qQuaternion.W + qQuaternion.X*qQuaternion.X - qQuaternion.Y*qQuaternion.Y - qQuaternion.Z*qQuaternion.Z);

   return vEuler;
}

void Rotation::getAxisAngle(Vector3* OutAxis, float* OutAngle) const
{
   float fSquareRoot = sqrt(1.0f - (qQuaternion.W * qQuaternion.W));

   OutAxis->X = qQuaternion.X / fSquareRoot;
   OutAxis->Y = qQuaternion.Y / fSquareRoot;
   OutAxis->Z = qQuaternion.Z / fSquareRoot;
   *OutAngle = 2.0f * acos(qQuaternion.W);
}

Rotation Rotation::operator*(const Rotation& Other) const
{
   return Rotation(Other.qQuaternion * qQuaternion);
}

Rotation& Rotation::operator*=(const Rotation& Other)
{
   qQuaternion *= Other.qQuaternion;
   calculateMatrixFromQuaternion();
   return *this;
}

Vector3 Rotation::operator*(const Vector3& V) const
{
   Vector3 vResult;
   Matrix4Transform(mRotationMatrix, V, &vResult);
   return vResult;
}
