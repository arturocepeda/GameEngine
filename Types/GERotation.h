
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GERotation.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEQuaternion.h"
#include "GEMatrix.h"

namespace GE
{
   class Rotation
   {
   private:
      Quaternion qQuaternion;
      Matrix4 mRotationMatrix;

      void calculateMatrixFromQuaternion();
      void calculateQuaternionFromMatrix();

   public:
      Rotation();
      Rotation(const Vector3& EulerAnglesInRadians);
      Rotation(const Vector3& Axis, float AngleInRadians);
      Rotation(const Quaternion& Q);
      Rotation(const Matrix4& RotationMatrix);

      void reset();
      void setFromEulerAngles(const Vector3& EulerAnglesInRadians);
      void setFromAxisAngle(const Vector3& Axis, float AngleInRadians);
      void setFromQuaternion(const Quaternion& Q);

      const Quaternion& getQuaternion() const;
      const Matrix4& getRotationMatrix() const;

      Vector3 getEulerAngles() const;
      void getAxisAngle(Vector3* OutAxis, float* OutAngle) const;

      Rotation operator*(const Rotation& Other) const;
      Rotation& operator*=(const Rotation& Other);

      Vector3 operator*(const Vector3& V) const;
   };
}
