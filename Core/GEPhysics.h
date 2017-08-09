
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEPhysics.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Entities/GEScene.h"

namespace GE { namespace Entities
{
   class ComponentCollider;
}}

namespace GE { namespace Core
{
   class Physics
   {
   public:
      struct Ray
      {
         Vector3 Origin;
         Vector3 Direction;

         Ray(const Vector3& vOrigin, const Vector3& vDirection)
            : Origin(vOrigin)
            , Direction(vDirection)
         {
         }
      };

      struct Plane
      {
         Vector3 Normal;
         float Distance;

         Plane(const Vector3& vNormal, float fDistance)
            : Normal(vNormal)
            , Distance(fDistance)
         {
         }

         Plane(const Vector3& vNormal, const Vector3& vPoint)
            : Normal(vNormal)
            , Distance(-vPoint.dotProduct(vNormal))
         {
         }
      };

      struct QuerySettings
      {
         uint CollisionGroupsBitMask;

         QuerySettings()
            : CollisionGroupsBitMask(0xffffffff)
         {
         }
      };

      struct HitInfo
      {
         Entities::Entity* Collider;
         Vector3 Position;
         Vector3 Normal;
         float Distance;

         HitInfo()
            : Collider(0)
            , Distance(0.0f)
         {
         }
      };

      static QuerySettings DefaultQuerySettings;

      static bool checkCollision(Entities::Scene* S, const Ray& R, 
         const QuerySettings& Settings = DefaultQuerySettings, HitInfo* OutHitInfo = 0);
      static bool checkCollision(Entities::Scene* S, Entities::ComponentCollider* Collider,
         const QuerySettings& Settings = DefaultQuerySettings, HitInfo* OutHitInfo = 0);

      static bool checkCollision(const Ray& R, const Plane& P, HitInfo* OutHitInfo = 0);
   };
}}
