
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
const ObjectName AnimationSet::TypeName = ObjectName("AnimationSet");

AnimationSet::AnimationSet(const char* FileName)
   : Resource(FileName, ObjectName::Empty, TypeName)
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

         char sSubDir[256];
         sprintf(sSubDir, "Animations/%s", FileName);

         ContentData animationData;
         Device::readContentFile(Content::ContentType::GenericBinaryData, sSubDir, sAnimationFileName, "animation.ge", &animationData);
         ContentDataMemoryBuffer memoryBuffer(animationData);
         std::istream stream(&memoryBuffer);

         Animation* cAnimation = loadAnimation(cAnimationName, stream);
         mAnimations.add(cAnimation);

         pugi::xml_attribute xmlAnimationApplyRootMotionX = xmlAnimation.attribute("applyRootMotionX");

         if(!xmlAnimationApplyRootMotionX.empty())
         {
            cAnimation->setApplyRootMotionX(Parser::parseBool(xmlAnimationApplyRootMotionX.value()));
         }

         pugi::xml_attribute xmlAnimationApplyRootMotionY = xmlAnimation.attribute("applyRootMotionY");

         if(!xmlAnimationApplyRootMotionY.empty())
         {
            cAnimation->setApplyRootMotionY(Parser::parseBool(xmlAnimationApplyRootMotionY.value()));
         }

         pugi::xml_attribute xmlAnimationApplyRootMotionZ = xmlAnimation.attribute("applyRootMotionZ");

         if(!xmlAnimationApplyRootMotionZ.empty())
         {
            cAnimation->setApplyRootMotionZ(Parser::parseBool(xmlAnimationApplyRootMotionZ.value()));
         }
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
         const ObjectName cAnimationName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         Value::fromStream(ValueType::String, sStream);

         const bool applyRootMotionX = Value::fromStream(ValueType::Bool, sStream).getAsBool();
         const bool applyRootMotionY = Value::fromStream(ValueType::Bool, sStream).getAsBool();
         const bool applyRootMotionZ = Value::fromStream(ValueType::Bool, sStream).getAsBool();

         Value::fromStream(ValueType::UInt, sStream);

         Animation* cAnimation = loadAnimation(cAnimationName, sStream);
         mAnimations.add(cAnimation);

         cAnimation->setApplyRootMotionX(applyRootMotionX);
         cAnimation->setApplyRootMotionY(applyRootMotionY);
         cAnimation->setApplyRootMotionZ(applyRootMotionZ);
      }
   }
}

AnimationSet::~AnimationSet()
{
}

Animation* AnimationSet::loadAnimation(const Core::ObjectName& pAnimationName, std::istream& pStream)
{
   // "GEMeshAnimation "
   char buffer[64];
   pStream.read(buffer, 16);

   //TODO: remove the name from the animation files
   pStream.read(buffer, 32);

   // animation info
   uint32_t keyFramesCount;
   pStream.read(reinterpret_cast<char*>(&keyFramesCount), sizeof(uint32_t));
   uint32_t skeletonBonesCount;
   pStream.read(reinterpret_cast<char*>(&skeletonBonesCount), sizeof(uint32_t));

   Animation* animation = Allocator::alloc<Animation>();
   GEInvokeCtor(Animation, animation)(pAnimationName, keyFramesCount, skeletonBonesCount);

   // reserved
   pStream.read(buffer, Animation::FileFormatHeaderReservedBytes);

   // fill key frames data
   for(uint32_t keyFrameIndex = 0; keyFrameIndex < keyFramesCount; keyFrameIndex++)
   {
      for(uint32_t boneIndex = 0; boneIndex < skeletonBonesCount; boneIndex++)
      {
         AnimationKeyFrame& keyFrame = animation->getKeyFrame(keyFrameIndex, boneIndex);

         pStream.read(reinterpret_cast<char*>(&keyFrame.TimeInSeconds), sizeof(float));

         pStream.read(reinterpret_cast<char*>(&keyFrame.FramePosition.X), sizeof(float));
         pStream.read(reinterpret_cast<char*>(&keyFrame.FramePosition.Y), sizeof(float));
         pStream.read(reinterpret_cast<char*>(&keyFrame.FramePosition.Z), sizeof(float));

         Vector3 eulerRotation;
         pStream.read(reinterpret_cast<char*>(&eulerRotation.X), sizeof(float));
         pStream.read(reinterpret_cast<char*>(&eulerRotation.Y), sizeof(float));
         pStream.read(reinterpret_cast<char*>(&eulerRotation.Z), sizeof(float));
         keyFrame.FrameRotation = Rotation(eulerRotation * GE_DEG2RAD);

         pStream.read(reinterpret_cast<char*>(&keyFrame.FrameScale.X), sizeof(float));
         pStream.read(reinterpret_cast<char*>(&keyFrame.FrameScale.Y), sizeof(float));
         pStream.read(reinterpret_cast<char*>(&keyFrame.FrameScale.Z), sizeof(float));
      }
   }

   return animation;
}

Animation* AnimationSet::getAnimation(const ObjectName& Name)
{
   return mAnimations.get(Name);
}

const ObjectRegistry* AnimationSet::getObjectRegistry()
{
   return mAnimations.getObjectRegistry();
}
