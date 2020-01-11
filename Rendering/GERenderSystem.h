
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GERenderSystem.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GESingleton.h"
#include "Core/GEObjectManager.h"
#include "Core/GEThreads.h"
#include "Core/GEAllocator.h"

#include "Rendering/GEGraphicsDevice.h"
#include "Rendering/GERenderingObjects.h"
#include "Rendering/GEShaderProgram.h"
#include "Rendering/GEFont.h"

#include "Entities/GEComponentCamera.h"
#include "Entities/GEComponentLight.h"
#include "Entities/GEComponentMesh.h"
#include "Entities/GEComponentSprite.h"
#include "Entities/GEComponentLabel.h"
#include "Entities/GEComponentUIElement.h"

#include <vector>
#include <set>
#include <map>

namespace GE { namespace Rendering
{
   struct GPUBufferPair
   {
      void* VertexBuffer;
      void* IndexBuffer;
      uint VertexStride;
      uint CurrentVertexBufferOffset;
      uint CurrentIndexBufferOffset;
      uint IsDynamic;

      GPUBufferPair()
         : VertexBuffer(nullptr)
         , IndexBuffer(nullptr)
         , VertexStride(0u)
         , CurrentVertexBufferOffset(0u)
         , CurrentIndexBufferOffset(0u)
         , IsDynamic(1u)
      {
      }

      void clear()
      {
         CurrentVertexBufferOffset = 0u;
         CurrentIndexBufferOffset = 0u;
      }
   };


   struct _3DUICanvasEntry
   {
      uint16_t Index;
      uint16_t Settings;
      Vector3 WorldPosition;
   };


   class RenderSystem : public Core::Singleton<RenderSystem>
   {
   protected:
      static const uint VertexBufferSize = 1024 * 1024 * 16;
      static const uint IndexBufferSize = 1024 * 1024 * 8;
      static const uint ShadowMapSize = 1024;

      static const Core::ObjectName ShadowMapSolidProgram;
      static const Core::ObjectName ShadowMapAlphaProgram;

      enum class TextureSlot
      {
         Diffuse,
         ShadowMap,

         Count
      };

      void* pDevice;
      void* pWindow;
      bool bWindowed;

      uint iScreenWidth;
      uint iScreenHeight;

      GraphicsDevice* cDevice;
      Color cBackgroundColor;
   
      Matrix4 matModelView;
      Matrix4 matModelViewProjection;
      Matrix4 matModelInverseTranspose;
      Matrix4 matLightViewProjection;
      Matrix4 mat2DViewProjection;

      GEMutex mTextureLoadMutex;
      GESTLVector(PreloadedTexture) vPreloadedTextures;

      Core::ObjectManager<Texture> mTextures;
      Core::ObjectManager<Font> mFonts;
      Core::ObjectManager<Material> mMaterials;
      Core::ObjectManager<ShaderProgram> mShaderPrograms;

      uint iActiveProgram;
      uint iCurrentVertexStride;
      Texture* pBoundTexture[(uint)TextureSlot::Count];
      BlendingMode eBlendingMode;
      DepthBufferMode eDepthBufferMode;
      CullingMode eCullingMode;
      Entities::ComponentCamera* cActiveCamera;

      GESTLPriorityQueue(RenderOperation) vUIElementsToRender;
      GESTLPriorityQueue(RenderOperation) vPre3DSpritesToRender;
      GESTLPriorityQueue(RenderOperation) vPostUISpritesToRender;
      GESTLPriorityQueue(RenderOperation) v3DLabelsToRender;
      GESTLVector(RenderOperation) vShadowedMeshesToRender;
      GESTLVector(RenderOperation) vShadowedParticlesToRender;
      GESTLPriorityQueue(RenderOperation) vOpaqueMeshesToRender;
      GESTLVector(RenderOperation) vTransparentMeshesToRender;
      GESTLPriorityQueue(RenderOperation) vDebugGeometryToRender;

      GESTLPriorityQueue(RenderOperation) v3DUIElementsToRender[Entities::ComponentUI3DElement::CanvasCount];
      _3DUICanvasEntry s3DUICanvasEntries[Entities::ComponentUI3DElement::CanvasCount];
      bool mAny3DUIElementsToRender;

      GESTLVector(Entities::ComponentLight*) vLightsToRender;

      GPUBufferPair sGPUBufferPairs[GeometryGroup::Count];

      GESTLMap(uint, GeometryRenderInfo) mStaticGeometryToRender;
      GESTLMap(uint, GeometryRenderInfo) mDynamicGeometryToRender;

      GESTLMap(uint, RenderOperation) mBatches;
    
      Color cAmbientLightColor;
      bool bClearGeometryRenderInfoEntriesPending;
      bool bShaderReloadPending;

      float fFrameTime;
      float fFramesPerSecond;
      uint iDrawCalls;

      void loadDefaultRenderingResources();
      void loadShaders();

      void useMaterial(Material* cMaterial);
      void useShaderProgram(const Core::ObjectName& cName);
      void bindBuffers(const GPUBufferPair& sBufferPair);
      void bindTexture(TextureSlot eSlot, const Texture* cTexture);

      void calculate2DViewProjectionMatrix();

      void calculate2DTransformMatrix(const Matrix4& matModel);
      void calculate3DTransformMatrix(const Matrix4& matModel);
      void calculate3DInverseTransposeMatrix(const Matrix4& matModel);

      void calculateLightViewProjectionMatrix(Entities::ComponentLight* Light);

      void loadMaterial(Material* cMaterial);
      void unloadMaterial(const Core::ObjectName& cMaterialName);

      void loadRenderingData(const Content::GeometryData* pData, GPUBufferPair& pBuffers, uint32_t pIndexSize = 2u);

      void queueForRenderingSingle(Entities::ComponentRenderable* pRenderable, RenderOperation& sRenderOperation);
      void queueForRendering3DUI(Entities::ComponentRenderable* pRenderable, RenderOperation& pRenderOperation);
      void queueForRenderingBatch(Entities::ComponentRenderable* pRenderable, RenderOperation& sBatch);

      void prepareBatchForRendering(const RenderOperation& sBatch);

      void render(const RenderOperation& sRenderOperation);
      void renderShadowMap();

   public:
      std::function<bool(const _3DUICanvasEntry*, const _3DUICanvasEntry*)> _3DUICanvasSortFunction;

      RenderSystem(void* Window, bool Windowed, uint ScreenWidth, uint ScreenHeight);
      ~RenderSystem();

      // content
      void registerObjectManagers();

      // info
      const void* getWindowHandler() const;
      bool getWindowedMode() const;
      float getFPS() const;
      uint getDrawCalls() const;

      // background
      void setBackgroundColor(const Color& Color);
    
      // textures
      void preloadTextures(const char* FileName);
      void unloadTextures(const char* FileName);

      bool loadNextPreloadedTexture();
      void loadAllPreloadedTextures();
      bool preloadedTexturesPending();

      void loadTexture(PreloadedTexture* pPreloadedTexture);
      void unloadTexture(Texture* pTexture);

      Texture* getTexture(const Core::ObjectName& Name);

      // cameras
      Entities::ComponentCamera* getActiveCamera();
      void setActiveCamera(Entities::ComponentCamera* Camera);

      // lighting
      const Color& getAmbientLightColor() const;
      void setAmbientLightColor(const Color& Color);

      // materials
      void loadMaterials(const char* FileName);
      void unloadMaterials(const char* FileName);
      Material* getMaterial(const Core::ObjectName& Name);

      // shader programs
      ShaderProgram* getShaderProgram(const Core::ObjectName& Name);
      void requestShadersReload();

      // fonts
      void loadFonts(const char* FileName);
      void unloadFonts(const char* FileName);
      Font* getFont(const Core::ObjectName& Name);

      // blend mode
      void setBlendingMode(BlendingMode Mode);

      // depth buffer mode
      void setDepthBufferMode(DepthBufferMode Mode);

      // culling mode
      void setCullingMode(CullingMode Mode);

      // components to render
      void setup3DUICanvas(uint32_t pCanvasIndex, const Vector3& pWorldPosition, uint16_t pSettings);
      void queueForRendering(Entities::ComponentRenderable* Renderable, uint RequestIndex);
      void queueForRendering(Entities::ComponentLight* Light);
      void clearRenderingQueues();
      void clearGeometryRenderInfoEntries();

      // rendering
      void renderBegin();
      void renderFrame();
      void renderEnd();
   };
}}
