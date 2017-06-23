
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEMesh.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEMesh.h"
#include "Content/GEContentData.h"
#include "Core/GEGeometry.h"
#include "Core/GEConstants.h"
#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Core/GEParser.h"
#include "Rendering/GEPrimitives.h"
#include "pugixml/pugixml.hpp"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;
using namespace GE::Rendering;


//
//  Mesh
//
const ResourceType Mesh::Type = ResourceType::Mesh;

Mesh::Mesh(const char* FileName)
   : Resource(FileName, ObjectName::Empty, Type)
   , cSkinningData(0)
{
   loadFromFile(FileName);
}

Mesh::Mesh(const Primitive& P, const ObjectName& Name)
   : Resource(Name, ObjectName::Empty, Type)
   , cSkinningData(0)
{
   loadFromPrimivite(P);
}

Mesh::~Mesh()
{
   if(sGeometryData.VertexData)
      Allocator::free(sGeometryData.VertexData);

   if(sGeometryData.Indices)
      Allocator::free(sGeometryData.Indices);

   if(cSkinningData)
      Allocator::free(cSkinningData);
}

void Mesh::loadFromFile(const char* Filename)
{
   ContentData cContent;
   Device::readContentFile(Content::ContentType::GenericBinaryData, "Models", Filename, "mesh.ge", &cContent);

   char* pData = cContent.getData();

   // "GEMesh  "
   pData += 8;

   // number of vertices
   sGeometryData.NumVertices = *reinterpret_cast<uint*>(pData);
   pData += sizeof(uint);

   // number of indices
   sGeometryData.NumIndices = *reinterpret_cast<uint*>(pData);
   pData += sizeof(uint);

   // vertex stride
   sGeometryData.VertexStride = *reinterpret_cast<uint*>(pData);
   pData += sizeof(uint);

   // use diffuse texture?
   pData += sizeof(uint);

   // is skinned?
   bool bSkinned = *reinterpret_cast<uint*>(pData) ? true : false;
   pData += sizeof(uint);

   // reserved
   pData += FileFormatHeaderReservedBytes;

   // vertex data
   uint iNumValues = sGeometryData.NumVertices * sGeometryData.VertexStride / sizeof(float);
   sGeometryData.VertexData = Allocator::alloc<float>(iNumValues);
   float* pCurrentValue = sGeometryData.VertexData;

   for(uint i = 0; i < iNumValues; i++)
   {
      *pCurrentValue = *reinterpret_cast<float*>(pData);
      pCurrentValue++;
      pData += sizeof(float);
   }

   // indices
   if(sGeometryData.NumIndices > 0)
   {
      sGeometryData.Indices = Allocator::alloc<ushort>(sGeometryData.NumIndices);
      ushort* pCurrentIndex = sGeometryData.Indices;

      for(uint i = 0; i < sGeometryData.NumIndices; i++)
      {
         *pCurrentIndex = *reinterpret_cast<ushort*>(pData);
         pCurrentIndex++;
         pData += sizeof(ushort);
      }
   }

   // skinning data
   if(bSkinned)
   {
      // vertex-bone attachments
      cSkinningData = Allocator::alloc<VertexBoneAttachment>(sGeometryData.NumVertices * BoneAttachmentsPerVertex);

      for(uint iVertexIndex = 0; iVertexIndex < sGeometryData.NumVertices; iVertexIndex++)
      {
         for(uint iAttachmentIndex = 0; iAttachmentIndex < BoneAttachmentsPerVertex; iAttachmentIndex++)
         {
            VertexBoneAttachment& cAttachment = cSkinningData[(iVertexIndex * BoneAttachmentsPerVertex) + iAttachmentIndex];
            cAttachment.BoneIndex = *reinterpret_cast<uint*>(pData);
            pData += sizeof(uint);
            cAttachment.Weight = *reinterpret_cast<float*>(pData);
            pData += sizeof(float);
         }
      }
   }
}

void Mesh::loadFromPrimivite(const Primitive& P)
{
   loadFromArrays((uint)P.getNumVertices(), (float*)P.getVertices(), (float*)P.getNormals(), (float*)P.getTexCoords(),
      (uint)P.getNumIndices(), (ushort*)P.getIndices());
}

void Mesh::loadFromArrays(uint NumVertices, float* Vertex, float* Normals, float* TextureCoordinate,
                          uint NumIndices, ushort* Indices)
{
   sGeometryData.NumVertices = NumVertices;
   sGeometryData.VertexStride = (3 + 3 + 2) * sizeof(float);
   sGeometryData.VertexData = Allocator::alloc<float>(sGeometryData.NumVertices * sGeometryData.VertexStride);

   float* pCurrentVertex = sGeometryData.VertexData;
   float* pCurrentPosition = Vertex;
   float* pCurrentNormal = Normals;
   float* pCurrentTextureCoordinate = TextureCoordinate;

   for(uint i = 0; i < sGeometryData.NumVertices; i++)
   {
      memcpy(pCurrentVertex, pCurrentPosition, 3 * sizeof(float));
      pCurrentVertex += 3;
      pCurrentPosition += 3;

      memcpy(pCurrentVertex, pCurrentNormal, 3 * sizeof(float));
      pCurrentVertex += 3;
      pCurrentNormal += 3;

      memcpy(pCurrentVertex, pCurrentTextureCoordinate, 2 * sizeof(float));
      pCurrentVertex += 2;
      pCurrentTextureCoordinate += 2;
   }

   sGeometryData.NumIndices = NumIndices;
   sGeometryData.Indices = Allocator::alloc<ushort>(NumIndices);
   memcpy(sGeometryData.Indices, Indices, NumIndices * sizeof(ushort));
}

const GeometryData& Mesh::getGeometryData() const
{
   return sGeometryData;
}

bool Mesh::isSkinned() const
{
   return cSkinningData != 0;
}

VertexBoneAttachment* Mesh::getVertexBoneAttachment(uint Index)
{
   GEAssert(Index < (sGeometryData.NumVertices * BoneAttachmentsPerVertex));
   return &cSkinningData[Index];
}
