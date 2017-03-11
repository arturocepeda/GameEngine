
#pragma once

#include "fbxsdk.h"
#include "Types/GETypes.h"
#include "Content/GEMesh.h"
#include "Content/GESkeleton.h"
#include "Content/GEAnimation.h"
#include "pugixml/pugixml.hpp"

#include <vector>

#if !defined (FBXSDK_NAMESPACE)
# define FBXSDK_NAMESPACE fbxsdk_2017_1
#endif

struct SubMesh
{
   char MaterialName[64];
   std::vector<float> VertexData;
   std::vector<std::vector<GE::Content::VertexBoneAttachment>> SkinningData;
   std::vector<GE::ushort> Indices;
   GE::ushort CurrentIndexOffset;

   SubMesh()
      : CurrentIndexOffset(0)
   {
   }
};

class ImportBone : public GE::Content::Bone
{
public:
   GE::Core::ObjectName ParentName;
   std::vector<GE::Core::ObjectName> ChildrenNames;

   FbxNode* fbxNode;
   FBXSDK_NAMESPACE::FbxAMatrix fbxLinkTransform;
   FBXSDK_NAMESPACE::FbxAMatrix fbxInverseLinkTransform;
   bool LinkTransformMatrixSet;
   bool AnimationSet;

   ImportBone(GE::uint Index, const GE::Core::ObjectName& Name)
      : GE::Content::Bone(Index, Name)
      , fbxNode(0)
      , LinkTransformMatrixSet(false)
      , AnimationSet(false)
   {
   }
};

void importMeshData(FbxNode* fbxNode);
void importSkeletonData(FbxNode* fbxNode);
void importAnimations(FbxScene* fbxScene, FbxNode* fbxNode);
void importBones(FbxNode* fbxNode);
void importMaterials(FbxNode* fbxNode);
void importGeometry(FbxNode* fbxNode, const char* sMeshName, int iMaterialIndex);

GE::Vector3 getTranslation(const FbxVector4& fbxVector);
GE::Vector3 getRotation(const FbxVector4& fbxVector);
GE::Vector3 getScale(const FbxVector4& fbxVector);

void generateMeshFile(const SubMesh& sSubMesh, const char* sName);
void generateAnimationFile(GE::Content::Animation* cAnimation);
