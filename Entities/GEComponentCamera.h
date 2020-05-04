
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentCamera.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentTransform.h"
#include "Core/GEPhysics.h"

namespace GE { namespace Entities
{
   class ComponentCamera : public Component
   {
   private:
      ComponentTransform* cTransform;

      float fFOV;
      float fNearZ;
      float fFarZ;

      Matrix4 matView;
      Matrix4 matProjection;
      Matrix4 matViewProjection;

      void calculateProjectionMatrix();
      void calculateViewMatrix();

   public:
      static ComponentType getType() { return ComponentType::Camera; }

      static const float DefaultFOV;
      static const float DefaultNearZ;
      static const float DefaultFarZ;

      ComponentCamera(Entity* Owner);
      ~ComponentCamera();

      void update();

      ComponentTransform* getTransform() const;

      float getFOV() const;
      float getNearZ() const;
      float getFarZ() const;
      
      const Matrix4& getViewMatrix() const;
      const Matrix4& getProjectionMatrix() const;
      const Matrix4& getViewProjectionMatrix() const;

      void setFOV(float FOV);
      void setNearZ(float NearZ);
      void setFarZ(float FarZ);

      void lookAt(float X, float Y, float Z);
      void lookAt(const Vector3& LookAt);
      void orbit(const Vector3& ReferencePoint, float Distance, float Theta, float Phi);

      void worldToScreen(const Vector3& PositionWorld, Vector2* OutPositionScreen) const;
      void screenToWorld(const Vector2& PositionScreen, Vector3* OutWorldPointNear, Vector3* OutWorldPointFar) const;

      Core::Physics::Ray getScreenRay(const Vector2& PositionScreen) const;
   };
}}
