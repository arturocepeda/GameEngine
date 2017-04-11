
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEAnimation.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Core/GEObject.h"
#include "GEResource.h"

namespace GE { namespace Content
{
   struct AnimationKeyFrame
   {
      float TimeInSeconds;
      Vector3 FramePosition;
      Vector3 FrameRotation;
      Vector3 FrameScale;
   };


   class Animation : public Core::Object
   {
   private:
      uint iKeyFramesCount;
      uint iSkeletonBonesCount;
      AnimationKeyFrame** sKeyFrames;

      bool bApplyRootMotionX;
      bool bApplyRootMotionY;
      bool bApplyRootMotionZ;

   public:
      static const uint FileFormatHeaderReservedBytes = 40;

      Animation(const Core::ObjectName& Name, uint KeyFramesCount, uint SkeletonBonesCount);
      ~Animation();

      uint getKeyFramesCount() const;
      uint getSkeletonBonesCount() const;

      AnimationKeyFrame& getKeyFrame(uint KeyFrameIndex, uint BoneIndex);
      AnimationKeyFrame** getKeyFrames();

      float getKeyFrameTime(uint KeyFrameIndex) const;

      bool getApplyRootMotionX() const;
      bool getApplyRootMotionY() const;
      bool getApplyRootMotionZ() const;

      void setApplyRootMotionX(bool Apply);
      void setApplyRootMotionY(bool Apply);
      void setApplyRootMotionZ(bool Apply);
   };


   class AnimationSet : public Resource
   {
   private:
      GESTLVector(Animation*) vAnimations;

      Animation* loadAnimation(const char* FileName, const Core::ObjectName& AnimationName);

   public:
      static const ResourceType Type;

      AnimationSet(const char* FileName);
      ~AnimationSet();

      Animation* getAnimation(const Core::ObjectName& Name);
   };
}}
