
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
#include "Core/GELog.h"
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

const uint32_t kRenderBatchVertexDataFloatsCount = 1024u * 1024u;
const uint32_t kRenderBatchIndicesCount = 1024u * 256u;

const ObjectName RenderSystem::kShadowMapSolidProgram = ObjectName("ShadowMapSolid");
const ObjectName RenderSystem::kShadowMapAlphaProgram = ObjectName("ShadowMapAlpha");

const ObjectName _Mesh_ = ObjectName("Mesh");
const ObjectName _Sprite_ = ObjectName("Sprite");
const ObjectName _Label_ = ObjectName("Label");
const ObjectName _ParticleSystem_ = ObjectName("ParticleSystem");

RenderSystem::RenderSystem(void* Window, bool Windowed)
   : pWindow(Window)
   , bWindowed(Windowed)
   , cBackgroundColor(Color(0.0f, 0.0f, 0.0f))
   , cAmbientLightColor(Color(1.0f, 1.0f, 1.0f))
   , bClearGeometryRenderInfoEntriesPending(false)
   , bShaderReloadPending(false)
   , iActiveProgram(-1)
   , iCurrentVertexStride(0)
   , eBlendingMode(BlendingMode::None)
   , eDepthBufferMode(DepthBufferMode::NoDepth)
   , eCullingMode(CullingMode::Back)
   , cActiveCamera(0)
   , mAny3DUIElementsToRender(false)
   , fFrameTime(Time::getElapsed())
   , fFramesPerSecond(0.0f)
   , iDrawCalls(0)
{
   memset(pBoundTexture, 0, sizeof(Texture*) * (GE::uint)TextureSlot::Count);
   
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<ShaderProgram>(&mShaderPrograms);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<Texture>(&mTextures);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<Material>(&mMaterials);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<Font>(&mFonts);

   // Position (3) + UV (2)
   sGPUBufferPairs[GeometryGroup::SpriteStatic].VertexStride = (3 + 2) * sizeof(float);
   sGPUBufferPairs[GeometryGroup::SpriteDynamic].VertexStride = sGPUBufferPairs[GeometryGroup::SpriteStatic].VertexStride;
   // Position (3) + Color (4) + UV (2)
   sGPUBufferPairs[GeometryGroup::Label].VertexStride = (3 + 4 + 2) * sizeof(float);
   // Position (3) + Normal (3) + UV (2)
   sGPUBufferPairs[GeometryGroup::MeshStatic].VertexStride = (3 + 3 + 2) * sizeof(float);
   sGPUBufferPairs[GeometryGroup::MeshDynamic].VertexStride = sGPUBufferPairs[GeometryGroup::MeshStatic].VertexStride;
   // Position (3) + Color (4) + UV (2)
   sGPUBufferPairs[GeometryGroup::Particles].VertexStride = (3 + 4 + 2) * sizeof(float);

   // Batch buffers
   sGPUBufferPairs[GeometryGroup::SpriteBatch].VertexStride = sGPUBufferPairs[GeometryGroup::SpriteStatic].VertexStride;
   sGPUBufferPairs[GeometryGroup::LabelBatch].VertexStride = sGPUBufferPairs[GeometryGroup::Label].VertexStride;
   sGPUBufferPairs[GeometryGroup::MeshBatch].VertexStride = sGPUBufferPairs[GeometryGroup::MeshStatic].VertexStride;

   // Static buffers
   sGPUBufferPairs[GeometryGroup::SpriteStatic].IsDynamic = 0u;
   sGPUBufferPairs[GeometryGroup::MeshStatic].IsDynamic = 0u;

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

void RenderSystem::registerObjectManagers()
{
   ResourcesManager::getInstance()->registerObjectManager<ShaderProgram>(ShaderProgram::TypeName, &mShaderPrograms);
   ResourcesManager::getInstance()->registerObjectManager<Texture>(Texture::TypeName, &mTextures);
   ResourcesManager::getInstance()->registerObjectManager<Material>(Material::TypeName, &mMaterials);  
   ResourcesManager::getInstance()->registerObjectManager<Font>(Font::TypeName, &mFonts);
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
   Vector3 vLightPosition = Light->getTransform()->getWorldPosition();

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
      const uint32_t materialID = cMaterial->getName().getID();
      mBatches[materialID] = RenderOperation();

      GESTLMap(uint32_t, RenderOperation)::iterator it = mBatches.find(materialID);
      RenderOperation& renderBatch = it->second;
      renderBatch.mGeometryID = materialID;

      renderBatch.mRenderMaterialPass = Allocator::alloc<MaterialPass>();
      GEInvokeCtor(MaterialPass, renderBatch.mRenderMaterialPass)();
      renderBatch.mRenderMaterialPass->setMaterial(cMaterial);

      renderBatch.mData = Allocator::alloc<GeometryData>();
      GEInvokeCtor(GeometryData, renderBatch.mData);
      renderBatch.mData->VertexData = Allocator::alloc<float>(kRenderBatchVertexDataFloatsCount);
      renderBatch.mData->Indices = Allocator::alloc<ushort>(kRenderBatchIndicesCount);

      renderBatch.mDiffuseTexture = const_cast<Texture*>(cMaterial->getDiffuseTexture());
   }
}

void RenderSystem::unloadMaterial(const ObjectName& cMaterialName)
{
   Material* material = mMaterials.get(cMaterialName);
   GEAssert(material);

   if(GEHasFlag(material->getFlags(), MaterialFlagsBitMask::BatchRendering))
   {
      GESTLMap(uint32_t, RenderOperation)::iterator it = mBatches.find(material->getName().getID());
      RenderOperation& renderBatch = it->second;
      GEInvokeDtor(MaterialPass, renderBatch.mRenderMaterialPass);
      Allocator::free(renderBatch.mRenderMaterialPass);
      Allocator::free(renderBatch.mData->VertexData);
      Allocator::free(renderBatch.mData->Indices);
      Allocator::free(renderBatch.mData);
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
      const pugi::xml_node& xmlTextures = xml.child("TextureList");

      char sSubDir[256];
      sprintf(sSubDir, "Textures/%s", FileName);

#if defined (GE_EDITOR_SUPPORT)
      SerializableResourcesManager::getInstance()->registerSourceFile(
         Texture::TypeName, cGroupName, "Textures", "textures.xml");
#endif

      for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
      {
         const char* sTextureName = xmlTexture.attribute("name").value();

         PreloadedTexture sPreloadedTexture;
         sPreloadedTexture.Data = Allocator::alloc<ImageData>();
         GEInvokeCtor(ImageData, sPreloadedTexture.Data);

         sPreloadedTexture.Tex = Allocator::alloc<Texture>();
         GEInvokeCtor(Texture, sPreloadedTexture.Tex)(sTextureName, cGroupName);
         sPreloadedTexture.Tex->loadFromXml(xmlTexture);

#if defined (GE_PLATFORM_DESKTOP)
         const char* sTextureFormat = "dds";
#else
         const char* sTextureFormat = sPreloadedTexture.Tex->getFormat();
#endif

         Device::readContentFile(ContentType::Texture, sSubDir, sTextureName, sTextureFormat, sPreloadedTexture.Data);

         sPreloadedTexture.Tex->setWidth(sPreloadedTexture.Data->getWidth());
         sPreloadedTexture.Tex->setHeight(sPreloadedTexture.Data->getHeight());

         GEMutexLock(mTextureLoadMutex);

         vPreloadedTextures.push_back(sPreloadedTexture);

         if(GEHasFlag(sPreloadedTexture.Tex->getSettings(), TextureSettingsBitMask::AtlasUV))
         {
            float fWidth = (float)sPreloadedTexture.Tex->getWidth();
            float fHeight = (float)sPreloadedTexture.Tex->getHeight();

            ContentData cAtlasInfo;
            Device::readContentFile(ContentType::TextureAtlasInfo, sSubDir, sTextureName, "xml", &cAtlasInfo);

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
         }

         sPreloadedTexture.Tex->populateAtlasUVManager();

         mTextures.add(sPreloadedTexture.Tex);

         GEMutexUnlock(mTextureLoadMutex);
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Textures", sFileName, "ge", &cTexturesData);
      ContentDataMemoryBuffer sMemoryBuffer(cTexturesData);
      std::istream sStream(&sMemoryBuffer);
      
      uint iTexturesCount = (uint)Value::fromStream(ValueType::UShort, sStream).getAsUShort();

      for(uint i = 0; i < iTexturesCount; i++)
      {
         const ObjectName cTextureName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();

         PreloadedTexture sPreloadedTexture;
         sPreloadedTexture.Tex = Allocator::alloc<Texture>();
         GEInvokeCtor(Texture, sPreloadedTexture.Tex)(cTextureName, cGroupName);
         sPreloadedTexture.Tex->loadFromStream(sStream);

         const uint iTextureDataSize = Value::fromStream(ValueType::UInt, sStream).getAsUInt();

         sPreloadedTexture.Data = Allocator::alloc<ImageData>();
         GEInvokeCtor(ImageData, sPreloadedTexture.Data);
         sPreloadedTexture.Data->load(iTextureDataSize, sStream);

         sPreloadedTexture.Tex->setWidth(sPreloadedTexture.Data->getWidth());
         sPreloadedTexture.Tex->setHeight(sPreloadedTexture.Data->getHeight());

         GEMutexLock(mTextureLoadMutex);

         vPreloadedTextures.push_back(sPreloadedTexture);

         if(GEHasFlag(sPreloadedTexture.Tex->getSettings(), TextureSettingsBitMask::AtlasUV))
         {
            float fWidth = (float)sPreloadedTexture.Tex->getWidth();
            float fHeight = (float)sPreloadedTexture.Tex->getHeight();

            uint iAtlasEntriesCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

            for(uint j = 0; j < iAtlasEntriesCount; j++)
            {
               ObjectName cAtlasEntryName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();

               float x = (float)Value::fromStream(ValueType::UShort, sStream).getAsUShort();
               float y = (float)Value::fromStream(ValueType::UShort, sStream).getAsUShort();
               float w = (float)Value::fromStream(ValueType::UShort, sStream).getAsUShort();
               float h = (float)Value::fromStream(ValueType::UShort, sStream).getAsUShort();

               TextureCoordinates sUV;
               sUV.U0 = x / fWidth;
               sUV.U1 = sUV.U0 + (w / fWidth);
               sUV.V0 = y / fHeight;
               sUV.V1 = sUV.V0 + (h / fHeight);

               TextureAtlasEntry sAtlasEntry = TextureAtlasEntry(cAtlasEntryName, sUV);
               sPreloadedTexture.Tex->AtlasUV.push_back(sAtlasEntry);
            }
         }

         sPreloadedTexture.Tex->populateAtlasUVManager();

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
      const pugi::xml_node& xmlTextures = xml.child("TextureList");

      for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
      {
         ObjectName cTextureName = ObjectName(xmlTexture.attribute("name").value());
         Texture* cTexture = mTextures.get(cTextureName);
         GEAssert(cTexture);
         unloadTexture(cTexture);
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
         Texture* cTexture = mTextures.get(cTextureName);
         GEAssert(cTexture);
         unloadTexture(cTexture);
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

#if defined (GE_EDITOR_SUPPORT)
      SerializableResourcesManager::getInstance()->registerSourceFile(
         Material::TypeName, cGroupName, "Materials", "materials.xml");
#endif

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
   ObjectName cGroupName = ObjectName(FileName);
   ContentData cFontsData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Fonts", FileName, "fonts.xml", &cFontsData);

#if defined (GE_EDITOR_SUPPORT)
      SerializableResourcesManager::getInstance()->registerSourceFile(
         Font::TypeName, cGroupName, "Fonts", "fonts.xml");
#endif

      pugi::xml_document xml;
      xml.load_buffer(cFontsData.getData(), cFontsData.getDataSize());
      const pugi::xml_node& xmlFonts = xml.child("FontList");

      for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
      {
         const char* sFontName = xmlFont.attribute("name").value();

         Font* fFont = Allocator::alloc<Font>();
         GEInvokeCtor(Font, fFont)(sFontName, cGroupName);
         fFont->loadFromXml(xmlFont);
         fFont->load(pDevice);
         mFonts.add(fFont);
         pBoundTexture[(GE::uint)TextureSlot::Diffuse] = const_cast<Texture*>(fFont->getTexture());
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Fonts", FileName, "fonts.ge", &cFontsData);
      ContentDataMemoryBuffer sMemoryBuffer(cFontsData);
      std::istream sStream(&sMemoryBuffer);

      uint iFontsCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iFontsCount; i++)
         Value::fromStream(ValueType::ObjectName, sStream);

      for(uint i = 0; i < iFontsCount; i++)
      {
         ObjectName cFontName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         Font* fFont = Allocator::alloc<Font>();
         GEInvokeCtor(Font, fFont)(cFontName, cGroupName);
         fFont->loadFromStream(sStream);
         fFont->load(sStream, pDevice);
         mFonts.add(fFont);
         pBoundTexture[(GE::uint)TextureSlot::Diffuse] = const_cast<Texture*>(fFont->getTexture());
      }
   }
}

void RenderSystem::unloadFonts(const char* FileName)
{
   ContentData cFontsData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Fonts", FileName, "fonts.xml", &cFontsData);

      pugi::xml_document xml;
      xml.load_buffer(cFontsData.getData(), cFontsData.getDataSize());
      const pugi::xml_node& xmlFonts = xml.child("FontList");

      for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
      {
         ObjectName cFontName = ObjectName(xmlFont.attribute("name").value());
         GEAssert(mFonts.get(cFontName));
         mFonts.remove(cFontName);
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Fonts", FileName, "fonts.ge", &cFontsData);
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

void RenderSystem::setup3DUICanvas(uint32_t pCanvasIndex, const Vector3& pWorldPosition, uint16_t pSettings)
{
   GEAssert(pCanvasIndex < k3DUICanvasCount);

   s3DUICanvasEntries[pCanvasIndex].WorldPosition = pWorldPosition;
   s3DUICanvasEntries[pCanvasIndex].Settings = pSettings;
}

void RenderSystem::queueForRendering(ComponentRenderable* Renderable, uint RequestIndex)
{
   GEProfilerMarker("RenderSystem::queueForRendering()");

   Renderable->setRenderPass(RenderPass::None);

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
      MaterialPass* materialPass = Renderable->getMaterialPass(i);

      if(!materialPass->getMaterial() || !materialPass->getActive())
         continue;

      if(GEHasFlag(materialPass->getMaterial()->getFlags(), MaterialFlagsBitMask::BatchRendering))
      {
         GEMutexLock(mTextureLoadMutex);

         const uint32_t materialID = materialPass->getMaterial()->getName().getID();
         RenderOperation& batch = mBatches.find(materialID)->second;
         batch.mRenderMaterialPass = materialPass;

         if(Renderable->getClassName() == _Mesh_)
         {
            batch.mGroup = GeometryGroup::MeshBatch;
         }
         else if(Renderable->getClassName() == _Sprite_)
         {
            batch.mGroup = GeometryGroup::SpriteBatch;
         }
         else if(Renderable->getClassName() == _Label_)
         {
            batch.mGroup = GeometryGroup::LabelBatch;
         }

         queueForRenderingBatch(Renderable, batch);

         GEMutexUnlock(mTextureLoadMutex);
      }
      else
      {
         RenderOperation renderOperation;
         renderOperation.mIndex = (Renderable->getRenderPriority() << 24) | (RequestIndex << 8) | i;
         renderOperation.mRenderMaterialPass = materialPass;

         if(Renderable->getClassName() == _Mesh_)
         {
            renderOperation.mGroup = Renderable->getGeometryType() == GeometryType::Static ? GeometryGroup::MeshStatic : GeometryGroup::MeshDynamic;
         }
         else if(Renderable->getClassName() == _Sprite_)
         {
            renderOperation.mGroup = Renderable->getGeometryType() == GeometryType::Static ? GeometryGroup::SpriteStatic : GeometryGroup::SpriteDynamic;
         }
         else if(Renderable->getClassName() == _Label_)
         {
            renderOperation.mGroup = GeometryGroup::Label;
         }
         else if(Renderable->getClassName() == _ParticleSystem_)
         {
            renderOperation.mGroup = GeometryGroup::Particles;
         }

         GEMutexLock(mTextureLoadMutex);

         queueForRenderingSingle(Renderable, renderOperation);

         GEMutexUnlock(mTextureLoadMutex);
      }
   }
}

void RenderSystem::queueForRendering(ComponentLight* Light)
{
   if(Light->getOwner()->isActiveInHierarchy())
   {
      vLightsToRender.push_back(Light);
   }
}

void RenderSystem::queueForRenderingSingle(ComponentRenderable* pRenderable, RenderOperation& sRenderOperation)
{
   GEProfilerMarker("RenderSystem::queueForRenderingSingle()");

#if defined (GE_EDITOR_SUPPORT)
   if(pRenderable->getRenderingMode() == RenderingMode::_3D && !cActiveCamera)
   {
      Entity* owner = pRenderable->getOwner();
      Log::log(LogType::Warning, "There is no active camera. The entity '%s' will be deactivated.", owner->getFullName().getString());
      owner->setActive(false);
      return;
   }
#endif

   const uint32_t renderableID = pRenderable->getOwner()->getFullName().getID();
   ComponentTransform* transform = pRenderable->getOwner()->getComponent<ComponentTransform>();
   ComponentUIElement* uiElement = pRenderable->getOwner()->getComponent<ComponentUIElement>();

   sRenderOperation.mGeometryID = renderableID;
   sRenderOperation.mVertexIndexSize = pRenderable->getClassName() == _Mesh_ || pRenderable->getClassName() == _ParticleSystem_
      ? 4u
      : 2u;
   sRenderOperation.mData = const_cast<GeometryData*>(&pRenderable->getGeometryData());
   sRenderOperation.mColor = pRenderable->getColor();

   if(uiElement)
   {
      sRenderOperation.mColor.Alpha *= uiElement->getAlphaInHierarchy();
   }

   sRenderOperation.mWorldTransform = transform->getGlobalWorldMatrix();

   if(pRenderable->getRenderingMode() == RenderingMode::_3D)
   {
      GESetFlag(sRenderOperation.mFlags, RenderOperationFlags::RenderThroughActiveCamera);
   }

   sRenderOperation.mDiffuseTexture = const_cast<Texture*>(sRenderOperation.mRenderMaterialPass->getMaterial()->getDiffuseTexture());

   if(pRenderable->getClassName() == _Mesh_)
   {
      ComponentMesh* cMesh = static_cast<ComponentMesh*>(pRenderable);
      MaterialPass* cMaterialPass = sRenderOperation.mRenderMaterialPass;

      if(GEHasFlag(cMesh->getDynamicShadows(), DynamicShadowsBitMask::Cast))
      {
         bool bAlreadyQueued = false;

         for(uint i = 0; i < vShadowedMeshesToRender.size(); i++)
         {
            if(vShadowedMeshesToRender[i].mGeometryID == renderableID)
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

      GESetFlag(sRenderOperation.mFlags, RenderOperationFlags::LightingSupport);

      if(GEHasFlag(cMesh->getDynamicShadows(), DynamicShadowsBitMask::Receive))
      {
         GESetFlag(sRenderOperation.mFlags, RenderOperationFlags::BindShadowMap);
      }

#if defined (GE_EDITOR_SUPPORT)
      if(GEHasFlag(pRenderable->getInternalFlags(), ComponentRenderable::InternalFlags::DebugGeometry))
      {
         vDebugGeometryToRender.push(sRenderOperation);
         pRenderable->setRenderPass(RenderPass::_09_DebugGeometry);
      }
      else
#endif
      {
         if(uiElement)
         {
            queueForRendering3DUI(pRenderable, sRenderOperation);
         }
         else if(GEHasFlag(cMesh->getSettings(), MeshSettingsBitMask::Transparency))
         {
            vTransparentMeshesToRender.push_back(sRenderOperation);
            pRenderable->setRenderPass(RenderPass::_04_TransparentMeshes);
         }
         else
         {
            vOpaqueMeshesToRender.push(sRenderOperation);
            pRenderable->setRenderPass(RenderPass::_02_OpaqueMeshes);
         }
      }

      if(cMesh->getGeometryType() == GeometryType::Static)
      {
         GESTLMap(uint, GeometryRenderInfo)::const_iterator it = mStaticGeometryToRender.find(renderableID);

         if(it == mStaticGeometryToRender.end())
         {
            GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::MeshStatic];
            mStaticGeometryToRender[renderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
            loadRenderingData(sRenderOperation.mData, sBuffers, 4u);
         }
      }
      else
      {
         GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::MeshDynamic];
         mDynamicGeometryToRender[renderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
         loadRenderingData(sRenderOperation.mData, sBuffers, 4u);
      }
   }
   else if(pRenderable->getClassName() == _Sprite_)
   {
      ComponentSprite* sprite = static_cast<ComponentSprite*>(pRenderable);

#if defined (GE_EDITOR_SUPPORT)
      if(GEHasFlag(pRenderable->getInternalFlags(), ComponentRenderable::InternalFlags::DebugGeometry))
      {
         vDebugGeometryToRender.push(sRenderOperation);
         pRenderable->setRenderPass(RenderPass::_09_DebugGeometry);
      }
      else
#endif
      {
         if(sprite->getLayer() == SpriteLayer::GUI)
         {
            if(!uiElement || uiElement->getClassName() == ComponentUI2DElement::ClassName)
            {
               vUIElementsToRender.push(sRenderOperation);
               pRenderable->setRenderPass(RenderPass::_06_UI2D);
            }
            else
            {
               queueForRendering3DUI(pRenderable, sRenderOperation);
            }
         }
         else if(sprite->getLayer() == SpriteLayer::Pre3D)
         {
            vPre3DSpritesToRender.push(sRenderOperation);
            pRenderable->setRenderPass(RenderPass::_01_Pre3D);
         }
         else
         {
            vPostUISpritesToRender.push(sRenderOperation);
            pRenderable->setRenderPass(RenderPass::_08_PostUI);
         }
      }

      if(sprite->getGeometryType() == GeometryType::Static)
      {
         GESTLMap(uint, GeometryRenderInfo)::const_iterator it = mStaticGeometryToRender.find(renderableID);

         if(it == mStaticGeometryToRender.end())
         {
            GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::SpriteStatic];
            mStaticGeometryToRender[renderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
            loadRenderingData(sRenderOperation.mData, sBuffers);
         }
      }
      else
      {
         GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::SpriteDynamic];
         mDynamicGeometryToRender[renderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
         loadRenderingData(sRenderOperation.mData, sBuffers);
      }
   }
   else if(pRenderable->getClassName() == _Label_)
   {
      ComponentLabel* label = static_cast<ComponentLabel*>(pRenderable);
      sRenderOperation.mDiffuseTexture = const_cast<Texture*>(label->getFont()->getTexture());

#if defined (GE_EDITOR_SUPPORT)
      if(GEHasFlag(pRenderable->getInternalFlags(), ComponentRenderable::InternalFlags::DebugGeometry))
      {
         vDebugGeometryToRender.push(sRenderOperation);
         pRenderable->setRenderPass(RenderPass::_09_DebugGeometry);
      }
      else
#endif
      {
         if(label->getLayer() == SpriteLayer::GUI)
         {
            if(uiElement)
            {
               if(uiElement->getClassName() == ComponentUI2DElement::ClassName)
               {
                  vUIElementsToRender.push(sRenderOperation);
                  pRenderable->setRenderPass(RenderPass::_06_UI2D);
               }
               else
               {
                  queueForRendering3DUI(pRenderable, sRenderOperation);
               }
            }
            else
            {
               v3DLabelsToRender.push(sRenderOperation);
               pRenderable->setRenderPass(RenderPass::_03_Labels3D);
            }
         }
         else if(label->getLayer() == SpriteLayer::Pre3D)
         {
            vPre3DSpritesToRender.push(sRenderOperation);
            pRenderable->setRenderPass(RenderPass::_01_Pre3D);
         }
         else
         {
            vPostUISpritesToRender.push(sRenderOperation);
            pRenderable->setRenderPass(RenderPass::_08_PostUI);
         }
      }

      GPUBufferPair& buffers = sGPUBufferPairs[GeometryGroup::Label];
      mDynamicGeometryToRender[renderableID] = GeometryRenderInfo(buffers.CurrentVertexBufferOffset, buffers.CurrentIndexBufferOffset);
      loadRenderingData(sRenderOperation.mData, buffers);
   }
   else if(pRenderable->getClassName() == _ParticleSystem_)
   {
      if(pRenderable->getGeometryData().NumIndices > 0)
      {
         ComponentParticleSystem* cParticleSystem = static_cast<ComponentParticleSystem*>(pRenderable);

         if(GEHasFlag(cParticleSystem->getSettings(), ParticleSystemSettingsBitMask::DynamicShadows) &&
            pRenderable->getRenderingMode() == RenderingMode::_3D)
         {
            vShadowedParticlesToRender.push_back(sRenderOperation);
         }

         if(uiElement)
         {
            if(uiElement->getClassName() == ComponentUI2DElement::ClassName)
            {
               vUIElementsToRender.push(sRenderOperation);
               pRenderable->setRenderPass(RenderPass::_06_UI2D);
            }
            else
            {
               queueForRendering3DUI(pRenderable, sRenderOperation);
            }
         }
         else if(pRenderable->getRenderingMode() == RenderingMode::_2D)
         {
            vUIElementsToRender.push(sRenderOperation);
            pRenderable->setRenderPass(RenderPass::_06_UI2D);
         }
         else
         {
            vTransparentMeshesToRender.push_back(sRenderOperation);
            pRenderable->setRenderPass(RenderPass::_04_TransparentMeshes);
         }

         GPUBufferPair& sBuffers = sGPUBufferPairs[GeometryGroup::Particles];
         mDynamicGeometryToRender[renderableID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);
         loadRenderingData(sRenderOperation.mData, sBuffers, 4u);
      }
   }
}

void RenderSystem::queueForRendering3DUI(ComponentRenderable* pRenderable, RenderOperation& pRenderOperation)
{
   ComponentUI3DElement* uiElement = pRenderable->getOwner()->getComponent<ComponentUI3DElement>();
   GEAssert(uiElement);

   const uint32_t canvasIndex = (uint32_t)uiElement->getCanvasIndex();
   GEAssert(uiElement->getCanvasIndex() < k3DUICanvasCount);
   
   const uint16_t canvasSettings = s3DUICanvasEntries[canvasIndex].Settings;
   const bool firstPass = !GEHasFlag(canvasSettings, CanvasSettingsBitMask::RenderAfter2DElements);

   v3DUIElementsToRender[uiElement->getCanvasIndex()].push(pRenderOperation);
   pRenderable->setRenderPass(firstPass ? RenderPass::_05_UI3DFirst : RenderPass::_07_UI3DSecond);

   mAny3DUIElementsToRender = true;
}

void RenderSystem::queueForRenderingBatch(ComponentRenderable* pRenderable, RenderOperation& sBatch)
{
   GEProfilerMarker("RenderSystem::queueForRenderingBatch()");

   pRenderable->setGeometryType(GeometryType::Dynamic);

   ComponentTransform* cTransform = pRenderable->getTransform();
   const Matrix4& matModel = cTransform->getGlobalWorldMatrix();
   
   const uint iRenderableNumVertices = pRenderable->getGeometryData().NumVertices;
   const uint iRenderableVertexStride = pRenderable->getGeometryData().VertexStride;
   const uint iBatchNumVertices = sBatch.mData->NumVertices;

   sBatch.mData->VertexStride = iRenderableVertexStride;

   GE::byte* pRenderableVertexData = reinterpret_cast<GE::byte*>(pRenderable->getGeometryData().VertexData);
   GE::byte* pBatchVertexData = reinterpret_cast<GE::byte*>(sBatch.mData->VertexData);
   pBatchVertexData += iBatchNumVertices * sBatch.mData->VertexStride;

   for(uint i = 0; i < iRenderableNumVertices; i++)
   {
      Vector3 vVertexPosition = *(reinterpret_cast<Vector3*>(pRenderableVertexData));
      pRenderableVertexData += sizeof(Vector3);

      Matrix4Transform(matModel, &vVertexPosition);

      memcpy(pBatchVertexData, &vVertexPosition, sizeof(Vector3));
      pBatchVertexData += sizeof(Vector3);

      memcpy(pBatchVertexData, pRenderableVertexData, iRenderableVertexStride - sizeof(Vector3));
      pRenderableVertexData += iRenderableVertexStride - sizeof(Vector3);
      pBatchVertexData += iRenderableVertexStride - sizeof(Vector3);
   }

   const uint iRenderableNumIndices = pRenderable->getGeometryData().NumIndices;
   const uint iBatchNumIndices = sBatch.mData->NumIndices;

   GE::ushort* pRenderableIndices = pRenderable->getGeometryData().Indices;
   GE::ushort* pBatchIndices = sBatch.mData->Indices + iBatchNumIndices;

   GE::ushort iBatchNumVerticesUShort = (GE::ushort)iBatchNumVertices;

   for(uint i = 0; i < iRenderableNumIndices; i++)
   {
      *pBatchIndices = *pRenderableIndices + iBatchNumVerticesUShort;

      pRenderableIndices++;
      pBatchIndices++;
   }

   sBatch.mData->NumVertices += iRenderableNumVertices;
   sBatch.mData->NumIndices += iRenderableNumIndices;
}

void RenderSystem::prepareBatchForRendering(const RenderOperation& sBatch)
{
   GEProfilerMarker("RenderSystem::prepareBatchForRendering()");

   GPUBufferPair& sBuffers = sGPUBufferPairs[sBatch.mGroup];
   mDynamicGeometryToRender[sBatch.mGeometryID] = GeometryRenderInfo(sBuffers.CurrentVertexBufferOffset, sBuffers.CurrentIndexBufferOffset);

   loadRenderingData(sBatch.mData, sBuffers, sBatch.mVertexIndexSize);

   //TODO: push the batch into the corresponding queue
   vUIElementsToRender.push(sBatch);
}

void RenderSystem::clearRenderingQueues()
{
   vShadowedMeshesToRender.clear();
   vShadowedParticlesToRender.clear();
   vTransparentMeshesToRender.clear();
   vLightsToRender.clear();

   for(uint32_t i = 0u; i < k3DUICanvasCount; i++)
   {
      s3DUICanvasEntries[i].Index = (uint16_t)i;
      s3DUICanvasEntries[i].Settings = 0u;
      s3DUICanvasEntries[i].WorldPosition = Vector3::Zero;
   }

   mAny3DUIElementsToRender = false;

   for(GESTLMap(uint, RenderOperation)::iterator it = mBatches.begin(); it != mBatches.end(); it++)
   {
      RenderOperation& sBatch = (*it).second;
      sBatch.mData->NumVertices = 0;
      sBatch.mData->NumIndices = 0;
   }

   for(int i = 0; i < GeometryGroup::Count; i++)
   {
      GPUBufferPair& bufferPair = sGPUBufferPairs[i];

      if(bufferPair.IsDynamic)
      {
         bufferPair.clear();
      }
   }
}

void RenderSystem::clearGeometryRenderInfoEntries()
{
   bClearGeometryRenderInfoEntriesPending = true;
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

         if(sBatch.mData->NumVertices > 0)
         {
            prepareBatchForRendering(sBatch);
         }
      }
   }

   while(!vPre3DSpritesToRender.empty())
   {
      const RenderOperation& sRenderOperation = vPre3DSpritesToRender.top();
      useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
      render(sRenderOperation);
      vPre3DSpritesToRender.pop();
      iDrawCalls++;
   }

   if(!vOpaqueMeshesToRender.empty() ||
      !v3DLabelsToRender.empty() ||
      !vTransparentMeshesToRender.empty() ||
      mAny3DUIElementsToRender)
   {
      if(!vShadowedMeshesToRender.empty() || !vShadowedParticlesToRender.empty())
      {
         renderShadowMap();
      }

      while(!vOpaqueMeshesToRender.empty())
      {
         const RenderOperation& sRenderOperation = vOpaqueMeshesToRender.top();
         useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
         render(sRenderOperation);
         vOpaqueMeshesToRender.pop();
         iDrawCalls++;
      }

      while(!v3DLabelsToRender.empty())
      {
         const RenderOperation& sRenderOperation = v3DLabelsToRender.top();
         useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
         render(sRenderOperation);
         v3DLabelsToRender.pop();
         iDrawCalls++;
      }

      if(cActiveCamera)
      {
         if(!vTransparentMeshesToRender.empty())
         {
            std::sort(vTransparentMeshesToRender.begin(), vTransparentMeshesToRender.end(),
            [this](const RenderOperation& sRO1, const RenderOperation& sRO2) -> bool
            {
               const Vector3& vCameraWorldPosition = cActiveCamera->getTransform()->getWorldPosition();

               const Vector3 p1WorldPosition = Vector3
               (
                  sRO1.mWorldTransform.m[GE_M4_1_4],
                  sRO1.mWorldTransform.m[GE_M4_2_4],
                  sRO1.mWorldTransform.m[GE_M4_3_4]
               );
               const Vector3 p2WorldPosition = Vector3
               (
                  sRO2.mWorldTransform.m[GE_M4_1_4],
                  sRO2.mWorldTransform.m[GE_M4_2_4],
                  sRO2.mWorldTransform.m[GE_M4_3_4]
               );

               const Vector3 vP1ToCamera = vCameraWorldPosition - p1WorldPosition;
               const Vector3 vP2ToCamera = vCameraWorldPosition - p2WorldPosition;

               return vP1ToCamera.getSquaredLength() > vP2ToCamera.getSquaredLength();
            });

            GESTLVector(RenderOperation)::const_iterator it = vTransparentMeshesToRender.begin();

            for(; it != vTransparentMeshesToRender.end(); it++)
            {
               const RenderOperation& sRenderOperation = *it;
               useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
               render(sRenderOperation);
               iDrawCalls++;
            }
         }

         qsort(s3DUICanvasEntries, k3DUICanvasCount, sizeof(_3DUICanvasEntry), canvasSortComparer);

         for(uint32_t i = 0u; i < k3DUICanvasCount; i++)
         {
            if(!GEHasFlag(s3DUICanvasEntries[i].Settings, CanvasSettingsBitMask::RenderAfter2DElements))
            {
               uint iIndex = s3DUICanvasEntries[i].Index;

               while(!v3DUIElementsToRender[iIndex].empty())
               {
                  const RenderOperation& sRenderOperation = v3DUIElementsToRender[iIndex].top();
                  useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
                  render(sRenderOperation);
                  v3DUIElementsToRender[iIndex].pop();
                  iDrawCalls++;
               }
            }
         }
      }
   }

   while(!vUIElementsToRender.empty())
   {
      const RenderOperation& sRenderOperation = vUIElementsToRender.top();
      useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
      render(sRenderOperation);
      vUIElementsToRender.pop();
      iDrawCalls++;
   }

   for(uint32_t i = 0u; i < k3DUICanvasCount; i++)
   {
      const uint32_t index = s3DUICanvasEntries[i].Index;

      while(!v3DUIElementsToRender[index].empty())
      {
         const RenderOperation& sRenderOperation = v3DUIElementsToRender[index].top();
         useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
         render(sRenderOperation);
         v3DUIElementsToRender[index].pop();
         iDrawCalls++;
      }
   }

   while(!vPostUISpritesToRender.empty())
   {
      const RenderOperation& sRenderOperation = vPostUISpritesToRender.top();
      useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
      render(sRenderOperation);
      vPostUISpritesToRender.pop();
      iDrawCalls++;
   }

#if defined (GE_EDITOR_SUPPORT)
   while(!vDebugGeometryToRender.empty())
   {
      const RenderOperation& sRenderOperation = vDebugGeometryToRender.top();
      useMaterial(sRenderOperation.mRenderMaterialPass->getMaterial());
      render(sRenderOperation);
      vDebugGeometryToRender.pop();
   }
#endif

   float fCurrentTime = Time::getElapsed();
   fFramesPerSecond = 1.0f / (fCurrentTime - fFrameTime);
   fFrameTime = fCurrentTime;

   if(bClearGeometryRenderInfoEntriesPending)
   {
      mStaticGeometryToRender.clear();
      mDynamicGeometryToRender.clear();

      for(uint i = 0; i < GeometryGroup::Count; i++)
      {
         sGPUBufferPairs[i].clear();
      }

      bClearGeometryRenderInfoEntriesPending = false;
   }

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
