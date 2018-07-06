
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEAnimation.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAnimation.h"
#include "Core/GEAllocator.h"
#include "Core/GEParser.h"
#include "Core/GEApplication.h"
#include "Core/GEValue.h"
#include "Core/GEDevice.h"
#include "Content/GEContentData.h"
#include "pugixml/pugixml.hpp"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;


//
//  Animation
//
Animation::Animation(const ObjectName& Name, uint KeyFramesCount, uint SkeletonBonesCount)
   : Object(Name)
   , iKeyFramesCount(KeyFramesCount)
   , iSkeletonBonesCount(SkeletonBonesCount)
   , bApplyRootMotionX(true)
   , bApplyRootMotionY(true)
   , bApplyRootMotionZ(true)
{
   sKeyFrames = Allocator::alloc<AnimationKeyFrame*>(iKeyFramesCount);

   for(uint i = 0; i < iKeyFramesCount; i++)
   {
      sKeyFrames[i] = Allocator::alloc<AnimationKeyFrame>(iSkeletonBonesCount);

      for(uint j = 0; j < iSkeletonBonesCount; j++)
      {
         memset(&sKeyFrames[i][j], 0, sizeof(AnimationKeyFrame));
         sKeyFrames[i][j].TimeInSeconds = -1.0f;
      }
   }
}

Animation::~Animation()
{
   for(uint i = 0; i < iKeyFramesCount; i++)
      Allocator::free(sKeyFrames[i]);

   Allocator::free(sKeyFrames);
}

uint Animation::getKeyFramesCount() const
{
   return iKeyFramesCount;
}

uint Animation::getSkeletonBonesCount() const
{
   return iSkeletonBonesCount;
}

AnimationKeyFrame& Animation::getKeyFrame(uint KeyFrameIndex, uint BoneIndex)
{
   GEAssert(KeyFrameIndex < iKeyFramesCount);
   GEAssert(BoneIndex < iSkeletonBonesCount);
   return sKeyFrames[KeyFrameIndex][BoneIndex];
}

AnimationKeyFrame** Animation::getKeyFrames()
{
   return sKeyFrames;
}

float Animation::getKeyFrameTime(uint KeyFrameIndex) const
{
   GEAssert(KeyFrameIndex < iKeyFramesCount);
   uint i = 0;

   while(sKeyFrames[KeyFrameIndex][i].TimeInSeconds < 0.0f)
      i++;

   return sKeyFrames[KeyFrameIndex][i].TimeInSeconds;
}

bool Animation::getApplyRootMotionX() const
{
   return bApplyRootMotionX;
}

bool Animation::getApplyRootMotionY() const
{
   return bApplyRootMotionY;
}

bool Animation::getApplyRootMotionZ() const
{
   return bApplyRootMotionZ;
}

void Animation::setApplyRootMotionX(bool Apply)
{
   bApplyRootMotionX = Apply;
}

void Animation::setApplyRootMotionY(bool Apply)
{
   bApplyRootMotionY = Apply;
}

void Animation::setApplyRootMotionZ(bool Apply)
{
   bApplyRootMotionZ = Apply;
}



//
//  AnimationSet
//
const ResourceType AnimationSet::Type = ResourceType::AnimationSet;

AnimationSet::AnimationSet(const char* FileName)
   : Resource(FileName, ObjectName::Empty, Type)
{
   char sFileName[64];
   sprintf(sFileName, "%s.animationset", FileName);

   ContentData cAnimationSetData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(Content::ContentType::GenericTextData, "Animations", sFileName, "xml", &cAnimationSetData);

      pugi::xml_document xml;
      xml.load_buffer(cAnimationSetData.getData(), cAnimationSetData.getDataSize());
      const pugi::xml_node& xmlAnimationSet = xml.child("AnimationSet");

      for(const pugi::xml_node& xmlAnimation : xmlAnimationSet.children("Animation"))
      {
         ObjectName cAnimationName = xmlAnimation.attribute("name").value();
         const char* sAnimationFileName = xmlAnimation.attribute("fileName").value();

         Animation* cAnimation = loadAnimation(sAnimationFileName, FileName, cAnimationName);
         mAnimations.add(cAnimation);

         pugi::xml_attribute xmlAnimationApplyRootMotionX = xmlAnimation.attribute("applyRootMotionX");

         if(!xmlAnimationApplyRootMotionX.empty())
            cAnimation->setApplyRootMotionX(Parser::parseBool(xmlAnimationApplyRootMotionX.value()));

         pugi::xml_attribute xmlAnimationApplyRootMotionY = xmlAnimation.attribute("applyRootMotionY");

         if(!xmlAnimationApplyRootMotionY.empty())
            cAnimation->setApplyRootMotionY(Parser::parseBool(xmlAnimationApplyRootMotionY.value()));

         pugi::xml_attribute xmlAnimationApplyRootMotionZ = xmlAnimation.attribute("applyRootMotionZ");

         if(!xmlAnimationApplyRootMotionZ.empty())
            cAnimation->setApplyRootMotionZ(Parser::parseBool(xmlAnimationApplyRootMotionZ.value()));
      }
   }
   else
   {
      Device::readContentFile(Content::ContentType::GenericBinaryData, "Animations", sFileName, "ge", &cAnimationSetData);
      ContentDataMemoryBuffer sMemoryBuffer(cAnimationSetData);
      std::istream sStream(&sMemoryBuffer);

      uint iAnimationsCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iAnimationsCount; i++)
      {
         ObjectName cAnimationName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         Value cAnimationFileName = Value::fromStream(ValueType::String, sStream);

         Animation* cAnimation = loadAnimation(cAnimationFileName.getAsString(), FileName, cAnimationName);
         mAnimations.add(cAnimation);

         cAnimation->setApplyRootMotionX(Value::fromStream(ValueType::Bool, sStream).getAsBool());
         cAnimation->setApplyRootMotionY(Value::fromStream(ValueType::Bool, sStream).getAsBool());
         cAnimation->setApplyRootMotionZ(Value::fromStream(ValueType::Bool, sStream).getAsBool());
      }
   }
}

AnimationSet::~AnimationSet()
{
}

Animation* AnimationSet::loadAnimation(const char* FileName, const char* AnimationSetName, const ObjectName& AnimationName)
{
   char sSubDir[256];
   sprintf(sSubDir, "Animations/%s", AnimationSetName);

   ContentData cContent;
   Device::readContentFile(Content::ContentType::GenericBinaryData, sSubDir, FileName, "animation.ge", &cContent);

   char* pData = cContent.getData();

   // "GEMeshAnimation "
   pData += 16;

   //TODO: remove the name from the animation files
   pData += 32;

   // animation info
   uint iKeyFramesCount = *reinterpret_cast<uint*>(pData);
   pData += sizeof(uint);
   uint iSkeletonBonesCount = *reinterpret_cast<uint*>(pData);
   pData += sizeof(uint);

   Animation* cAnimation = Allocator::alloc<Animation>();
   GEInvokeCtor(Animation, cAnimation)(AnimationName, iKeyFramesCount, iSkeletonBonesCount);

   // reserved
   pData += Animation::FileFormatHeaderReservedBytes;

   // fill key frames data
   for(uint iKeyFrameIndex = 0; iKeyFrameIndex < iKeyFramesCount; iKeyFrameIndex++)
   {
      for(uint iBoneIndex = 0; iBoneIndex < iSkeletonBonesCount; iBoneIndex++)
      {
         AnimationKeyFrame& cKeyFrame = cAnimation->getKeyFrame(iKeyFrameIndex, iBoneIndex);

         cKeyFrame.TimeInSeconds = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);

         cKeyFrame.FramePosition.X = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);
         cKeyFrame.FramePosition.Y = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);
         cKeyFrame.FramePosition.Z = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);

         Vector3 vEulerRotation;
         vEulerRotation.X = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);
         vEulerRotation.Y = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);
         vEulerRotation.Z = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);

         cKeyFrame.FrameRotation = Rotation(vEulerRotation * GE_DEG2RAD);

         cKeyFrame.FrameScale.X = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);
         cKeyFrame.FrameScale.Y = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);
         cKeyFrame.FrameScale.Z = *reinterpret_cast<float*>(pData);
         pData += sizeof(float);
      }
   }

   return cAnimation;
}

Animation* AnimationSet::getAnimation(const ObjectName& Name)
{
   return mAnimations.get(Name);
}

const ObjectRegistry* AnimationSet::getObjectRegistry()
{
   return mAnimations.getObjectRegistry();
}
