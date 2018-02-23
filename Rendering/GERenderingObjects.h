
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Rendering
//
//  --- GERenderingObjects.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Types/GESTLTypes.h"
#include "Rendering/GEGraphicsDevice.h"
#include "Content/GEGeometryData.h"
#include "Core/GESerializable.h"


namespace GE { namespace Entities
{
   class ComponentRenderable;
   class ComponentMesh;
}}


namespace GE { namespace Content
{
   class ImageData;
}}


namespace GE { namespace Rendering
{
   class Texture;
   class MaterialPass;


   GESerializableEnum(RenderingMode)
   {
      _2D,
      _3D,

      Count
   };


   GESerializableEnum(GeometryType)
   {
      Static,
      Dynamic,

      Count
   };


   class GeometryGroup
   {
   public:
      enum
      {
         _2DStatic,
         _2DDynamic,
         _2DBatch,
         MeshStatic,
         MeshDynamic,
         MeshBatch,
         Particles,

         Count
      };
   };


   struct RenderOperation
   {
      uint Index;
      uint SubIndex;
      MaterialPass* RenderMaterialPass;
      Entities::ComponentRenderable* Renderable;
      Content::GeometryData Data;
      int Group;

      RenderOperation()
         : Index(0)
         , SubIndex(0)
         , RenderMaterialPass(0)
         , Renderable(0)
         , Group(0)
      {
      }

      bool operator<(const RenderOperation& Other) const
      {
         return Other.Index == Index
            ? Other.SubIndex < SubIndex
            : Other.Index < Index;
      }
   };


   struct GeometryRenderInfo
   {
      uint VertexBufferOffset;
      uint IndexBufferOffset;

      GeometryRenderInfo()
         : VertexBufferOffset(0)
         , IndexBufferOffset(0)
      {
      }

      GeometryRenderInfo(uint VertexOffset, uint IndexOffset)
         : VertexBufferOffset(VertexOffset)
         , IndexBufferOffset(IndexOffset)
      {      
      }
   };


   GESerializableEnum(BlendingMode)
   {
      None,
      Alpha,
      Additive,

      Count
   };


   GESerializableEnum(DepthBufferMode)
   {
      NoDepth,
      TestOnly,
      TestAndWrite,

      Count
   };


   GESerializableEnum(CullingMode)
   {
      Back,
      Front,
      None,

      Count
   };


   struct PreloadedTexture
   {
      Texture* Tex;
      Content::ImageData* Data;

      PreloadedTexture()
         : Tex(0)
         , Data(0)
      {
      }
   };


   GESerializableEnum(DynamicShadowsBitMask)
   {
      Cast     = 1 << 0,
      Receive  = 1 << 1,

      Count = 2
   };
}}
