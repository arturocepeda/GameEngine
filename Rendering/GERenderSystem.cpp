
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GERenderSystem.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GERenderSystem.h"
#include "Core/GEDevice.h"
#include "Core/GETime.h"
#include "Core/GEParser.h"
#include "Core/GEAllocator.h"
#include "Core/GEApplication.h"
#include "Core/GEProfiler.h"
#include "Content/GEResourcesManager.h"
#include "Entities/GEEntity.h"
#include "Entities/GEComponentParticleSystem.h"
#include "Entities/GEComponentUIElement.h"
#include "pugixml/pugixml.hpp"

#include <stdio.h>
#include <algorithm>

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;

const uint RenderBatchVertexDataFloatsCount = 1024 * 1024;
const uint RenderBatchIndicesCount = 1024 * 256;

const ObjectName RenderSystem::ShadowMapSolidProgram = ObjectName("ShadowMapSolid");
const ObjectName RenderSystem::ShadowMapAlphaProgram = ObjectName("ShadowMapAlpha");

RenderSystem::RenderSystem(void* Window, bool Windowed, uint ScreenWidth, uint ScreenHeight)
   : pWindow(Window)
   , bWindowed(Windowed)
   , iScreenWidth(ScreenWidth)
   , iScreenHeight(ScreenHeight)
   , cBackgroundColor(Color(0.0f, 0.0f, 0.0f))
   , cAmbientLightColor(Color(1.0f, 1.0f, 1.0f))
   , bShaderReloadPending(false)
   , iActiveProgram(-1)
   , iCurrentVertexStride(0)
   , eBlendingMode(BlendingMode::None)
   , eDepthBufferMode(DepthBufferMode::NoDepth)
   , eCullingMode(CullingMode::Back)
   , cActiveCamera(0)
   , fFrameTime(Time::getElapsed())
   , fFramesPerSecond(0.0f)
   , iDrawCalls(0)
{
   memset(pBoundTexture, 0, sizeof(Texture*) * (GE::uint)TextureSlot::Count);

   ResourcesManager::getInstance()->registerObjectManager<Texture>("Texture", &mTextures);
   ResourcesManager::getInstance()->registerObjectManager<ShaderProgram>("ShaderProgram", &mShaderPrograms);
   ResourcesManager::getInstance()->registerObjectManager<Material>("Material", &mMaterials);
   ResourcesManager::getInstance()->registerObjectManager<Font>("Font", &mFonts);
   
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<ShaderProgram>("ShaderProgram", &mShaderPrograms);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<Material>("Material", &mMaterials);

   // Position (3) + UV (2)
   sGPUBufferPairs[GeometryGroup::_2DStatic].VertexStride = (3 + 2) * sizeof(float);
   sGPUBufferPairs[GeometryGroup::_2DDynamic].VertexStride = sGPUBufferPairs[GeometryGroup::_2DStatic].VertexStride;
   // Position (3) + Normal (3) + UV (2)
   sGPUBufferPairs[GeometryGroup::MeshStatic].VertexStride = (3 + 3 + 2) * sizeof(float);
   sGPUBufferPairs[GeometryGroup::MeshDynamic].VertexStride = sGPUBufferPairs[GeometryGroup::MeshStatic].VertexStride;
   // Position (3) + Color (4) + UV (2)
   sGPUBufferPairs[GeometryGroup::Particles].VertexStride = (3 + 4 + 2) * sizeof(float);

   // Batch buffers --> + WorldViewProjection (16)
   sGPUBufferPairs[GeometryGroup::_2DBatch].VertexStride = sGPUBufferPairs[GeometryGroup::_2DStatic].VertexStride + (16 * sizeof(float));
   sGPUBufferPairs[GeometryGroup::MeshBatch].VertexStride = sGPUBufferPairs[GeometryGroup::MeshStatic].VertexStride + (16 * sizeof(float));

   GEMutexInit(mTextureLoadMutex);
   calculate2DViewProjectionMatrix();

   clearRenderingQueues();
}

RenderSystem::~RenderSystem()
{
   mTextures.clear();
   mFonts.clear();
   mMaterials.clear();
   GEMutexDestroy(mTextureLoadMutex);
}

const void* RenderSystem::getWindowHandler() const
{
   return pWindow;
}

bool RenderSystem::getWindowedMode() const
{
   return bWindowed;
}

float RenderSystem::getFPS() const
{
   return fFramesPerSecond;
}

uint RenderSystem::getDrawCalls() const
{
   return iDrawCalls;
}

void RenderSystem::setBackgroundColor(const Color& Color)
{
   cBackgroundColor = Color;
}

void RenderSystem::loadDefaultRenderingResources()
{
   preloadTextures("default");
   loadAllPreloadedTextures();
   loadMaterials("default");
   loadFonts("default");
}

void RenderSystem::useMaterial(Material* cMaterial)
{
   setBlendingMode(cMaterial->getBlendingMode());
   useShaderProgram(cMaterial->getShaderProgram());
}

void RenderSystem::calculate2DViewProjectionMatrix()
{
   Matrix4 matProjection;
   Matrix4 matView;

#if defined GE_RENDERING_API_DIRECTX
   float fAspectRatio = Device::Orientation == DeviceOrientation::Portrait
      ? Device::getAspectRatio()
      : 1.0f / Device::getAspectRatio();

   Matrix4MakeOrtho(-1.0f, 1.0f, -fAspectRatio, fAspectRatio, 0.1f, 100.0f, &matProjection);
   Matrix4MakeLookAt(Vector3::UnitZ, Vector3::Zero, Vector3::UnitY, &matView);

   if(Device::Orientation == DeviceOrientation::Landscape)
   {
      Matrix4Scale(&matView, Vector3::One * fAspectRatio);
      Matrix4RotateZ(&matView, GE_HALFPI);
   }

   Matrix4Transpose(&matProjection);
   Matrix4Transpose(&matView);
#else
   Matrix4MakeOrtho(-1.0f, 1.0f, -Device::getAspectRatio(), Device::getAspectRatio(), 0.1f, 100.0f, &matProjection);
   Matrix4MakeLookAt(Vector3::UnitZ, Vector3::Zero, Vector3::UnitY, &matView);
#endif

   Matrix4Multiply(matProjection, matView, &mat2DViewProjection);
}

void RenderSystem::calculate2DTransformMatrix(const Matrix4& matModel)
{
   Matrix4Multiply(mat2DViewProjection, matModel, &matModelViewProjection);
}

void RenderSystem::calculate3DTransformMatrix(const Matrix4& matModel)
{
   GEAssert(cActiveCamera);
   const Matrix4& matViewProjection = cActiveCamera->getViewProjectionMatrix();
   Matrix4Multiply(matViewProjection, matModel, &matModelViewProjection);
}

void RenderSystem::calculate3DInverseTransposeMatrix(const Matrix4& matModel)
{
   matModelInverseTranspose = matModel;
   Matrix4Invert(&matModelInverseTranspose);
   Matrix4Transpose(&matModelInverseTranspose);
}

void RenderSystem::calculateLightViewProjectionMatrix(ComponentLight* Light)
{
   Scene* cActiveScene = Scene::getActiveScene();

   if(!cActiveScene || !cActiveCamera)
      return;   

   float fShadowsDistance = cActiveScene->getShadowsMaxDistance();

   Vector3 vLightDirection = Light->getDirection();
   Vector3 vLightPosition = -(vLightDirection * fShadowsDistance * 0.5f);
   Light->getTransform()->setPosition(vLightPosition);

   Matrix4 matLightView;
   Matrix4MakeLookAt(vLightPosition, vLightPosition + vLightDirection, Vector3::UnitY, &matLightView);

   Matrix4 matLightProjection;
   Matrix4MakeOrtho(-fShadowsDistance, fShadowsDistance, -fShadowsDistance, fShadowsDistance, -fShadowsDistance, fShadowsDistance, &matLightProjection);

   Matrix4Multiply(matLightProjection, matLightView, &matLightViewProjection);
}

void RenderSystem::loadMaterial(Material* cMaterial)
{
   mMaterials.add(cMaterial);

   if(GEHasFlag(cMaterial->getFlags(), MaterialFlagsBitMask::BatchRendering))
   {
      const uint iMaterialID = cMaterial->getName().getID();
      mBatches[iMaterialID] = RenderOperation();

      GESTLMap(uint, RenderOperation)::iterator it = mBatches.find(iMaterialID);
      RenderOperation& sRenderBatch = it->second;

      sRenderBatch.RenderMaterialPass = Allocator::alloc<MaterialPass>();
      GEInvokeCtor(MaterialPass, sRenderBatch.RenderMaterialPass)();
      sRenderBatch.RenderMaterialPass->setMaterial(cMaterial);

      sRenderBatch.Data.VertexData = Allocator::alloc<float>(RenderBatchVertexDataFloatsCount);
      sRenderBatch.Data.Indices = Allocator::alloc<ushort>(RenderBatchIndicesCount);
   }
}

void RenderSystem::unloadMaterial(const ObjectName& cMaterialName)
{
   Material* cMaterial = mMaterials.get(cMaterialName);
   GEAssert(cMaterial);

   if(GEHasFlag(cMaterial->getFlags(), MaterialFlagsBitMask::BatchRendering))
   {
      GESTLMap(uint, RenderOperation)::iterator it = mBatches.find(cMaterial->getName().getID());
      RenderOperation& sRenderBatch = it->second;
      GEInvokeDtor(MaterialPass, sRenderBatch.RenderMaterialPass);
      Allocator::free(sRenderBatch.RenderMaterialPass);
      Allocator::free(sRenderBatch.Data.VertexData);
      Allocator::free(sRenderBatch.Data.Indices);
      mBatches.erase(it);
   }

   mMaterials.remove(cMaterialName);
}

void RenderSystem::preloadTextures(const char* FileName)
{
   char sFileName[64];
   sprintf(sFileName, "%s.textures", FileName);

   ObjectName cGroupName = ObjectName(FileName);
   ContentData cTexturesData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Textures", sFileName, "xml", &cTexturesData);

      pugi::xml_document xml;
      xml.load_buffer(cTexturesData.getData(), cTexturesData.getDataSize());
      const pugi::xml_node& xmlTextures = xml.child("Textures");

      for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
      {
         const char* sTextureName = xmlTexture.attribute("name").value();
         const char* sTextureFileName = xmlTexture.attribute("fileName").value();
         const char* sTextureFormat = xmlTexture.attribute("format").value();
         const bool sTextureAtlas = strcmp(xmlTexture.attribute("atlas").value(), "true") == 0;

         PreloadedTexture sPreloadedTexture;
         sPreloadedTexture.Data = Allocator::alloc<ImageData>();
         GEInvokeCtor(ImageData, sPreloadedTexture.Data);

         Device::readContentFile(ContentType::Texture, "Textures", sTextureFileName, sTextureFormat, sPreloadedTexture.Data);

         sPreloadedTexture.Tex = Allocator::alloc<Texture>();
         GEInvokeCtor(Texture, sPreloadedTexture.Tex)
            (sTextureName, cGroupName, sPreloadedTexture.Data->getWidth(), sPreloadedTexture.Data->getHeight());

         GEMutexLock(mTextureLoadMutex);

         vPreloadedTextures.push_back(sPreloadedTexture);

         if(sTextureAtlas)
         {
            float fWidth = (float)sPreloadedTexture.Tex->getWidth();
            float fHeight = (float)sPreloadedTexture.Tex->getHeight();

            ContentData cAtlasInfo;
            Device::readContentFile(ContentType::TextureAtlasInfo, "Textures", sTextureFileName, "xml", &cAtlasInfo);

            pugi::xml_document xml;
            xml.load_buffer(cAtlasInfo.getData(), cAtlasInfo.getDataSize());
            const pugi::xml_node& xmlChars = xml.child("TextureAtlas");

            for(const pugi::xml_node& xmlChar : xmlChars.children("sprite"))
            {
               TextureCoordinates sAtlasUV;

               sAtlasUV.U0 = Parser::parseFloat(xmlChar.attribute("x").value()) / fWidth;
               sAtlasUV.U1 = sAtlasUV.U0 + (Parser::parseFloat(xmlChar.attribute("w").value()) / fWidth);
               sAtlasUV.V0 = Parser::parseFloat(xmlChar.attribute("y").value()) / fHeight;
               sAtlasUV.V1 = sAtlasUV.V0 + (Parser::parseFloat(xmlChar.attribute("h").value()) / fHeight);

               ObjectName cAtlasEntryName = ObjectName(xmlChar.attribute("n").value());
               TextureAtlasEntry sAtlasEntry = TextureAtlasEntry(cAtlasEntryName, sAtlasUV);

               sPreloadedTexture.Tex->AtlasUV.push_back(sAtlasEntry);
            }

            sPreloadedTexture.Tex->populateAtlasUVManager();
         }

         mTextures.add(sPreloadedTexture.Tex);

         GEMutexUnlock(mTextureLoadMutex);
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Textures", sFileName, "ge", &cTexturesData);
      ContentDataMemoryBuffer sMemoryBuffer(cTexturesData);
      std::istream sStream(&sMemoryBuffer);
      
      uint iTexturesCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iTexturesCount; i++)
      {
         ObjectName cTextureName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         const char* sTextureFormat = Value::fromStream(ValueType::String, sStream).getAsString();
         bool bTextureAtlas = Value::fromStream(ValueType::Bool, sStream).getAsBool();
         uint iTextureDataSize = Value::fromStream(ValueType::UInt, sStream).getAsUInt();

         PreloadedTexture sPreloadedTexture;
         sPreloadedTexture.Data = Allocator::alloc<ImageData>();
         GEInvokeCtor(ImageData, sPreloadedTexture.Data);
         sPreloadedTexture.Data->load(iTextureDataSize, sStream);

         sPreloadedTexture.Tex = Allocator::alloc<Texture>();
         GEInvokeCtor(Texture, sPreloadedTexture.Tex)
            (cTextureName, cGroupName, sPreloadedTexture.Data->getWidth(), sPreloadedTexture.Data->getHeight());

         GEMutexLock(mTextureLoadMutex);

         vPreloadedTextures.push_back(sPreloadedTexture);

         if(bTextureAtlas)
         {
            float fWidth = (float)sPreloadedTexture.Tex->getWidth();
            float fHeight = (float)sPreloadedTexture.Tex->getHeight();

            uint iAtlasEntriesCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

            for(uint j = 0; j < iAtlasEntriesCount; j++)
            {
               ObjectName cAtlasEntryName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();

               float x = (float)Value::fromStream(ValueType::Short, sStream).getAsShort();
               float y = (float)Value::fromStream(ValueType::Short, sStream).getAsShort();
               float w = (float)Value::fromStream(ValueType::Short, sStream).getAsShort();
               float h = (float)Value::fromStream(ValueType::Short, sStream).getAsShort();

               TextureCoordinates sUV;
               sUV.U0 = x / fWidth;
               sUV.U1 = sUV.U0 + (w / fWidth);
               sUV.V0 = y / fHeight;
               sUV.V1 = sUV.V0 + (h / fHeight);

               TextureAtlasEntry sAtlasEntry = TextureAtlasEntry(cAtlasEntryName, sUV);
               sPreloadedTexture.Tex->AtlasUV.push_back(sAtlasEntry);
            }

            sPreloadedTexture.Tex->populateAtlasUVManager();
         }

         mTextures.add(sPreloadedTexture.Tex);

         GEMutexUnlock(mTextureLoadMutex);
      }
   }
}

void RenderSystem::unloadTextures(const char* FileName)
{
   char sFileName[64];
   sprintf(sFileName, "%s.textures", FileName);

   ContentData cTexturesData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Textures", sFileName, "xml", &cTexturesData);

      pugi::xml_document xml;
      xml.load_buffer(cTexturesData.getData(), cTexturesData.getDataSize());
      const pugi::xml_node& xmlTextures = xml.child("Textures");

      for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
      {
         ObjectName cTextureName = ObjectName(xmlTexture.attribute("name").value());
         GEAssert(mTextures.get(cTextureName));
         mTextures.remove(cTextureName);
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Textures", sFileName, "ge", &cTexturesData);
      ContentDataMemoryBuffer sMemoryBuffer(cTexturesData);
      std::istream sStream(&sMemoryBuffer);

      uint iTexturesCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iTexturesCount; i++)
      {
         ObjectName cTextureName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         GEAssert(mTextures.get(cTextureName));
         mTextures.remove(cTextureName);
      }
   }
}

bool RenderSystem::loadNextPreloadedTexture()
{
   GEMutexLock(mTextureLoadMutex);

   if(vPreloadedTextures.empty())
   {
      GEMutexUnlock(mTextureLoadMutex);
      return false;
   }

   PreloadedTexture* cPreloadedTexture = &vPreloadedTextures.back();
   loadTexture(cPreloadedTexture);
   vPreloadedTextures.pop_back();

   GEMutexUnlock(mTextureLoadMutex);

   return true;
}

void RenderSystem::loadAllPreloadedTextures()
{
   while(loadNextPreloadedTexture())
   {
   }
}

bool RenderSystem::preloadedTexturesPending()
{
   GEMutexLock(mTextureLoadMutex);
   bool bPending = !vPreloadedTextures.empty();
   GEMutexUnlock(mTextureLoadMutex);
   return bPending;
}

Texture* RenderSystem::getTexture(const Core::ObjectName& Name)
{
   return mTextures.get(Name);
}

ComponentCamera* RenderSystem::getActiveCamera()
{
   return cActiveCamera;
}

void RenderSystem::setActiveCamera(ComponentCamera* Camera)
{
   cActiveCamera = Camera;
}

const Color& RenderSystem::getAmbientLightColor() const
{
   return cAmbientLightColor;
}

void RenderSystem::setAmbientLightColor(const Color& Color)
{
   cAmbientLightColor = Color;
}

void RenderSystem::loadMaterials(const char* FileName)
{
   char sFileName[64];
   sprintf(sFileName, "%s.materials", FileName);

   ObjectName cGroupName = ObjectName(FileName);
   ContentData cMaterialsData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Materials", sFileName, "xml", &cMaterialsData);

      pugi::xml_document xml;
      xml.load_buffer(cMaterialsData.getData(), cMaterialsData.getDataSize());
      const pugi::xml_node& xmlMaterials = xml.child("MaterialList");

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
      {
         Material* cMaterial = Allocator::alloc<Material>();
         GEInvokeCtor(Material, cMaterial)(xmlMaterial.attribute("name").value(), cGroupName);
         cMaterial->loadFromXml(xmlMaterial);
         loadMaterial(cMaterial);
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Materials", sFileName, "ge", &cMaterialsData);
      ContentDataMemoryBuffer sMemoryBuffer(cMaterialsData);
      std::istream sStream(&sMemoryBuffer);

      uint iMaterialsCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iMaterialsCount; i++)
      {
         ObjectName cMaterialName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         Material* cMaterial = Allocator::alloc<Material>();
         GEInvokeCtor(Material, cMaterial)(cMaterialName, cGroupName);
         cMaterial->loadFromStream(sStream);
         loadMaterial(cMaterial);
      }
   }
}

void RenderSystem::unloadMaterials(const char* FileName)
{
   char sFileName[64];
   sprintf(sFileName, "%s.materials", FileName);

   ContentData cMaterialsData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Materials", sFileName, "xml", &cMaterialsData);

      pugi::xml_document xml;
      xml.load_buffer(cMaterialsData.getData(), cMaterialsData.getDataSize());
      const pugi::xml_node& xmlMaterials = xml.child("MaterialList");

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
      {
         ObjectName cMaterialName = ObjectName(xmlMaterial.attribute("name").value());
         unloadMaterial(cMaterialName);
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Materials", sFileName, "ge", &cMaterialsData);
      ContentDataMemoryBuffer sMemoryBuffer(cMaterialsData);
      std::istream sStream(&sMemoryBuffer);

      uint iMaterialsCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iMaterialsCount; i++)
      {
         ObjectName cMaterialName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         unloadMaterial(cMaterialName);
      }
   }
}

Material* RenderSystem::getMaterial(const Core::ObjectName& Name)
{
   return mMaterials.get(Name);
}

ShaderProgram* RenderSystem::getShaderProgram(const Core::ObjectName& Name)
{
   return mShaderPrograms.get(Name);
}

void RenderSystem::requestShadersReload()
{
   bShaderReloadPending = true;
}

void RenderSystem::loadFonts(const char* FileName)
{
   char sFileName[64];
   sprintf(sFileName, "%s.fonts", FileName);

   ObjectName cGroupName = ObjectName(FileName);
   ContentData cFontsData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Fonts", sFileName, "xml", &cFontsData);

      pugi::xml_document xml;
      xml.load_buffer(cFontsData.getData(), cFontsData.getDataSize());
      const pugi::xml_node& xmlFonts = xml.child("Fonts");

      for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
      {
         const char* sFontName = xmlFont.attribute("name").value();
         const char* sFileName = xmlFont.attribute("fileName").value();

         Font* fFont = Allocator::alloc<Font>();
         GEInvokeCtor(Font, fFont)(sFontName, cGroupName, sFileName, pDevice);
         mFonts.add(fFont);
         pBoundTexture[(GE::uint)TextureSlot::Diffuse] = const_cast<Texture*>(fFont->getTexture());
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Fonts", sFileName, "ge", &cFontsData);
      ContentDataMemoryBuffer sMemoryBuffer(cFontsData);
      std::istream sStream(&sMemoryBuffer);

      uint iFontsCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iFontsCount; i++)
         Value::fromStream(ValueType::ObjectName, sStream);

      for(uint i = 0; i < iFontsCount; i++)
      {
         ObjectName cFontName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         Font* fFont = Allocator::alloc<Font>();
         GEInvokeCtor(Font, fFont)(cFontName, cGroupName, sStream, pDevice);
         mFonts.add(fFont);
         pBoundTexture[(GE::uint)TextureSlot::Diffuse] = const_cast<Texture*>(fFont->getTexture());
      }
   }
}

void RenderSystem::unloadFonts(const char* FileName)
{
   char sFileName[64];
   sprintf(sFileName, "%s.fonts", FileName);

   ContentData cFontsData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Fonts", sFileName, "xml", &cFontsData);

      pugi::xml_document xml;
      xml.load_buffer(cFontsData.getData(), cFontsData.getDataSize());
      const pugi::xml_node& xmlFonts = xml.child("Fonts");

      for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
      {
         ObjectName cFontName = ObjectName(xmlFont.attribute("name").value());
         GEAssert(mFonts.get(cFontName));
         mFonts.remove(cFontName);
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Fonts", sFileName, "ge", &cFontsData);
      ContentDataMemoryBuffer sMemoryBuffer(cFontsData);
      std::istream sStream(&sMemoryBuffer);

      uint iFontsCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iFontsCount; i++)
      {
         ObjectName cFontName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         GEAssert(mFonts.get(cFontName));
         mFonts.remove(cFontName);
      }
   }
}

Font* RenderSystem::getFont(const ObjectName& Name)
{
   return mFonts.get(Name);
}

void RenderSystem::queueForRendering(ComponentRenderable* Renderable)
{
   GEProfilerMarker("RenderSystem::queueForRendering()");

   if(!Renderable->getVisible() ||
      !Renderable->getOwner()->isActiveInHierarchy() ||
      Renderable->getMaterialPassCount() == 0 ||
      Renderable->getGeometryData().NumIndices == 0)
   {
      return;
   }

   ComponentUIElement* cUIElement = Renderable->getOwner()->getComponent<ComponentUIElement>();

   if(cUIElement && cUIElement->getAlphaInHierarchy() < GE_EPSILON)
   {
      return;
   }

   for(uint i = 0; i < Renderable->getMaterialPassCount(); i++)
   {
      MaterialPass* cMaterialPass = Renderable->getMaterialPass(i);

      if(!cMaterialPass->getMaterial() || !cMaterialPass->getActive())
         continue;

      if(GEHasFlag(cMaterialPass->getMaterial()->getFlags(), MaterialFlagsBitMask::BatchRendering))
      {
         GEMutexLock(mTextureLoadMutex);

         const uint iMaterialID = cMaterialPass->getMaterial()->getName().getID();
         RenderOperation& sBatch = mBatches.find(iMaterialID)->second;
         sBatch.Renderable = Renderable;
         sBatch.RenderMaterialPass = cMaterialPass;

         switch(Renderable->getRenderableType())
         {
         case RenderableType::Mesh:
            sBatch.Group = GeometryGroup::MeshBatch;
            break;
         case RenderableType::Sprite:
         case RenderableType::Label:
            sBatch.Group = GeometryGroup::_2DBatch;
            break;
         }

         queueForRenderingBatch(sBatch);

         GEMutexUnlock(mTextureLoadMutex);
      }
      else
      {
         RenderOperation sRenderOperation;
         sRenderOperation.Renderable = Renderable;
         sRenderOperation.RenderMaterialPass = cMaterialPass;

         switch(Renderable->getRenderableType())
         {
         case RenderableType::Mesh:
            sRenderOperation.Group = Renderable->getGeometryType() == GeometryType::Static ? GeometryGroup::MeshStatic : GeometryGroup::MeshDynamic;
            break;
         case RenderableType::Sprite:
         case RenderableType::Label:
            sRenderOperation.Group = Renderable->getGeometryType() == GeometryType::Static ? GeometryGroup::_2DStatic : GeometryGroup::_2DDynamic;
            break;
         case RenderableType::ParticleSystem:
            sRenderOperation.Group = GeometryGroup::Particles;
            break;
         }

         GEMutexLock(mTextureLoadMutex);

         queueForRenderingSingle(sRenderOperation);

         GEMutexUnlock(mTextureLoadMutex);
      }
   }
}

void RenderSystem::queueForRendering(ComponentLight* Light)
{
   if(Light->getOwner()->isActiveInHierarchy())
      vLightsToRender.push_back(Light);
}

void RenderSystem::queueForRenderingSingle(RenderOperation& sRenderOperation)
{
   GEProfilerMarker("RenderSystem::queueForRenderingSingle()");

   ComponentRenderable* cRenderable = sRenderOperation.Renderable;

   memcpy(&sRenderOperation.Data, &cRenderable->getGeometryData(), sizeof(GeometryData));
   uint iRenderableID = cRenderable->getOwner()->getFullName().getID();

   switch(cRenderable->getRenderableType())
   {
   case RenderableType::Mesh:
   {
      ComponentMesh* cMesh = static_cast<ComponentMesh*>(cRenderable);
      MaterialPass* cMaterialPass = sRenderOperation.RenderMaterialPass;

      if(GEHasFlag(cMesh->getDynamicShadows(), DynamicShadowsBitMask::Cast))
      {
         bool bAlreadyQueued = false;

         for(uint i = 0; i < vShadowedMeshesToRender.size(); i++)
         {
            if(vShadowedMeshesToRender[i].Renderable == cRenderable)
            {
               bAlreadyQueued = true;
               break;
            }
         }

         if(!bAlreadyQueued)
         {
            vShadowedMeshesToRender.push_back(sRenderOperation);
         }
      }

#if defined (GE_EDITOR_SUPPORT)
      if(GEHasFlag(cRenderable->getInternalFlags(), ComponentRenderable::InternalFlags::DebugGeometry))
      {
         vDebugGeometryToRender.push_back(sRenderOperation);
      }
      else
#endif
      {
         if(cMaterialPass->getMaterial()->getAlpha() < 0.99f)
         {
            vTransparentMeshesToRender.push_back(sRenderOperation);
         }
         else
         {
            vOpaqueMeshesToRender.push_back(sRenderOperation);
         }
      }

      if(cMesh->getGeometryType() == GeometryType::Static)
      {
         GESTLMap(uint, GeometryRenderInfo)::const_iterator it = mStaticGeometryToRender.find(iRenderableID);

         if(it == mStaticGeometryToRender.end())
         {
            GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::MeshStatic];
            mStaticGeometryToRender[iRenderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
            loadRenderingData(cRenderable->getGeometryData(), sBuffers, 4);
         }
      }
      else
      {
         GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::MeshDynamic];
         mDynamicGeometryToRender[iRenderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
         loadRenderingData(cRenderable->getGeometryData(), sBuffers, 4);
      }
   }
   break;

   case RenderableType::Sprite:
   {
      ComponentSprite* cSprite = static_cast<ComponentSprite*>(cRenderable);

#if defined (GE_EDITOR_SUPPORT)
      if(GEHasFlag(cRenderable->getInternalFlags(), ComponentRenderable::InternalFlags::DebugGeometry))
      {
         vDebugGeometryToRender.push_back(sRenderOperation);
      }
      else
#endif
      {
         if(cSprite->getLayer() == SpriteLayer::GUI)
         {
            ComponentUIElement* cUIElement = cRenderable->getOwner()->getComponent<ComponentUIElement>();

            if(!cUIElement || cUIElement->getUIElementType() == UIElementType::_2D)
            {
               vUIElementsToRender.push_back(sRenderOperation);
            }
            else
            {
               ComponentUI3DElement* cUI3DElement = static_cast<ComponentUI3DElement*>(cUIElement);
               GEAssert(cUI3DElement->getCanvasIndex() < ComponentUI3DElement::CanvasCount);

               v3DUIElementsToRender[cUI3DElement->getCanvasIndex()].push_back(sRenderOperation);

               if(!cRenderable->getOwner()->getParent() ||
                  !cRenderable->getOwner()->getParent()->getComponent<ComponentUIElement>())
               {
                  const Vector3& vWorldPosition = cUI3DElement->getOwner()->getComponent<ComponentTransform>()->getWorldPosition();
                  s3DUICanvasEntries[cUI3DElement->getCanvasIndex()].WorldPosition = vWorldPosition;
               }
            }
         }
         else
         {
            vPre3DSpritesToRender.push_back(sRenderOperation);
         }
      }

      if(cSprite->getGeometryType() == GeometryType::Static)
      {
         GESTLMap(uint, GeometryRenderInfo)::const_iterator it = mStaticGeometryToRender.find(iRenderableID);

         if(it == mStaticGeometryToRender.end())
         {
            GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::_2DStatic];
            mStaticGeometryToRender[iRenderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
            loadRenderingData(cRenderable->getGeometryData(), sBuffers);
         }
      }
      else
      {
         GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::_2DDynamic];
         mDynamicGeometryToRender[iRenderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
         loadRenderingData(cRenderable->getGeometryData(), sBuffers);
      }
   }
   break;

   case RenderableType::Label:
   {
#if defined (GE_EDITOR_SUPPORT)
      if(GEHasFlag(cRenderable->getInternalFlags(), ComponentRenderable::InternalFlags::DebugGeometry))
      {
         vDebugGeometryToRender.push_back(sRenderOperation);
      }
      else
#endif
      {
         ComponentUIElement* cUIElement = cRenderable->getOwner()->getComponent<ComponentUIElement>();

         if(cUIElement)
         {
            if(cUIElement->getUIElementType() == UIElementType::_2D)
            {
               vUIElementsToRender.push_back(sRenderOperation);
            }
            else
            {
               ComponentUI3DElement* cUI3DElement = static_cast<ComponentUI3DElement*>(cUIElement);
               GEAssert(cUI3DElement->getCanvasIndex() < ComponentUI3DElement::CanvasCount);

               v3DUIElementsToRender[cUI3DElement->getCanvasIndex()].push_back(sRenderOperation);

               if(!cRenderable->getOwner()->getParent() ||
                  !cRenderable->getOwner()->getParent()->getComponent<ComponentUIElement>())
               {
                  const Vector3& vWorldPosition = cUI3DElement->getOwner()->getComponent<ComponentTransform>()->getWorldPosition();
                  s3DUICanvasEntries[cUI3DElement->getCanvasIndex()].WorldPosition = vWorldPosition;
               }
            }
         }
         else
         {
            v3DLabelsToRender.push_back(sRenderOperation);
         }
      }

      GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::_2DDynamic];
      mDynamicGeometryToRender[iRenderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
      loadRenderingData(cRenderable->getGeometryData(), sBuffers);
   }
   break;

   case RenderableType::ParticleSystem:
   {
      if(cRenderable->getGeometryData().NumIndices > 0)
      {
         ComponentParticleSystem* cParticleSystem = static_cast<ComponentParticleSystem*>(cRenderable);

         if(cRenderable->getRenderingMode() == RenderingMode::_2D)
         {
            vUIElementsToRender.push_back(sRenderOperation);
         }
         else
         {
            if(cParticleSystem->getDynamicShadows())
            {
               vShadowedParticlesToRender.push_back(sRenderOperation);
            }

            vTransparentMeshesToRender.push_back(sRenderOperation);
         }

         GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::Particles];
         mDynamicGeometryToRender[iRenderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
         loadRenderingData(cRenderable->getGeometryData(), sBuffers, 4);
      }
   }
   break;
   }
}

void RenderSystem::queueForRenderingBatch(RenderOperation& sBatch)
{
   GEProfilerMarker("RenderSystem::queueForRenderingBatch()");

   ComponentRenderable* cRenderable = sBatch.Renderable;
   cRenderable->setGeometryType(GeometryType::Dynamic);

   ComponentTransform* cTransform = cRenderable->getTransform();
   const Matrix4& matModel = cTransform->getGlobalWorldMatrix();

   if(cRenderable->getRenderingMode() == RenderingMode::_2D)
      calculate2DTransformMatrix(matModel);
   else
      calculate3DTransformMatrix(matModel);
   
   const uint iRenderableNumVertices = cRenderable->getGeometryData().NumVertices;
   const uint iRenderableVertexStride = cRenderable->getGeometryData().VertexStride;
   const uint iBatchNumVertices = sBatch.Data.NumVertices;

   sBatch.Data.VertexStride = iRenderableVertexStride + sizeof(Matrix4);

   GE::byte* pRenderableVertexData = reinterpret_cast<GE::byte*>(cRenderable->getGeometryData().VertexData);
   GE::byte* pBatchVertexData = reinterpret_cast<GE::byte*>(sBatch.Data.VertexData);
   pBatchVertexData += iBatchNumVertices * sBatch.Data.VertexStride;

   for(uint i = 0; i < iRenderableNumVertices; i++)
   {
      memcpy(pBatchVertexData, &matModelViewProjection.m[0], sizeof(Matrix4));
      pBatchVertexData += sizeof(Matrix4);

      memcpy(pBatchVertexData, pRenderableVertexData, iRenderableVertexStride);
      pRenderableVertexData += iRenderableVertexStride;
      pBatchVertexData += iRenderableVertexStride;
   }

   const uint iRenderableNumIndices = cRenderable->getGeometryData().NumIndices;
   const uint iBatchNumIndices = sBatch.Data.NumIndices;

   GE::ushort* pRenderableIndices = cRenderable->getGeometryData().Indices;
   GE::ushort* pBatchIndices = sBatch.Data.Indices + iBatchNumIndices;

   GE::ushort iBatchNumVerticesUShort = (GE::ushort)iBatchNumVertices;

   for(uint i = 0; i < iRenderableNumIndices; i++)
   {
      *pBatchIndices = *pRenderableIndices + iBatchNumVerticesUShort;

      pRenderableIndices++;
      pBatchIndices++;
   }

   sBatch.Data.NumVertices += iRenderableNumVertices;
   sBatch.Data.NumIndices += iRenderableNumIndices;
}

void RenderSystem::prepareBatchForRendering(const RenderOperation& sBatch)
{
   GEProfilerMarker("RenderSystem::prepareBatchForRendering()");

   ComponentRenderable* cRenderable = sBatch.Renderable;
   GPUBufferPair& sBuffers = sGPUBufferPairs[sBatch.Group];

   const uint iRenderableID = cRenderable->getOwner()->getFullName().getID();
   mDynamicGeometryToRender[iRenderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);

   loadRenderingData(sBatch.Data, sBuffers, cRenderable->getRenderableType() == RenderableType::Mesh ? 4 : 2);

   switch(cRenderable->getRenderableType())
   {
   case RenderableType::Sprite:
   case RenderableType::Label:
      vUIElementsToRender.push_back(sBatch);
      break;
   case RenderableType::Mesh:
      vOpaqueMeshesToRender.push_back(sBatch);
      break;
   }
}

void RenderSystem::clearRenderingQueues()
{
   vUIElementsToRender.clear();
   vPre3DSpritesToRender.clear();
   v3DLabelsToRender.clear();
   vShadowedMeshesToRender.clear();
   vShadowedParticlesToRender.clear();
   vOpaqueMeshesToRender.clear();
   vTransparentMeshesToRender.clear();
   vDebugGeometryToRender.clear();
   vLightsToRender.clear();

   for(uint i = 0; i < ComponentUI3DElement::CanvasCount; i++)
   {
      v3DUIElementsToRender[i].clear();
      s3DUICanvasEntries[i].Index = i;
      s3DUICanvasEntries[i].WorldPosition = Vector3::Zero;
   }

   for(GESTLMap(uint, RenderOperation)::iterator it = mBatches.begin(); it != mBatches.end(); it++)
   {
      RenderOperation& sBatch = (*it).second;
      sBatch.Data.NumVertices = 0;
      sBatch.Data.NumIndices = 0;
   }

   sGPUBufferPairs[GeometryGroup::_2DDynamic].clear();
   sGPUBufferPairs[GeometryGroup::_2DBatch].clear();
   sGPUBufferPairs[GeometryGroup::MeshDynamic].clear();
   sGPUBufferPairs[GeometryGroup::MeshBatch].clear();
   sGPUBufferPairs[GeometryGroup::Particles].clear();
}

void RenderSystem::clearGeometryRenderInfoEntries()
{
   mStaticGeometryToRender.clear();
   mDynamicGeometryToRender.clear();
   mBatches.clear();

   for(uint i = 0; i < GeometryGroup::Count; i++)
   {
      sGPUBufferPairs[i].clear();
   }
}

int canvasSortComparer(const void* pCanvas1, const void* pCanvas2)
{
   const _3DUICanvasEntry* sCanvas1 = static_cast<const _3DUICanvasEntry*>(pCanvas1);
   const _3DUICanvasEntry* sCanvas2 = static_cast<const _3DUICanvasEntry*>(pCanvas2);

   RenderSystem* cRender = RenderSystem::getInstance();

   if(cRender->_3DUICanvasSortFunction)
   {
      return cRender->_3DUICanvasSortFunction(sCanvas1, sCanvas2) ? 1 : -1;
   }

   ComponentCamera* cActiveCamera = cRender->getActiveCamera();

   if(!cActiveCamera)
      return 1;

   const Vector3& vCameraPosition = cActiveCamera->getTransform()->getWorldPosition();

   return vCameraPosition.getSquaredDistanceTo(sCanvas1->WorldPosition) < vCameraPosition.getSquaredDistanceTo(sCanvas2->WorldPosition)
      ? 1
      : -1;
}

void RenderSystem::renderFrame()
{
   if(!mBatches.empty())
   {
      GESTLMap(uint, RenderOperation)::const_iterator it = mBatches.begin();

      for(; it != mBatches.end(); it++)
      {
         const RenderOperation& sBatch = it->second;

         if(sBatch.Data.NumVertices > 0)
         {
            prepareBatchForRendering(sBatch);
         }
      }
   }

   if(!vPre3DSpritesToRender.empty())
   {
      GESTLVector(RenderOperation)::const_iterator it = vPre3DSpritesToRender.begin();

      for(; it != vPre3DSpritesToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;
         useMaterial(sRenderOperation.RenderMaterialPass->getMaterial());
         render(sRenderOperation);
      }
   }

   if(!vOpaqueMeshesToRender.empty() ||
      !v3DLabelsToRender.empty() ||
      !vTransparentMeshesToRender.empty())
   {
      if(!vShadowedMeshesToRender.empty() || !vShadowedParticlesToRender.empty())
      {
         renderShadowMap();
      }

      if(!vOpaqueMeshesToRender.empty())
      {
         GESTLVector(RenderOperation)::const_iterator it = vOpaqueMeshesToRender.begin();

         for(; it != vOpaqueMeshesToRender.end(); it++)
         {
            const RenderOperation& sRenderOperation = *it;
            useMaterial(sRenderOperation.RenderMaterialPass->getMaterial());
            render(sRenderOperation);
         }
      }

      if(!v3DLabelsToRender.empty())
      {
         GESTLVector(RenderOperation)::const_iterator it = v3DLabelsToRender.begin();

         for(; it != v3DLabelsToRender.end(); it++)
         {
            const RenderOperation& sRenderOperation = *it;
            useMaterial(sRenderOperation.RenderMaterialPass->getMaterial());
            render(sRenderOperation);
         }
      }

      if(cActiveCamera && !vTransparentMeshesToRender.empty())
      {
         std::sort(vTransparentMeshesToRender.begin(), vTransparentMeshesToRender.end(),
         [this](const RenderOperation& sRO1, const RenderOperation& sRO2) -> bool
         {
            const Vector3& vCameraWorldPosition = cActiveCamera->getTransform()->getWorldPosition();

            Vector3 vP1ToCamera = vCameraWorldPosition - sRO1.Renderable->getTransform()->getWorldPosition();
            Vector3 vP2ToCamera = vCameraWorldPosition - sRO2.Renderable->getTransform()->getWorldPosition();

            return vP1ToCamera.getSquaredLength() > vP2ToCamera.getSquaredLength();
         });

         GESTLVector(RenderOperation)::const_iterator it = vTransparentMeshesToRender.begin();

         for(; it != vTransparentMeshesToRender.end(); it++)
         {
            const RenderOperation& sRenderOperation = *it;
            useMaterial(sRenderOperation.RenderMaterialPass->getMaterial());
            render(sRenderOperation);
         }
      }
   }

   if(cActiveCamera)
   {
      qsort(s3DUICanvasEntries, ComponentUI3DElement::CanvasCount, sizeof(_3DUICanvasEntry), canvasSortComparer);

      for(uint i = 0; i < ComponentUI3DElement::CanvasCount; i++)
      {
         uint iIndex = s3DUICanvasEntries[i].Index;

         if(!v3DUIElementsToRender[iIndex].empty())
         {
            GESTLVector(RenderOperation)::const_iterator it = v3DUIElementsToRender[iIndex].begin();

            for(; it != v3DUIElementsToRender[iIndex].end(); it++)
            {
               const RenderOperation& sRenderOperation = *it;
               useMaterial(sRenderOperation.RenderMaterialPass->getMaterial());
               render(sRenderOperation);
            }
         }
      }
   }

   if(!vUIElementsToRender.empty())
   {
      GESTLVector(RenderOperation)::const_iterator it = vUIElementsToRender.begin();

      for(; it != vUIElementsToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;
         useMaterial(sRenderOperation.RenderMaterialPass->getMaterial());
         render(sRenderOperation);
      }
   }

#if defined (GE_EDITOR_SUPPORT)
   if(!vDebugGeometryToRender.empty())
   {
      GESTLVector(RenderOperation)::const_iterator it = vDebugGeometryToRender.begin();

      for(; it != vDebugGeometryToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;
         useMaterial(sRenderOperation.RenderMaterialPass->getMaterial());
         render(sRenderOperation);
      }
   }
#endif

   float fCurrentTime = Time::getElapsed();
   fFramesPerSecond = 1.0f / (fCurrentTime - fFrameTime);
   fFrameTime = fCurrentTime;

   if(bShaderReloadPending)
   {
      loadShaders();

      if(Scene::getActiveScene())
      {
         const GESTLVector(Component*)& cRenderables = Scene::getActiveScene()->getComponents<ComponentRenderable>();

         for(uint i = 0; i < cRenderables.size(); i++)
         {
            ComponentRenderable* cRenderable = static_cast<ComponentRenderable*>(cRenderables[i]);

            for(uint j = 0; j < cRenderable->getMaterialPassCount(); j++)
            {
               MaterialPass* cMaterialPass = cRenderable->getMaterialPass(j);
               cMaterialPass->reload();
            }
         }
      }

      bShaderReloadPending = false;
   }
}
