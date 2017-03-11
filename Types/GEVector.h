
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GEVector.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEMath.h"
#include <cmath>

namespace GE
{
   struct Vector2
   {
      float X;
      float Y;

      static Vector2 Zero;
      static Vector2 One;
      static Vector2 UnitX;
      static Vector2 UnitY;

      Vector2()
      : X(0.0f)
      , Y(0.0f)
      {
      }

      Vector2(float vX, float vY)
      {
         set(vX, vY);
      }

      void set(float vX, float vY)
      {
         X = vX;
         Y = vY;
      }

      float getLength()
      {
         return sqrt(X * X + Y * Y);
      }

      void normalize()
      {
         float fLength = sqrt(X * X + Y * Y);

         X /= fLength;
         Y /= fLength;
      }

      Vector2 operator+(const Vector2& v)
      {
         return Vector2(X + v.X, Y + v.Y);
      }

      Vector2& operator+=(const Vector2& v)
      {
         X += v.X; Y += v.Y;
         return *this;
      }

      Vector2 operator-(const Vector2& v)
      {
         return Vector2(X - v.X, Y - v.Y);
      }

      Vector2& operator-=(const Vector2& v)
      {
         X -= v.X; Y -= v.Y;
         return *this;
      }

      Vector2 operator*(const float fValue) const
      {
         return Vector2(X * fValue, Y * fValue);
      }

      Vector2& operator*=(const float fValue)
      {
         X *= fValue; Y *= fValue;
         return *this;
      }

      float operator*(const Vector2& v)
      {
         return (X * v.X + Y * v.Y);
      }

      Vector2& operator*=(const Vector2& v)
      {
         X *= v.X; Y *= v.Y;
         return *this;
      }

      Vector2 operator-()
      {
         return Vector2(-X, -Y);
      }

      static Vector2 lerp(const Vector2& V1, const Vector2& V2, float T)
      {
         return Vector2(Core::Math::lerp(V1.X, V2.X, T), Core::Math::lerp(V1.Y, V2.Y, T));
      }
   };

   struct Vector3
   {
      float X;
      float Y;
      float Z;

      static Vector3 Zero;
      static Vector3 One;
      static Vector3 UnitX;
      static Vector3 UnitY;
      static Vector3 UnitZ;

      Vector3()
         : X(0.0f)
         , Y(0.0f)
         , Z(0.0f)
      {
      }

      Vector3(float vX, float vY, float vZ)
         : X(vX)
         , Y(vY)
         , Z(vZ)
      {
      }

      Vector3(float v)
         : X(v)
         , Y(v)
         , Z(v)
      {
      }

      void set(float vX, float vY, float vZ)
      {
         X = vX;
         Y = vY;
         Z = vZ;
      }

      float getSquaredLength() const
      {
         return X * X + Y * Y + Z * Z;
      }

      float getLength() const
      {
         return sqrt(X * X + Y * Y + Z * Z);
      }

      float normalize()
      {
         float fLength = sqrt(X * X + Y * Y + Z * Z);

         X /= fLength;
         Y /= fLength;
         Z /= fLength;

         return fLength;
      }

      float dotProduct(const Vector3& other) const
      {
         return X * other.X + Y * other.Y + Z * other.Z;
      }

      Vector3 crossProduct(const Vector3& other) const
      {
         return Vector3(Y * other.Z - Z * other.Y, Z * other.X - X * other.Z, X * other.Y - Y * other.X);
      }

      float getSquaredDistanceTo(const Vector3& other) const
      {
         float fDeltaX = other.X - X;
         float fDeltaY = other.Y - Y;
         float fDeltaZ = other.Z - Z;

         return fDeltaX * fDeltaX + fDeltaY * fDeltaY + fDeltaZ * fDeltaZ;
      }

      float getDistanceTo(const Vector3& other) const
      {
         float fDeltaX = other.X - X;
         float fDeltaY = other.Y - Y;
         float fDeltaZ = other.Z - Z;

         return sqrt(fDeltaX * fDeltaX + fDeltaY * fDeltaY + fDeltaZ * fDeltaZ);
      }

      void rotateYaw(float angle)
      {
         X = X * cosf(angle) - Z * sinf(angle);
         Z = X * sinf(angle) + Z * cosf(angle);
      }

      Vector3 operator+(const Vector3& v) const
      {
         return Vector3(X + v.X, Y + v.Y, Z + v.Z);
      }

      Vector3& operator+=(const Vector3& v)
      {
         X += v.X; Y += v.Y; Z += v.Z;
         return *this;
      }

      Vector3 operator-(const Vector3& v) const
      {
         return Vector3(X - v.X, Y - v.Y, Z - v.Z);
      }

      Vector3& operator-=(const Vector3& v)
      {
         X -= v.X; Y -= v.Y; Z -= v.Z;
         return *this;
      }

      Vector3 operator*(const float fValue) const
      {
         return Vector3(X * fValue, Y * fValue, Z * fValue);
      }

      Vector3& operator*=(const float fValue)
      {
         X *= fValue; Y *= fValue; Z *= fValue;
         return *this;
      }

      float operator*(const Vector3& v) const
      {
         return (X * v.X + Y * v.Y + Z * v.Z);
      }

      Vector3& operator*=(const Vector3& v)
      {
         X *= v.X; Y *= v.Y; Z *= v.Z;
         return *this;
      }

      Vector3 operator-()
      {
         return Vector3(-X, -Y, -Z);
      }

      static Vector3 lerp(const Vector3& V1, const Vector3& V2, float T)
      {
         return Vector3(Core::Math::lerp(V1.X, V2.X, T), Core::Math::lerp(V1.Y, V2.Y, T), Core::Math::lerp(V1.Z, V2.Z, T));
      }
   };

   inline Vector3 Vector3Negate(const Vector3& V)
   {
      return Vector3(-V.X, -V.Y, -V.Z);
   }

   inline Vector3 Vector3Add(const Vector3& V1, const Vector3& V2)
   {
      return Vector3(V1.X + V2.X, V1.Y + V2.Y, V1.Z + V2.Z);
   }

   inline float Vector3Length(const Vector3& V)
   {
      return sqrt(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
   }

   inline Vector3 Vector3Normalize(const Vector3& V)
   {
      Vector3 vNormalized(V);
      vNormalized.normalize();
      return vNormalized;
   }

   inline float Vector3DotProduct(const Vector3& V1, const Vector3& V2)
   {
      return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;
   }

   inline Vector3 Vector3CrossProduct(const Vector3& V1, const Vector3& V2)
   {
      return Vector3(V1.Y * V2.Z - V1.Z * V2.Y, V1.Z * V2.X - V1.X * V2.Z, V1.X * V2.Y - V1.Y * V2.X);
   }
}
