
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GESkeleton.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GESkeleton.h"
#include "Types/GETypes.h"
#include "Core/GEConstants.h"
#include "Core/GEAllocator.h"
#include "Core/GEParser.h"
#include "Core/GEGeometry.h"
#include "Core/GEApplication.h"
#include "Core/GEValue.h"
#include "Core/GEDevice.h"
#include "Content/GEContentData.h"
#include "Externals/pugixml/pugixml.hpp"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;


//
//  Bone
//
Bone::Bone(uint Index, const Core::ObjectName& Name)
   : Object(Name)
   , iIndex(Index)
   , iParentIndex(0)
   , fSize(0.0f)
{
   Matrix4MakeIdentity(&mBindMatrix);
   Matrix4MakeIdentity(&mInverseBindMatrix);
}

Bone::~Bone()
{
}

uint Bone::getIndex() const
{
   return iIndex;
}

uint Bone::getParentIndex() const
{
   return iParentIndex;
}

void Bone::setParentIndex(uint ParentIndex)
{
   iParentIndex = ParentIndex;
}

uint Bone::getChildrenCount() const
{
   return (uint)vChildren.size();
}

uint Bone::getChild(uint Index) const
{
   GEAssert(Index < vChildren.size());
   return vChildren[Index];
}

void Bone::addChild(uint ChildIndex)
{
   vChildren.push_back(ChildIndex);
}

const Matrix4& Bone::getBindMatrix() const
{
   return mBindMatrix;
}

const Matrix4& Bone::getInverseBindMatrix() const
{
   return mInverseBindMatrix;
}

void Bone::setBindMatrix(const Matrix4& BindMatrix)
{
   mBindMatrix = BindMatrix;
   mInverseBindMatrix = BindMatrix;
   Matrix4Invert(&mInverseBindMatrix);
}

float Bone::getSize() const
{
   return fSize;
}

void Bone::setSize(float Size)
{
   fSize = Size;
}


//
//  Skeleton
//
const ObjectName Skeleton::TypeName = ObjectName("Skeleton");

Skeleton::Skeleton(const char* FileName)
   : Resource(FileName, ObjectName::Empty, TypeName)
{
   ContentData cSkeletonData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(Content::ContentType::GenericTextData, "Meshes", FileName, "skeleton.xml", &cSkeletonData);

      pugi::xml_document xml;
      xml.load_buffer(cSkeletonData.getData(), cSkeletonData.getDataSize());
      const pugi::xml_node& xmlSkeleton = xml.child("Skeleton");
      uint iBonesCount = Parser::parseInt(xmlSkeleton.attribute("bonesCount").value());

      uint iBoneIndex = 0;

      for(pugi::xml_node_iterator it = xmlSkeleton.begin(); it != xmlSkeleton.end(); it++)
      {
         const pugi::xml_node& xmlBone = *it;
         Bone* cCurrentBone = Allocator::alloc<Bone>();
         GEInvokeCtor(Bone, cCurrentBone)(iBoneIndex, ObjectName(xmlBone.attribute("name").value()));

         pugi::xml_attribute xmlBoneParentIndex = xmlBone.attribute("parentIndex");

         if(!xmlBoneParentIndex.empty())
            cCurrentBone->setParentIndex(Parser::parseInt(xmlBoneParentIndex.value()));

         Vector3 vDefaultPosePosition = Parser::parseVector3(xmlBone.attribute("bindT").value());
         Vector3 vRotationEulerInDegrees = Parser::parseVector3(xmlBone.attribute("bindR").value());
         Vector3 vDefaultPoseScale = Parser::parseVector3(xmlBone.attribute("bindS").value());
         cCurrentBone->setSize(Parser::parseFloat(xmlBone.attribute("size").value()));

         Matrix4 mBindMatrix;
         Rotation cDefaultPoseRotation = Rotation(vRotationEulerInDegrees * GE_DEG2RAD);
         Geometry::createTRSMatrix(vDefaultPosePosition, cDefaultPoseRotation, vDefaultPoseScale, &mBindMatrix);
         cCurrentBone->setBindMatrix(mBindMatrix);

         const pugi::xml_node& xmlBoneChildren = xmlBone.child("Children");

         for(pugi::xml_node_iterator it2 = xmlBoneChildren.begin(); it2 != xmlBoneChildren.end(); it2++)
         {
            const pugi::xml_node& xmlBoneChild = *it2;
            cCurrentBone->addChild(Parser::parseInt(xmlBoneChild.attribute("index").value()));
         }

         vBones.push_back(cCurrentBone);
      }

      GEAssert(vBones.size() == iBonesCount);
   }
   else
   {
      Device::readContentFile(Content::ContentType::GenericBinaryData, "Meshes", FileName, "skeleton.ge", &cSkeletonData);
      ContentDataMemoryBuffer sMemoryBuffer(cSkeletonData);
      std::istream sStream(&sMemoryBuffer);

      uint iBonesCout = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint iBoneIndex = 0; iBoneIndex < iBonesCout; iBoneIndex++)
      {
         ObjectName cBoneName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();

         Bone* cCurrentBone = Allocator::alloc<Bone>();
         GEInvokeCtor(Bone, cCurrentBone)(iBoneIndex, cBoneName);
         cCurrentBone->setParentIndex((uint)Value::fromStream(ValueType::Byte, sStream).getAsByte());

         Vector3 vDefaultPosePosition = Value::fromStream(ValueType::Vector3, sStream).getAsVector3();
         Vector3 vRotationEulerInDegrees = Value::fromStream(ValueType::Vector3, sStream).getAsVector3();
         Vector3 vDefaultPoseScale = Value::fromStream(ValueType::Vector3, sStream).getAsVector3();
         cCurrentBone->setSize(Value::fromStream(ValueType::Float, sStream).getAsFloat());

         Matrix4 mBindMatrix;
         Rotation cDefaultPoseRotation = Rotation(vRotationEulerInDegrees * GE_DEG2RAD);
         Geometry::createTRSMatrix(vDefaultPosePosition, cDefaultPoseRotation, vDefaultPoseScale, &mBindMatrix);
         cCurrentBone->setBindMatrix(mBindMatrix);

         uint iBoneChildrenCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

         for(uint i = 0; i < iBoneChildrenCount; i++)
            cCurrentBone->addChild((uint)Value::fromStream(ValueType::Byte, sStream).getAsByte());

         vBones.push_back(cCurrentBone);
      }
   }
}

Skeleton::~Skeleton()
{
   uint iBonesCount = (uint)vBones.size();

   for(uint i = 0; i < iBonesCount; i++)
      Allocator::free(vBones[i]);
}

uint Skeleton::getBonesCount() const
{
   return (uint)vBones.size();
}

const Bone* Skeleton::getBone(uint Index) const
{
   GEAssert(Index < vBones.size());
   return vBones[Index];
}