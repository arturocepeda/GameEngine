
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentSkeleton.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentSkeleton.h"
#include "GEComponentTransform.h"
#include "GEComponentMesh.h"
#include "Content/GEResourcesManager.h"
#include "Core/GEGeometry.h"
#include "Core/GETime.h"
#include "Core/GEEvents.h"
#include "Core/GEProfiler.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;
using namespace GE::Entities;


//
//  ComponentSkeleton
//
ComponentSkeleton::ComponentSkeleton(Entity* Owner)
   : Component(Owner)
   , cSkeleton(0)
   , sBoneMatrices(0)
   , sBoneInverseTransposeMatrices(0)
   , mBonePoseMatrix(0)
   , cAnimationSet(0)
   , fDefaultBlendingTime(0.0f)
   , fAnimationSpeedFactor(1.0f)
{
   mClassNames.push_back(ObjectName("Skeleton"));

   GERegisterProperty(ObjectName, SkeletonName);
   GERegisterProperty(ObjectName, AnimationSetName);
   GERegisterProperty(ObjectName, DefaultAnimationName);
   GERegisterProperty(Float, DefaultBlendingTime);
   GERegisterProperty(Float, AnimationSpeedFactor);
}

ComponentSkeleton::~ComponentSkeleton()
{
   releaseSkeletonData();
}

const ObjectName& ComponentSkeleton::getSkeletonName() const
{
   return cSkeleton ? cSkeleton->getName() : ObjectName::Empty;
}

void ComponentSkeleton::setSkeletonName(const ObjectName& SkeletonName)
{
   if(cSkeleton && cSkeleton->getName() == SkeletonName)
      return;

   releaseSkeletonData();

   if(SkeletonName.isEmpty())
   {
      cSkeleton = 0;
      return;
   }

   cSkeleton = ResourcesManager::getInstance()->get<Skeleton>(SkeletonName);

   if(!cSkeleton)
      cSkeleton = ResourcesManager::getInstance()->load<Skeleton>(SkeletonName.getString());

   GEAssert(cSkeleton);

   const uint iBonesCount = cSkeleton->getBonesCount();

   if(iBonesCount > 1)
   {
      for(uint i = 0; i < iBonesCount; i++)
      {
         const Bone* cCurrentBone = cSkeleton->getBone(i);

         // create entity
         Entity* cBoneEntity = cOwner->getOwner()->addEntity(cCurrentBone->getName(), cOwner);
         cBoneEntity->setInternalFlags((uint8_t)Entity::InternalFlags::Generated);
         vBoneEntities.push_back(cBoneEntity);

         Vector3 vBoneEntityPosition;
         Rotation cBoneEntityRotation;
         Vector3 vBoneEntityScale;

         Geometry::extractTRSFromMatrix(cCurrentBone->getBindMatrix(),
            &vBoneEntityPosition, &cBoneEntityRotation, &vBoneEntityScale);

         ComponentTransform* cBoneEntityTransform = cBoneEntity->addComponent<ComponentTransform>();
         cBoneEntityTransform->setPosition(vBoneEntityPosition);
         cBoneEntityTransform->setRotation(cBoneEntityRotation);
         cBoneEntityTransform->setScale(vBoneEntityScale);
      }

      // setup the hierarchy in the bone entities
      const Bone* cSkeletonRootBone = cSkeleton->getBone(0);

      for(uint i = 0; i < iBonesCount; i++)
      {
         const Bone* cCurrentBone = cSkeleton->getBone(i);

         if(cCurrentBone != cSkeletonRootBone)
         {
            Entity* cBoneEntityParent = vBoneEntities[cCurrentBone->getParentIndex()];
            Scene* cScene = vBoneEntities[i]->getOwner();
            cScene->setEntityParent(vBoneEntities[i], cBoneEntityParent);
         }
      }
   }
   else
   {
      vBoneEntities.push_back(cOwner);
   }

   // allocate memory for the bone matrices
   sBoneMatrices = Allocator::alloc<Matrix4>(iBonesCount);
   sBoneInverseTransposeMatrices = Allocator::alloc<Matrix4>(iBonesCount);
   mBonePoseMatrix = Allocator::alloc<Matrix4>(iBonesCount);

   // reload mesh
   ComponentMesh* cMesh = cOwner->getComponent<ComponentMesh>();

   if(cMesh && cMesh->getMesh())
      cMesh->loadMesh(cMesh->getMesh());
}

Content::Skeleton* ComponentSkeleton::getSkeleton() const
{
   return cSkeleton;
}

Entity* ComponentSkeleton::getBoneEntity(uint BoneIndex) const
{
   GEAssert(BoneIndex < vBoneEntities.size());
   return vBoneEntities[BoneIndex];
}

Entity* ComponentSkeleton::getBoneEntity(const Core::ObjectName& BoneName) const
{
   for(uint i = 0; i < vBoneEntities.size(); i++)
   {
      if(vBoneEntities[i]->getName() == BoneName)
         return vBoneEntities[i];
   }

   return 0;
}

const Matrix4* ComponentSkeleton::getBoneMatrices() const
{
   return sBoneMatrices;
}

const Matrix4* ComponentSkeleton::getBoneInverseTransposeMatrices() const
{
   return sBoneInverseTransposeMatrices;
}

void ComponentSkeleton::updateBoneMatrices()
{
   const uint iBonesCount = cSkeleton->getBonesCount();

   for(uint i = 0; i < iBonesCount; i++)
   {
      const Bone* cBone = cSkeleton->getBone(i);
      Entity* cBoneEntity = vBoneEntities[i];
      sBoneMatrices[i] = cBone->getInverseBindMatrix();

      do
      {
         ComponentTransform* cBoneEntityTransform = cBoneEntity->getComponent<ComponentTransform>();
         Matrix4 mBoneMatrixSoFar = sBoneMatrices[i];
         Matrix4Multiply(cBoneEntityTransform->getLocalWorldMatrix(), mBoneMatrixSoFar, &sBoneMatrices[i]);
         cBoneEntity = cBoneEntity->getParent();
      }
      while(cBoneEntity && cBoneEntity != cOwner);

      sBoneInverseTransposeMatrices[i] = sBoneMatrices[i];
      Matrix4Invert(&sBoneInverseTransposeMatrices[i]);
      Matrix4Transpose(&sBoneInverseTransposeMatrices[i]);
   }
}

void ComponentSkeleton::updateSkinnedMeshes()
{
   ComponentRenderable* cRenderable = cOwner->getComponent<ComponentRenderable>();

   if(cRenderable && cRenderable->getClassName() == ComponentMesh::ClassName)
   {
      ComponentMesh* cComponentMesh = static_cast<ComponentMesh*>(cRenderable);
      Mesh* cMesh = cComponentMesh->getMesh();

      if(cMesh && cMesh->isSkinned())
      {
         cComponentMesh->updateSkinning();
      }
   }

   for(uint i = 0; i < cOwner->getChildrenCount(); i++)
   {
      cRenderable = cOwner->getChildByIndex(i)->getComponent<ComponentRenderable>();

      if(cRenderable && cRenderable->getClassName() == ComponentMesh::ClassName)
      {
         ComponentMesh* cComponentMesh = static_cast<ComponentMesh*>(cRenderable);
         Mesh* cMesh = cComponentMesh->getMesh();

         if(cMesh && cMesh->isSkinned())
         {
            cComponentMesh->updateSkinning();
         }
      }
   }
}

AnimationSet* ComponentSkeleton::getAnimationSet() const
{
   return cAnimationSet;
}

void ComponentSkeleton::setAnimationSet(AnimationSet* Set)
{
   cAnimationSet = Set;
   cDefaultAnimationName = ObjectName::Empty;

#if defined (GE_EDITOR_SUPPORT)
   Property* cDefaultAnimationNameProperty = const_cast<Property*>(getProperty("DefaultAnimationName"));
   cDefaultAnimationNameProperty->DataPtr = Set ? (void*)Set->getObjectRegistry() : 0;

   EventArgs sArgs;
   sArgs.Data = this;
   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
#endif
}

const ObjectName& ComponentSkeleton::getAnimationSetName() const
{
   return cAnimationSet ? cAnimationSet->getName() : ObjectName::Empty;
}

void ComponentSkeleton::setAnimationSetName(const ObjectName& Name)
{
   AnimationSet* animationSet = ResourcesManager::getInstance()->get<AnimationSet>(Name);

   if(!animationSet)
      animationSet = ResourcesManager::getInstance()->load<AnimationSet>(Name.getString());

   setAnimationSet(animationSet);
}

const ObjectName& ComponentSkeleton::getDefaultAnimationName() const
{
   return cDefaultAnimationName;
}

void ComponentSkeleton::setDefaultAnimationName(const ObjectName& Name)
{
   cDefaultAnimationName = Name;

   if(!cAnimationSet)
      return;

   AnimationPlayInfo sPlayInfo;
   sPlayInfo.AnimationName = Name;
   sPlayInfo.BlendTime = fDefaultBlendingTime;
   playAnimation(sPlayInfo);
}

float ComponentSkeleton::getDefaultBlendingTime() const
{
   return fDefaultBlendingTime;
}

void ComponentSkeleton::setDefaultBlendingTime(float Time)
{
   fDefaultBlendingTime = Time;
}

float ComponentSkeleton::getAnimationSpeedFactor() const
{
   return fAnimationSpeedFactor;
}

void ComponentSkeleton::setAnimationSpeedFactor(float Factor)
{
   fAnimationSpeedFactor = Factor;
}

void ComponentSkeleton::playAnimation(const AnimationPlayInfo& PlayInfo)
{
   if(!cAnimationSet)
      return;

   Animation* cAnimation = cAnimationSet->getAnimation(PlayInfo.AnimationName);
   
   if(!cAnimation)
      return;

   for(uint i = 0; i < vActiveAnimationInstances.size(); i++)
   {
      if(vActiveAnimationInstances[i].RefAnimation == cAnimation)
         return;
   }

   vActiveAnimationInstances.push_back(AnimationInstance(cAnimation));
   AnimationInstance& cInstance = vActiveAnimationInstances.back();
   cInstance.Mode = PlayInfo.Mode;
   cInstance.Speed = PlayInfo.Speed;

   uint iInstanceLastIndex = (uint)(vActiveAnimationInstances.size() - 1);

   if(PlayInfo.BlendTime > GE_EPSILON)
   {
      cInstance.blendIn(PlayInfo.BlendTime);

      for(uint i = 0; i < iInstanceLastIndex; i++)
         vActiveAnimationInstances[i].blendOut(PlayInfo.BlendTime);
   }
   else
   {
      cInstance.State = AnimationInstanceState::Playing;
      cInstance.CurrentWeight = 1.0f;

      for(uint i = 0; i < iInstanceLastIndex; i++)
         vActiveAnimationInstances.erase(vActiveAnimationInstances.begin());
   }

   if(cSkeleton->getBonesCount() == 1)
   {
      updateAnimationInstances(0.0f);
   }
}

void ComponentSkeleton::stopAllAnimations()
{
   vActiveAnimationInstances.clear();
}

uint ComponentSkeleton::getAnimationInstancesCount() const
{
   return (uint)vActiveAnimationInstances.size();
}

AnimationInstance& ComponentSkeleton::getAnimationInstance(uint Index)
{
   GEAssert(Index < vActiveAnimationInstances.size());
   return vActiveAnimationInstances[Index];
}

void ComponentSkeleton::setCallbackOnAnimationInstancesUpdated(Callback fCallback)
{
   onAnimationInstancesUpdated = fCallback;
}

void ComponentSkeleton::updateAnimationInstances(float fDeltaTime)
{
   if(vActiveAnimationInstances.empty())
      return;

   const uint iBonesCount = cSkeleton->getBonesCount();

   // update animation instances
   memset(mBonePoseMatrix, 0, sizeof(Matrix4) * iBonesCount);

   for(uint iInstanceIndex = 0; iInstanceIndex < vActiveAnimationInstances.size(); iInstanceIndex++)
      updateAnimationInstance(&vActiveAnimationInstances[iInstanceIndex], fDeltaTime);

   // update bone transform matrices
   for(uint iBoneIndex = 0; iBoneIndex < iBonesCount; iBoneIndex++)
   {
      Entity* cBoneEntity = vBoneEntities[iBoneIndex];
      ComponentTransform* cBoneEntityTransform = cBoneEntity->getComponent<ComponentTransform>();
      cBoneEntityTransform->setLocalWorldMatrix(mBonePoseMatrix[iBoneIndex]);
   }

   // remove inactive instances
   for(uint iInstanceIndex = 0; iInstanceIndex < vActiveAnimationInstances.size(); iInstanceIndex++)
   {
      if(vActiveAnimationInstances[iInstanceIndex].State == AnimationInstanceState::Inactive)
      {
         vActiveAnimationInstances.erase(vActiveAnimationInstances.begin() + iInstanceIndex);
         iInstanceIndex--;
      }
   }
}

void ComponentSkeleton::updateAnimationInstance(AnimationInstance* cInstance, float fDeltaTime)
{
   cInstance->TimePosition += fDeltaTime * cInstance->Speed * fAnimationSpeedFactor;

   switch(cInstance->State)
   {
   case AnimationInstanceState::Inactive:
   case AnimationInstanceState::Playing:
      break;

   case AnimationInstanceState::BlendingIn:
      {
         cInstance->RemainingBlendTime -= fDeltaTime;

         if(cInstance->RemainingBlendTime > 0.0f)
         {
            cInstance->CurrentWeight = 1.0f - (cInstance->RemainingBlendTime / cInstance->BlendTime);
         }
         else
         {
            cInstance->RemainingBlendTime = 0.0f;
            cInstance->CurrentWeight = 1.0f;
            cInstance->State = AnimationInstanceState::Playing;
         }
      }
      break;

   case AnimationInstanceState::BlendingOut:
      {
         cInstance->RemainingBlendTime -= fDeltaTime;

         if(cInstance->RemainingBlendTime > 0.0f)
         {
            cInstance->CurrentWeight = cInstance->RemainingBlendTime / cInstance->BlendTime;
         }
         else
         {
            cInstance->RemainingBlendTime = 0.0f;
            cInstance->CurrentWeight = 0.0f;
            cInstance->State = AnimationInstanceState::Inactive;
         }
      }
      break;
   }

   uint iLastKeyFrameIndex = cInstance->RefAnimation->getKeyFramesCount() - 1;
   float fTimeLastKeyFrame = cInstance->RefAnimation->getKeyFrameTime(iLastKeyFrameIndex);

   if(cInstance->TimePosition >= fTimeLastKeyFrame)
   {
      if(cInstance->Mode == AnimationPlayMode::Loop)
      {
         cInstance->TimePosition -= fTimeLastKeyFrame;
      }
      else
      {
         cInstance->State = AnimationInstanceState::Inactive;
      }
   }

   uint iKeyFrameIndexA = 0;

   for(; iKeyFrameIndexA < iLastKeyFrameIndex; iKeyFrameIndexA++)
   {
      if(cInstance->TimePosition < cInstance->RefAnimation->getKeyFrameTime(iKeyFrameIndexA + 1))
         break;
   }

   uint iKeyFrameIndexB = iKeyFrameIndexA + 1;

   if(iKeyFrameIndexB > iLastKeyFrameIndex)
      iKeyFrameIndexB = iLastKeyFrameIndex;

   float fLerpFactor = 0.0f;

   if(iKeyFrameIndexA != iKeyFrameIndexB)
   {
      float fTimeA = cInstance->RefAnimation->getKeyFrameTime(iKeyFrameIndexA);
      float fTimeB = cInstance->RefAnimation->getKeyFrameTime(iKeyFrameIndexB);

      Scaler s(fTimeA, fTimeB, 0.0f, 1.0f);
      fLerpFactor = s.y(cInstance->TimePosition);
   }

   // set position and rotation for each bone
   const uint iBonesCount = cSkeleton->getBonesCount();

   for(uint iBoneIndex = 0; iBoneIndex < iBonesCount; iBoneIndex++)
   {
      AnimationKeyFrame& sKeyFrameA = cInstance->RefAnimation->getKeyFrame(iKeyFrameIndexA, iBoneIndex);
      AnimationKeyFrame& sKeyFrameB = cInstance->RefAnimation->getKeyFrame(iKeyFrameIndexB, iBoneIndex);

      if(sKeyFrameA.TimeInSeconds < 0.0f || sKeyFrameB.TimeInSeconds < 0.0f)
         continue;

      Vector3 vPosition = Vector3::lerp(sKeyFrameA.FramePosition, sKeyFrameB.FramePosition, fLerpFactor);
      Vector3 vScale = Vector3::lerp(sKeyFrameA.FrameScale, sKeyFrameB.FrameScale, fLerpFactor);

      const Quaternion& qQuaternionA = sKeyFrameA.FrameRotation.getQuaternion();
      const Quaternion& qQuaternionB = sKeyFrameB.FrameRotation.getQuaternion();
      Quaternion qQuaternion = Quaternion::slerp(qQuaternionA, qQuaternionB, fLerpFactor);

      Matrix4 mBoneInstancePoseMatrix;
      Geometry::createTRSMatrix(vPosition, Rotation(qQuaternion), vScale, &mBoneInstancePoseMatrix);

      Matrix4 mBonePosePartialMatrix;
      Matrix4Multiply(mBoneInstancePoseMatrix, cInstance->CurrentWeight, &mBonePosePartialMatrix);

      if(iBoneIndex == 0)
      {
         mBonePoseMatrix[iBoneIndex].m[GE_M4_1_1] += mBonePosePartialMatrix.m[GE_M4_1_1];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_2_1] += mBonePosePartialMatrix.m[GE_M4_2_1];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_3_1] += mBonePosePartialMatrix.m[GE_M4_3_1];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_4_1] += mBonePosePartialMatrix.m[GE_M4_4_1];

         mBonePoseMatrix[iBoneIndex].m[GE_M4_1_2] += mBonePosePartialMatrix.m[GE_M4_1_2];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_2_2] += mBonePosePartialMatrix.m[GE_M4_2_2];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_3_2] += mBonePosePartialMatrix.m[GE_M4_3_2];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_4_2] += mBonePosePartialMatrix.m[GE_M4_4_2];

         mBonePoseMatrix[iBoneIndex].m[GE_M4_1_3] += mBonePosePartialMatrix.m[GE_M4_1_3];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_2_3] += mBonePosePartialMatrix.m[GE_M4_2_3];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_3_3] += mBonePosePartialMatrix.m[GE_M4_3_3];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_4_3] += mBonePosePartialMatrix.m[GE_M4_4_3];

         if(cInstance->RefAnimation->getApplyRootMotionX())
            mBonePoseMatrix[iBoneIndex].m[GE_M4_1_4] += mBonePosePartialMatrix.m[GE_M4_1_4];

         if(cInstance->RefAnimation->getApplyRootMotionY())
            mBonePoseMatrix[iBoneIndex].m[GE_M4_2_4] += mBonePosePartialMatrix.m[GE_M4_2_4];

         if(cInstance->RefAnimation->getApplyRootMotionZ())
            mBonePoseMatrix[iBoneIndex].m[GE_M4_3_4] += mBonePosePartialMatrix.m[GE_M4_3_4];

         mBonePoseMatrix[iBoneIndex].m[GE_M4_4_4] += mBonePosePartialMatrix.m[GE_M4_4_4];
      }
      else
      {
         mBonePoseMatrix[iBoneIndex].m[GE_M4_1_1] += mBonePosePartialMatrix.m[GE_M4_1_1];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_2_1] += mBonePosePartialMatrix.m[GE_M4_2_1];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_3_1] += mBonePosePartialMatrix.m[GE_M4_3_1];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_4_1] += mBonePosePartialMatrix.m[GE_M4_4_1];

         mBonePoseMatrix[iBoneIndex].m[GE_M4_1_2] += mBonePosePartialMatrix.m[GE_M4_1_2];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_2_2] += mBonePosePartialMatrix.m[GE_M4_2_2];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_3_2] += mBonePosePartialMatrix.m[GE_M4_3_2];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_4_2] += mBonePosePartialMatrix.m[GE_M4_4_2];

         mBonePoseMatrix[iBoneIndex].m[GE_M4_1_3] += mBonePosePartialMatrix.m[GE_M4_1_3];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_2_3] += mBonePosePartialMatrix.m[GE_M4_2_3];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_3_3] += mBonePosePartialMatrix.m[GE_M4_3_3];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_4_3] += mBonePosePartialMatrix.m[GE_M4_4_3];

         mBonePoseMatrix[iBoneIndex].m[GE_M4_1_4] += mBonePosePartialMatrix.m[GE_M4_1_4];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_2_4] += mBonePosePartialMatrix.m[GE_M4_2_4];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_3_4] += mBonePosePartialMatrix.m[GE_M4_3_4];
         mBonePoseMatrix[iBoneIndex].m[GE_M4_4_4] += mBonePosePartialMatrix.m[GE_M4_4_4];
      }
   }
}

void ComponentSkeleton::releaseSkeletonData()
{
   stopAllAnimations();

   if(sBoneMatrices)
      Allocator::free(sBoneMatrices);

   if(sBoneInverseTransposeMatrices)
      Allocator::free(sBoneInverseTransposeMatrices);

   if(mBonePoseMatrix)
      Allocator::free(mBonePoseMatrix);

   if(cSkeleton)
   {
      if(vBoneEntities.size() > 1)
      {
         cOwner->getOwner()->removeEntity(vBoneEntities[0]->getFullName());
      }

      vBoneEntities.clear();
      cSkeleton = 0;
   }
}

void ComponentSkeleton::update()
{
   if(!cSkeleton)
      return;

   GEProfilerMarker("ComponentSkeleton::update()");

   const float fDeltaTime = cOwner->getClock()->getDelta();
   updateAnimationInstances(fDeltaTime);

   if(onAnimationInstancesUpdated)
   {
      onAnimationInstancesUpdated(this);
   }

   updateBoneMatrices();
   updateSkinnedMeshes();
}
