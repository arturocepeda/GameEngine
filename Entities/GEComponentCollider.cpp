
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentCollider.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentCollider.h"
#include "GEComponentMesh.h"
#include "GEEntity.h"
#include "Content/GEResourcesManager.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Content;


//
//  ComponentCollider
//
ComponentCollider::ComponentCollider(Entity* Owner, ColliderType Type)
   : Component(Owner)
   , eColliderType(Type)
   , iCollisionGroup(0)
{
   mClassNames.push_back(ObjectName("Collider"));

   GERegisterProperty(UInt, CollisionGroup);
}

ComponentCollider::~ComponentCollider()
{
}

ColliderType ComponentCollider::getColliderType() const
{
   return eColliderType;
}

uint ComponentCollider::getCollisionGroup() const
{
   return iCollisionGroup;
}

void ComponentCollider::setCollisionGroup(uint Group)
{
   iCollisionGroup = Group;
}


//
//  ComponentColliderSphere
//
ComponentColliderSphere::ComponentColliderSphere(Entity* Owner)
   : ComponentCollider(Owner, ColliderType::Sphere)
   , fRadius(1.0f)
{
   mClassNames.push_back(ObjectName("ColliderSphere"));

   GERegisterProperty(Float, Radius);
}

ComponentColliderSphere::~ComponentColliderSphere()
{
}

float ComponentColliderSphere::getRadius() const
{
   return fRadius;
}

void ComponentColliderSphere::setRadius(float Radius)
{
   fRadius = Radius;
}

float ComponentColliderSphere::getScaledRadius() const
{
   return fRadius * cOwner->getComponent<ComponentTransform>()->getWorldScale().X;
}

bool ComponentColliderSphere::checkCollision(const Physics::Ray& R, Physics::HitInfo* OutHitInfo) const
{
   const Vector3& vColliderPosition = cOwner->getComponent<ComponentTransform>()->getWorldPosition();
   Vector3 vRayOriginToCollider = vColliderPosition - R.Origin;
   float fDot = vRayOriginToCollider.dotProduct(R.Direction);

   if(fDot < 0.0f)
      return false;

   float fScaledRadius = getScaledRadius();
   float fSqDistanceToRayOrigin = vRayOriginToCollider.getSquaredLength();
   float fIntersection = (fScaledRadius * fScaledRadius) - (fSqDistanceToRayOrigin - (fDot * fDot));

   if(fIntersection < 0.0f)
      return false;

   if(OutHitInfo)
   {
      OutHitInfo->Collider = cOwner;
      OutHitInfo->Distance = fDot - sqrt(fIntersection);
      OutHitInfo->Position = R.Origin + (R.Direction * OutHitInfo->Distance);
   }

   return true;
}


//
//  ComponentColliderCapsule
//
ComponentColliderCapsule::ComponentColliderCapsule(Entity* Owner)
   : ComponentCollider(Owner, ColliderType::Capsule)
   , fRadius(0.0f)
   , fHeight(0.0f)
{
   mClassNames.push_back(ObjectName("ColliderCapsule"));

   GERegisterProperty(Float, Radius);
   GERegisterProperty(Float, Height);
}

ComponentColliderCapsule::~ComponentColliderCapsule()
{
}

float ComponentColliderCapsule::getRadius() const
{
   return fRadius;
}

void ComponentColliderCapsule::setRadius(float Radius)
{
   fRadius = Radius;
}

float ComponentColliderCapsule::getHeight() const
{
   return fHeight;
}

void ComponentColliderCapsule::setHeight(float Height)
{
   fHeight = Height;
}

bool ComponentColliderCapsule::checkCollision(const Physics::Ray& R, Physics::HitInfo* OutHitInfo) const
{
   return false;
}


//
//  ComponentColliderMesh
//
ComponentColliderMesh::ComponentColliderMesh(Entity* Owner)
   : ComponentCollider(Owner, ColliderType::Mesh)
   , cMesh(0)
{
   mClassNames.push_back(ObjectName("ColliderMesh"));

   GERegisterProperty(ObjectName, MeshName);
}

ComponentColliderMesh::~ComponentColliderMesh()
{
}

const ObjectName& ComponentColliderMesh::getMeshName() const
{
   return cMesh ? cMesh->getName() : ObjectName::Empty;
}

void ComponentColliderMesh::setMeshName(const ObjectName& MeshName)
{
   cMesh = SerializableResourcesManager::getInstance()->get<Mesh>(MeshName);
}

bool ComponentColliderMesh::checkCollision(const Physics::Ray& R, Physics::HitInfo* OutHitInfo) const
{
   if(!cMesh)
      return false;

   ushort* iIndices = cMesh->getGeometryData().Indices;
   const uint iTrianglesCount = cMesh->getGeometryData().NumIndices / 3;
   const uint iFloatsPerVertex = cMesh->getGeometryData().VertexStride / sizeof(float);
   const Matrix4& mWorldTransform = cOwner->getComponent<ComponentTransform>()->getGlobalWorldMatrix();

   Physics::HitInfo sHitInfo;
   float fClosestSqDistance = FLT_MAX;

   for(uint i = 0; i < iTrianglesCount; i++)
   {
      Vector3 v1 =
         *reinterpret_cast<Vector3*>(cMesh->getGeometryData().VertexData + ((uint)(*iIndices) * iFloatsPerVertex)); iIndices++;
      Vector3 v2 =
         *reinterpret_cast<Vector3*>(cMesh->getGeometryData().VertexData + ((uint)(*iIndices) * iFloatsPerVertex)); iIndices++;
      Vector3 v3 =
         *reinterpret_cast<Vector3*>(cMesh->getGeometryData().VertexData + ((uint)(*iIndices) * iFloatsPerVertex)); iIndices++;

      Matrix4Transform(mWorldTransform, &v1);
      Matrix4Transform(mWorldTransform, &v2);
      Matrix4Transform(mWorldTransform, &v3);

      Vector3 e1 = v2 - v1;
      Vector3 e2 = v3 - v1;

      Vector3 vPVec = R.Direction.crossProduct(e2);      
      float fDet = vPVec.dotProduct(e1);

      if(fabsf(fDet) < GE_EPSILON)
         continue;

      float fInvDet = 1.0f / fDet;
      Vector3 vTVec = R.Origin - v1;
      float u = fInvDet * vTVec.dotProduct(vPVec);

      if(u < 0.0f || u > 1.0f)
         continue;

      Vector3 vQVec = vTVec.crossProduct(e1);
      float v = fInvDet * vQVec.dotProduct(R.Direction);

      if(v >= 0.0f && (u + v) <= 1.0f)
      {
         if(!OutHitInfo)
            return true;

         float t = fInvDet * e2.dotProduct(vQVec);
         Vector3 vHitPosition = R.Origin + (R.Direction * t);
         float fSqDistance = R.Origin.getSquaredDistanceTo(vHitPosition);

         if(fSqDistance < fClosestSqDistance)
         {
            sHitInfo.Collider = cOwner;
            sHitInfo.Position = vHitPosition;
            sHitInfo.Normal = e1.crossProduct(e2);
            fClosestSqDistance = fSqDistance;
         }
      }
   }

   if(sHitInfo.Collider)
   {
      sHitInfo.Distance = sqrtf(fClosestSqDistance);
      sHitInfo.Normal.normalize();
      *OutHitInfo = sHitInfo;
      return true;
   }

   return false;
}
