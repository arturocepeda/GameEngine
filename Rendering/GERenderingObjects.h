
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


   class GeometryGroup
   {
   public:
      enum
      {
         SpriteStatic,
         SpriteDynamic,
         SpriteBatch,
         Label,
         LabelBatch,
         MeshStatic,
         MeshDynamic,
         MeshBatch,
         Particles,

         Count
      };
   };


   enum class RenderOperationFlags
   {
      RenderThroughActiveCamera  = 1 << 0,
      LightingSupport            = 1 << 1,
      BindShadowMap              = 1 << 2
   };


   struct RenderOperation
   {
      uint32_t mIndex;
      uint32_t mFlags;
      uint32_t mGeometryID;
      uint16_t mGroup;
      uint16_t mVertexIndexSize;

      MaterialPass* mRenderMaterialPass;
      Texture* mDiffuseTexture;
      Content::GeometryData* mData;
      Matrix4 mWorldTransform;
      Color mColor;

      RenderOperation()
         : mIndex(0u)
         , mFlags(0u)
         , mGeometryID(0u)
         , mGroup(0u)
         , mVertexIndexSize(2u)
         , mRenderMaterialPass(nullptr)
         , mDiffuseTexture(nullptr)
         , mData(nullptr)
      {
         Matrix4MakeIdentity(&mWorldTransform);
      }

      bool operator<(const RenderOperation& pOther) const
      {
         return pOther.mIndex < mIndex;
      }
      bool isStatic() const
      {
         return
            mGroup == GeometryGroup::SpriteStatic ||
            mGroup == GeometryGroup::MeshStatic;
      }
   };


   struct GeometryRenderInfo
   {
      uint32_t mVertexBufferOffset;
      uint32_t mIndexBufferOffset;

      GeometryRenderInfo()
         : mVertexBufferOffset(0u)
         , mIndexBufferOffset(0u)
      {
      }

      GeometryRenderInfo(uint32_t pVertexOffset, uint32_t pIndexOffset)
         : mVertexBufferOffset(pVertexOffset)
         , mIndexBufferOffset(pIndexOffset)
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
