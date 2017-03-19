
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
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


   enum VertexElement
   {
      VE_Position = 1 << 0,
      VE_Color    = 1 << 1,
      VE_Normal   = 1 << 2,
      VE_TexCoord = 1 << 3,
      VE_WVP      = 1 << 4,
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
      MaterialPass* RenderMaterialPass;
      Entities::ComponentRenderable* Renderable;
      Content::GeometryData Data;
      int Group;

      RenderOperation()
         : RenderMaterialPass(0)
         , Renderable(0)
         , Group(0)
      {
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
