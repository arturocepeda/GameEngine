
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

      GPUBufferPair()
         : VertexBuffer(0)
         , IndexBuffer(0)
         , VertexStride(0)
         , CurrentVertexBufferOffset(0)
         , CurrentIndexBufferOffset(0)
      {
      }

      void clear()
      {
         CurrentVertexBufferOffset = 0;
         CurrentIndexBufferOffset = 0;
      }
   };


   class RenderSystem : public Core::Singleton<RenderSystem>
   {
   protected:
      static const GE::uint VertexBufferSize = 1024 * 1024 * 16;
      static const GE::uint IndexBufferSize = 1024 * 1024 * 8;
      static const GE::uint ShadowMapSize = 1024;

      static const Core::ObjectName ShadowMapProgram;

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
      Texture* pBoundTexture;
      BlendingMode eBlendingMode;
      DepthBufferMode eDepthBufferMode;
      Entities::ComponentCamera* cActiveCamera;

      GESTLVector(RenderOperation) vGUISpritesToRender;
      GESTLVector(RenderOperation) vPre3DSpritesToRender;
      GESTLVector(RenderOperation) vUILabelsToRender;
      GESTLVector(RenderOperation) v3DLabelsToRender;
      GESTLVector(RenderOperation) vShadowedMeshesToRender;
      GESTLVector(RenderOperation) vOpaqueMeshesToRender;
      GESTLVector(RenderOperation) vTransparentMeshesToRender;
      GESTLVector(Entities::ComponentLight*) vLightsToRender;

      GPUBufferPair sGPUBufferPairs[GeometryGroup::Count];

      GESTLMap(uint, GeometryRenderInfo) mStaticGeometryToRender;
      GESTLMap(uint, GeometryRenderInfo) mDynamicGeometryToRender;

      GESTLMap(uint, RenderOperation) mBatches;
    
      Color cAmbientLightColor;
      bool bShaderReloadPending;

      float fFrameTime;
      float fFramesPerSecond;
      uint iDrawCalls;

      void loadDefaultRenderingResources();
      void loadShaders();

      void useMaterial(Material* cMaterial);
      void useShaderProgram(const Core::ObjectName& cName);
      void bindBuffers(const GPUBufferPair& sBufferPair);
      void bindTexture(const Texture* cTexture);

      void calculate2DViewProjectionMatrix();

      void calculate2DTransformMatrix(const Matrix4& matModel);
      void calculate3DTransformMatrix(const Matrix4& matModel);
      void calculate3DInverseTransposeMatrix(const Matrix4& matModel);

      void calculateLightViewProjectionMatrix(Entities::ComponentLight* Light);

      void loadTexture(PreloadedTexture* cPreloadedTexture);
      void loadMaterial(Material* cMaterial);
      void unloadMaterial(const Core::ObjectName& cMaterialName);

      void loadRenderingData(const Content::GeometryData& sData, GPUBufferPair& sBuffers, uint iIndexSize = 2);

      void queueForRenderingSingle(RenderOperation& sRenderOperation);
      void queueForRenderingBatch(RenderOperation& sBatch);
      void prepareBatchForRendering(const RenderOperation& sBatch);

      void render(const RenderOperation& sRenderOperation);
      void renderShadowMap();

   public:
       RenderSystem(void* Window, bool Windowed, uint ScreenWidth, uint ScreenHeight);
       ~RenderSystem();

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

       // components to render
       void queueForRendering(Entities::ComponentRenderable* Renderable);
       void queueForRendering(Entities::ComponentLight* Light);
       void clearRenderingQueues();
       void clearGeometryRenderInfoEntries();

       // rendering
       void renderBegin();
       void renderFrame();
       void renderEnd();
   };
}}
