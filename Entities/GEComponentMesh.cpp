
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Entities
//
//  --- GEComponentMesh.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentMesh.h"
#include "GEEntity.h"
#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Core/GEGeometry.h"
#include "Core/GELog.h"
#include "Content/GEResourcesManager.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;


//
//  ComponentMesh
//
const ObjectName ComponentMesh::ClassName = ObjectName("Mesh");

ComponentMesh::ComponentMesh(Entity* Owner)
   : ComponentRenderable(Owner)
   , cMesh(0)
   , eDynamicShadows(0)
   , eSettings(0)
{
   mClassNames.push_back(ClassName);

   eRenderingMode = RenderingMode::_3D;

   GERegisterProperty(ObjectName, MeshName);
   GERegisterPropertyBitMask(DynamicShadowsBitMask, DynamicShadows);
   GERegisterPropertyBitMask(MeshSettingsBitMask, Settings);
}

ComponentMesh::~ComponentMesh()
{
   unload();
}

const Core::ObjectName& ComponentMesh::getMeshName() const
{
   return cMesh ? cMesh->getName() : ObjectName::Empty;
}

void ComponentMesh::setMeshName(const ObjectName& MeshName)
{
   cMesh = 0;

   if(MeshName.isEmpty())
   {
      return;
   }

   Mesh* mesh = SerializableResourcesManager::getInstance()->get<Mesh>(MeshName);

   if(mesh)
   {
      loadMesh(mesh);
   }
   else
   {
      Log::log(LogType::Warning, "The '%s' mesh has not been loaded", MeshName.getString());
   }
}

void ComponentMesh::loadMesh(Mesh* M)
{
   GEAssert(M);
   unload();

   cMesh = M;
   sGeometryData = cMesh->getGeometryData();

   // skinning data
   if(cMesh->isSkinned())
   {
      eGeometryType = GeometryType::Dynamic;
      GESetFlag(eSettings, MeshSettingsBitMask::Skinning);

      uint iVertexDataFloats = cMesh->getGeometryData().NumVertices * cMesh->getGeometryData().VertexStride / sizeof(float);
      sGeometryData.VertexData = Allocator::alloc<float>(iVertexDataFloats);
      memcpy(sGeometryData.VertexData, cMesh->getGeometryData().VertexData, iVertexDataFloats * sizeof(float));
   }
}

void ComponentMesh::unload()
{
   if(cMesh && cMesh->isSkinned() && sGeometryData.VertexData)
   {
      Allocator::free(sGeometryData.VertexData);
      sGeometryData.VertexData = 0;
   }

   cMesh = 0;
   sGeometryData = GeometryData();
}

Content::Mesh* ComponentMesh::getMesh() const
{
   return cMesh;
}

void ComponentMesh::updateSkinning()
{
   ComponentSkeleton* cSkeleton = cOwner->getComponent<ComponentSkeleton>();

   if(!cSkeleton && cOwner->getParent())
   {
      cSkeleton = cOwner->getParent()->getComponent<ComponentSkeleton>();
   }

   if(!cSkeleton)
      return;

   float* fOriginalVertexDataPtr = cMesh->getGeometryData().VertexData;
   float* fVertexDataPtr = sGeometryData.VertexData;
   uint iFloatsPerVertex = sGeometryData.VertexStride / sizeof(float);

   const Matrix4* sBoneMatrices = cSkeleton->getBoneMatrices();
   VertexBoneAttachment* cAttachment = cMesh->getVertexBoneAttachment(0);

   for(uint iVertexIndex = 0; iVertexIndex < sGeometryData.NumVertices; iVertexIndex++)
   {
      Matrix4 mVertexTransform;
      memset(&mVertexTransform.m[0], 0, sizeof(Matrix4));

      for(uint iAttachmentIndex = 0; iAttachmentIndex < Mesh::BoneAttachmentsPerVertex; iAttachmentIndex++, cAttachment++)
      {
         if(cAttachment->Weight < GE_EPSILON)
            continue;

         mVertexTransform.m[ 0] += sBoneMatrices[cAttachment->BoneIndex].m[ 0] * cAttachment->Weight;
         mVertexTransform.m[ 1] += sBoneMatrices[cAttachment->BoneIndex].m[ 1] * cAttachment->Weight;
         mVertexTransform.m[ 2] += sBoneMatrices[cAttachment->BoneIndex].m[ 2] * cAttachment->Weight;
         mVertexTransform.m[ 3] += sBoneMatrices[cAttachment->BoneIndex].m[ 3] * cAttachment->Weight;
         mVertexTransform.m[ 4] += sBoneMatrices[cAttachment->BoneIndex].m[ 4] * cAttachment->Weight;
         mVertexTransform.m[ 5] += sBoneMatrices[cAttachment->BoneIndex].m[ 5] * cAttachment->Weight;
         mVertexTransform.m[ 6] += sBoneMatrices[cAttachment->BoneIndex].m[ 6] * cAttachment->Weight;
         mVertexTransform.m[ 7] += sBoneMatrices[cAttachment->BoneIndex].m[ 7] * cAttachment->Weight;
         mVertexTransform.m[ 8] += sBoneMatrices[cAttachment->BoneIndex].m[ 8] * cAttachment->Weight;
         mVertexTransform.m[ 9] += sBoneMatrices[cAttachment->BoneIndex].m[ 9] * cAttachment->Weight;
         mVertexTransform.m[10] += sBoneMatrices[cAttachment->BoneIndex].m[10] * cAttachment->Weight;
         mVertexTransform.m[11] += sBoneMatrices[cAttachment->BoneIndex].m[11] * cAttachment->Weight;
         mVertexTransform.m[12] += sBoneMatrices[cAttachment->BoneIndex].m[12] * cAttachment->Weight;
         mVertexTransform.m[13] += sBoneMatrices[cAttachment->BoneIndex].m[13] * cAttachment->Weight;
         mVertexTransform.m[14] += sBoneMatrices[cAttachment->BoneIndex].m[14] * cAttachment->Weight;
         mVertexTransform.m[15] += sBoneMatrices[cAttachment->BoneIndex].m[15] * cAttachment->Weight;
      }

      Vector3 vVertexPosition = Vector3(fOriginalVertexDataPtr[0], fOriginalVertexDataPtr[1], fOriginalVertexDataPtr[2]);
      Vector3 vVertexNormal = Vector3(fOriginalVertexDataPtr[3], fOriginalVertexDataPtr[4], fOriginalVertexDataPtr[5]);

      Matrix4Transform(mVertexTransform, &vVertexPosition);

      fVertexDataPtr[0] = vVertexPosition.X; fVertexDataPtr[1] = vVertexPosition.Y; fVertexDataPtr[2] = vVertexPosition.Z;
      fVertexDataPtr[3] = vVertexNormal.X; fVertexDataPtr[4] = vVertexNormal.Y; fVertexDataPtr[5] = vVertexNormal.Z;

      fVertexDataPtr += iFloatsPerVertex;
      fOriginalVertexDataPtr += iFloatsPerVertex;
   }
}
