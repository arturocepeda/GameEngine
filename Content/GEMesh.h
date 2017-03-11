
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
#include "GEManagedContent.h"
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


   class Mesh : public ManagedContent
   {
   private:
      GeometryData sGeometryData;
      VertexBoneAttachment* cSkinningData;

      void loadFromFile(const char* Filename);
      void loadFromArrays(uint NumVertices, float* Vertex, float* Normals, float* TextureCoordinate,
         uint NumIndices, ushort* Indices);
      void loadFromPrimivite(const Rendering::Primitive& P);

   public:
      static const ManagedContentType ContentType;

      static const uint BoneAttachmentsPerVertex = 4;
      static const uint FileFormatHeaderReservedBytes = 36;

      Mesh(const char* FileName);
      Mesh(const Rendering::Primitive& P, const Core::ObjectName& Name);
      ~Mesh();

      const GeometryData& getGeometryData() const;
      bool isSkinned() const;
      VertexBoneAttachment* getVertexBoneAttachment(uint Index);
   };
}}
