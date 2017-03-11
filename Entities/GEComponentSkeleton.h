
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentSkeleton.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEEntity.h"
#include "GEComponent.h"
#include "Content/GESkeleton.h"
#include "Content/GEAnimation.h"

namespace GE { namespace Entities
{
   enum class AnimationPlayMode
   {
      Loop,
      Once
   };


   struct AnimationPlayInfo
   {
      Core::ObjectName AnimationName;
      AnimationPlayMode Mode;

      float BlendTime;
      float Speed;

      AnimationPlayInfo()
         : Mode(AnimationPlayMode::Loop)
         , BlendTime(0.0f)
         , Speed(1.0f)
      {
      }
   };


   enum class AnimationInstanceState
   {
      Playing,
      BlendingIn,
      BlendingOut,
      Inactive
   };


   struct AnimationInstance
   {
      Content::Animation* RefAnimation;
      AnimationPlayMode Mode;
      AnimationInstanceState State;

      float CurrentWeight;
      float Speed;
      float TimePosition;

      float BlendTime;
      float RemainingBlendTime;

      AnimationInstance(Content::Animation* Anim)
         : RefAnimation(Anim)
         , Speed(1.0f)
         , TimePosition(0.0f)
         , BlendTime(0.0f)
         , RemainingBlendTime(0.0f)
      {
      }

      void blendIn(float Time)
      {
         State = AnimationInstanceState::BlendingIn;
         CurrentWeight = 0.0f;
         BlendTime = Time;
         RemainingBlendTime = Time;
      }

      void blendOut(float Time)
      {
         State = AnimationInstanceState::BlendingOut;
         CurrentWeight = 1.0f;
         BlendTime = Time;
         RemainingBlendTime = Time;
      }
   };


   class ComponentSkeleton : public Component
   {
   public:
      typedef std::function<void(ComponentSkeleton*)> Callback;

   private:
      Content::Skeleton* cSkeleton;
      GESTLVector(Entity*) vBoneEntities;

      Matrix4* sBoneMatrices;
      Matrix4* sBoneInverseTransposeMatrices;

      Content::AnimationSet* cAnimationSet;

      GESTLVector(AnimationInstance) vActiveAnimationInstances;
      Core::ObjectName cDefaultAnimationName;

      Matrix4* mBonePoseMatrix;
      float fAnimationSpeedFactor;

      Callback onAnimationInstancesUpdated;

      void updateBoneMatrices();
      void updateSkinnedMeshes();

      void updateAnimationInstances();
      void updateAnimationInstance(AnimationInstance* cInstance);

      void releaseSkeletonData();

   public:
      static ComponentType getType() { return ComponentType::Skeleton; }

      ComponentSkeleton(Entity* Owner);
      ~ComponentSkeleton();

      const Core::ObjectName& getSkeletonName() const;
      void setSkeletonName(const Core::ObjectName& SkeletonName);

      Content::Skeleton* getSkeleton() const;
      Entity* getBoneEntity(uint BoneIndex) const;

      const Matrix4* getBoneMatrices() const;
      const Matrix4* getBoneInverseTransposeMatrices() const;

      Content::AnimationSet* getAnimationSet() const;
      void setAnimationSet(Content::AnimationSet* Set);

      const Core::ObjectName& getAnimationSetName() const;
      void setAnimationSetName(const Core::ObjectName& Name);

      const Core::ObjectName& getDefaultAnimationName() const;
      void setDefaultAnimationName(const Core::ObjectName& Name);

      float getAnimationSpeedFactor() const;
      void setAnimationSpeedFactor(float Factor);

      void playAnimation(const AnimationPlayInfo& PlayInfo);
      void stopAllAnimations();

      uint getAnimationInstancesCount() const;
      AnimationInstance& getAnimationInstance(uint Index);

      void setCallbackOnAnimationInstancesUpdated(Callback fCallback);

      void update();

      GEProperty(ObjectName, SkeletonName)
      GEProperty(ObjectName, AnimationSetName)
      GEProperty(ObjectName, DefaultAnimationName)
      GEProperty(Float, AnimationSpeedFactor)
   };
}}
