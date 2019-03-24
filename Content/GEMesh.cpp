
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
const ObjectName Mesh::TypeName = ObjectName("Mesh");
const char* Mesh::SubDir = "Meshes";
const char* Mesh::Extension = "meshes";

Mesh::Mesh(const ObjectName& Name, const ObjectName& GroupName)
   : Resource(Name, GroupName, TypeName)
   , cSkinningData(0)
{
#if defined (GE_EDITOR_SUPPORT)
   GERegisterPropertyReadonly(UInt, VertexCount);
   GERegisterPropertyReadonly(UInt, TriangleCount);
#endif
}

Mesh::Mesh(const Primitive& P, const ObjectName& Name)
   : Resource(Name, ObjectName::Empty, TypeName)
   , cSkinningData(0)
{
   loadFromPrimivite(P);

#if defined (GE_EDITOR_SUPPORT)
   GERegisterPropertyReadonly(UInt, VertexCount);
   GERegisterPropertyReadonly(UInt, TriangleCount);
#endif
}

Mesh::~Mesh()
{
   if(sGeometryData.VertexData)
   {
      Allocator::free(sGeometryData.VertexData);
   }

   if(sGeometryData.Indices)
   {
      Allocator::free(sGeometryData.Indices);
   }

   if(cSkinningData)
   {
      Allocator::free(cSkinningData);
   }
}

void Mesh::loadFromFile(std::istream& pStream)
{
   // "GEMesh  "
   char buffer[64];
   pStream.read(buffer, 8);

   // number of vertices
   pStream.read(reinterpret_cast<char*>(&sGeometryData.NumVertices), sizeof(uint32_t));

   // number of indices
   pStream.read(reinterpret_cast<char*>(&sGeometryData.NumIndices), sizeof(uint32_t));

   // vertex stride
   pStream.read(reinterpret_cast<char*>(&sGeometryData.VertexStride), sizeof(uint32_t));

   // use diffuse texture?
   pStream.read(buffer, sizeof(uint32_t));

   // is skinned?
   uint32_t bSkinned;
   pStream.read(reinterpret_cast<char*>(&bSkinned), sizeof(uint32_t));

   // reserved
   pStream.read(buffer, FileFormatHeaderReservedBytes);

   // vertex data
   uint iNumValues = sGeometryData.NumVertices * sGeometryData.VertexStride / sizeof(float);
   sGeometryData.VertexData = Allocator::alloc<float>(iNumValues);
   float* pCurrentValue = sGeometryData.VertexData;

   for(uint i = 0; i < iNumValues; i++)
   {
      pStream.read(reinterpret_cast<char*>(pCurrentValue), sizeof(float));
      pCurrentValue++;
   }

   // indices
   if(sGeometryData.NumIndices > 0)
   {
      sGeometryData.Indices = Allocator::alloc<ushort>(sGeometryData.NumIndices);
      ushort* pCurrentIndex = sGeometryData.Indices;

      for(uint i = 0; i < sGeometryData.NumIndices; i++)
      {
         pStream.read(reinterpret_cast<char*>(pCurrentIndex), sizeof(uint16_t));
         pCurrentIndex++;
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
            pStream.read(reinterpret_cast<char*>(&cAttachment.BoneIndex), sizeof(uint32_t));
            pStream.read(reinterpret_cast<char*>(&cAttachment.Weight), sizeof(float));
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

uint32_t Mesh::getVertexCount() const
{
   return sGeometryData.NumVertices;
}

uint32_t Mesh::getTriangleCount() const
{
   return sGeometryData.NumIndices / 3u;
}

void Mesh::loadFromXml(const pugi::xml_node& pXmlNode)
{
   Serializable::loadFromXml(pXmlNode);

   char subdir[256];
   sprintf(subdir, "Meshes/%s", cGroupName.getString());

   ContentData content;
   Device::readContentFile(Content::ContentType::GenericBinaryData, subdir, cName.getString(), "mesh.ge", &content);

   ContentDataMemoryBuffer memoryBuffer(content);
   std::istream stream(&memoryBuffer);
   loadFromFile(stream);
}

void Mesh::loadFromStream(std::istream& pStream)
{
   Serializable::loadFromStream(pStream);

   char buffer[64];
   pStream.read(buffer, sizeof(uint32_t));

   loadFromFile(pStream);
}
