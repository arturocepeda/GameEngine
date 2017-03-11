
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEPhysics.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "GEPhysics.h"
#include "Entities/GEEntity.h"
#include "Entities/GEComponentCollider.h"

#include <cfloat>

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;

Physics::QuerySettings Physics::DefaultQuerySettings;

bool Physics::checkCollision(Scene* S, const Ray& R, const QuerySettings& Settings, HitInfo* OutHitInfo)
{
   GEAssert(S);
   const GESTLVector(Component*)& vColliders = S->getComponents<ComponentCollider>();

   bool bAnyHit = false;
   HitInfo sClosestHitInfo;
   sClosestHitInfo.Distance = FLT_MAX;

   for(uint i = 0; i < vColliders.size(); i++)
   {
      if(!vColliders[i]->getOwner()->isActiveInHierarchy())
         continue;

      ComponentCollider* cCollider = static_cast<ComponentCollider*>(vColliders[i]);
      uint iCollisionGroupBitMask = 1 << cCollider->getCollisionGroup();

      if((Settings.CollisionGroupsBitMask & iCollisionGroupBitMask) == 0)
         continue;

      HitInfo sHitInfo;

      if(cCollider->checkCollision(R, &sHitInfo))
      {
         bAnyHit = true;

         if(sHitInfo.Distance < sClosestHitInfo.Distance)
            sClosestHitInfo = sHitInfo;
      }
   }

   if(bAnyHit && OutHitInfo)
      *OutHitInfo = sClosestHitInfo;

   return bAnyHit;
}

bool Physics::checkCollision(Scene* S, ComponentCollider* Collider, const QuerySettings& Settings, HitInfo* OutHitInfo)
{
   GEAssert(S);
   const GESTLVector(Component*)& vColliders = S->getComponents<ComponentCollider>();

   HitInfo sClosestHitInfo;
   sClosestHitInfo.Distance = FLT_MAX;

   ComponentTransform* cColliderTransform = Collider->getOwner()->getComponent<ComponentTransform>();

   for(uint i = 0; i < vColliders.size(); i++)
   {
      if(vColliders[i] == Collider || !vColliders[i]->getOwner()->isActiveInHierarchy())
         continue;

      ComponentCollider* cOtherCollider = static_cast<ComponentCollider*>(vColliders[i]);
      uint iOtherCollisionGroupBitMask = 1 << cOtherCollider->getCollisionGroup();

      if((Settings.CollisionGroupsBitMask & iOtherCollisionGroupBitMask) == 0)
         continue;

      ComponentTransform* cOtherColliderTransform = cOtherCollider->getOwner()->getComponent<ComponentTransform>();

      if(Collider->getColliderType() == ColliderType::Sphere)
      {
         ComponentColliderSphere* cColliderSphere = static_cast<ComponentColliderSphere*>(Collider);
         float fColliderRadius = cColliderSphere->getScaledRadius();

         if(cOtherCollider->getColliderType() == ColliderType::Sphere)
         {
            ComponentColliderSphere* cOtherColliderSphere = static_cast<ComponentColliderSphere*>(cOtherCollider);

            Vector3 vColliderPosition = cColliderTransform->getWorldPosition();
            Vector3 vOtherColliderPosition = cOtherColliderTransform->getWorldPosition();

            float fSqDistance = vColliderPosition.getSquaredDistanceTo(vOtherColliderPosition);
            float fCombinedRadius = fColliderRadius + cOtherColliderSphere->getScaledRadius();

            if(fSqDistance <= (fCombinedRadius * fCombinedRadius))
            {
               if(OutHitInfo)
               {
                  OutHitInfo->Collider = cOtherCollider->getOwner();
                  OutHitInfo->Position = (vColliderPosition + vOtherColliderPosition) * 0.5f;
               }

               return true;
            }
         }
      }
   }

   return false;
}
