
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentCollider.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentTransform.h"
#include "Core/GEPhysics.h"
#include "Content/GEMesh.h"

namespace GE { namespace Entities
{
   enum class ColliderType
   {
      Sphere,
      Capsule,
      Mesh
   };


   class ComponentCollider : public Component
   {
   protected:
      ColliderType eColliderType;
      uint iCollisionGroup;

      ComponentCollider(Entity* Owner, ColliderType Type);

   public:
      static ComponentType getType() { return ComponentType::Collider; }

      virtual ~ComponentCollider();

      ColliderType getColliderType() const;

      uint getCollisionGroup() const;
      void setCollisionGroup(uint Group);

      virtual bool checkCollision(const Core::Physics::Ray& R, Core::Physics::HitInfo* OutHitInfo) const = 0;

      GEProperty(UInt, CollisionGroup)
   };


   class ComponentColliderSphere : public ComponentCollider
   {
   private:
      float fRadius;

   public:
      ComponentColliderSphere(Entity* Owner);
      virtual ~ComponentColliderSphere();

      float getRadius() const;
      void setRadius(float Radius);

      float getScaledRadius() const;

      virtual bool checkCollision(const Core::Physics::Ray& R, Core::Physics::HitInfo* OutHitInfo) const override;

      GEProperty(Float, Radius)
   };


   class ComponentColliderCapsule : public ComponentCollider
   {
   private:
      float fRadius;
      float fHeight;

   public:
      ComponentColliderCapsule(Entity* Owner);
      virtual ~ComponentColliderCapsule();

      float getRadius() const;
      void setRadius(float Radius);

      float getHeight() const;
      void setHeight(float Height);

      virtual bool checkCollision(const Core::Physics::Ray& R, Core::Physics::HitInfo* OutHitInfo) const override;

      GEProperty(Float, Radius)
      GEProperty(Float, Height)
   };


   class ComponentColliderMesh : public ComponentCollider
   {
   private:
      Content::Mesh* cMesh;

   public:
      ComponentColliderMesh(Entity* Owner);
      virtual ~ComponentColliderMesh();

      const Core::ObjectName& getMeshName() const;
      void setMeshName(const Core::ObjectName& MeshName);

      virtual bool checkCollision(const Core::Physics::Ray& R, Core::Physics::HitInfo* OutHitInfo) const override;

      GEProperty(ObjectName, MeshName)
   };
}}
