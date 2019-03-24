
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEMesh.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEGeometryData.h"
#include "GEResource.h"
#include "Core/GEObject.h"
#include "Rendering/GEPrimitives.h"

namespace GE { namespace Content
{
   struct VertexBoneAttachment
   {
      uint BoneIndex;
      float Weight;

      VertexBoneAttachment()
         : BoneIndex(0)
         , Weight(0.0f)
      {
      }
   };


   class Mesh : public Resource
   {
   private:
      GeometryData sGeometryData;
      VertexBoneAttachment* cSkinningData;

      void loadFromFile(std::istream& pStream);
      void loadFromArrays(uint NumVertices, float* Vertex, float* Normals, float* TextureCoordinate,
         uint NumIndices, ushort* Indices);
      void loadFromPrimivite(const Rendering::Primitive& P);

   public:
      static const Core::ObjectName TypeName;
      static const char* SubDir;
      static const char* Extension;

      static const uint BoneAttachmentsPerVertex = 8;
      static const uint FileFormatHeaderReservedBytes = 36;

      Mesh(const Core::ObjectName& Name, const Core::ObjectName& GroupName);
      Mesh(const Rendering::Primitive& P, const Core::ObjectName& Name);
      ~Mesh();

      const GeometryData& getGeometryData() const;
      bool isSkinned() const;
      VertexBoneAttachment* getVertexBoneAttachment(uint Index);

      uint32_t getVertexCount() const;
      uint32_t getTriangleCount() const;

      virtual void loadFromXml(const pugi::xml_node& pXmlNode) override;
      virtual void loadFromStream(std::istream& pStream) override;
   };
}}
