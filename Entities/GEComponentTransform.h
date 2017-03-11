
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentTransform.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponent.h"
#include "GEComponentType.h"
#include "Core/GESerializable.h"

namespace GE { namespace Entities
{
   class ComponentTransform : public Component
   {
   protected:
      friend class Scene;

      Vector3 vPosition;
      Rotation cRotation;
      Vector3 vEulerAngles;
      Vector3 vScale;

      Matrix4 mLocalWorldMatrix;
      Matrix4 mGlobalWorldMatrix;

      Vector3 vForward;
      Vector3 vUp;
      Vector3 vRight;

      void updateDirectionVectors();

   public:
      static ComponentType getType() { return ComponentType::Transform; }

      ComponentTransform(Entity* Owner);
      virtual ~ComponentTransform();

      void move(float DX, float DY, float DZ = 0.0f);
      void move(const Vector3& Move);
      void rotate(const Rotation& Rotate);
      void scale(float SX, float SY, float SZ = 1.0f);
      void scale(float ScaleFactor);
      void scale(const Vector3& Scale);

      void reset();
      void updateWorldMatrix();

      Vector3& getPosition();
      Rotation& getRotation();
      Vector3& getOrientation();
      Vector3& getScale();

      Vector3 getWorldPosition() const;
      Rotation getWorldRotation() const;
      Vector3 getWorldScale() const;

      const Matrix4& getLocalWorldMatrix() const;
      const Matrix4& getGlobalWorldMatrix() const;

      const Vector3& getForwardVector() const;
      const Vector3& getUpVector() const;
      const Vector3& getRightVector() const;

      void setPosition(float X, float Y, float Z = 0.0f);
      void setPosition(const Vector3& Position);
      void setRotation(const Rotation& R);
      void setOrientation(const Vector3& EulerAnglesInDegrees);
      void setScale(float X, float Y, float Z = 1.0f);
      void setScale(float ScaleFactor);
      void setScale(const Vector3& Scale);
      void setUniformScale(float ScaleFactor);

      void setLocalWorldMatrix(const Matrix4& LocalWorldMatrix);

      void setForwardVector(const Vector3& Forward);

      GEProperty(Vector3, Position)
      GEProperty(Vector3, Orientation)
      GEProperty(Vector3, Scale)
   };
}}
