
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (OpenGL ES)
//
//  --- GERenderSystemES20.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GERenderSystemES20.h"
#include "Rendering/GERenderingObjects.h"
#include "Core/GEDevice.h"
#include "Core/GELog.h"
#include "Core/GEAllocator.h"
#include "Core/GEProfiler.h"
#include "Core/GEApplication.h"
#include "Content/GEImageData.h"
#include "Entities/GEEntity.h"
#include "pugixml/pugixml.hpp"
#include "GEOpenGLES20.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Entities;
using namespace GE::Rendering;

// Buffers
void* gCurrentVertexBuffer = nullptr;
void* gCurrentIndexBuffer = nullptr;

GESTLVector(uint16_t) gMappedIndices16;
GESTLVector(uint32_t) gMappedIndices32;

// Shadow mapping
uint32_t gFrameBuffer = 0u;
uint32_t gRenderBuffer = 0u;
Texture* gDepthTexture = nullptr;

// Shaders
ShaderProgramES20* gActiveProgram = nullptr;

const ObjectName _Mesh_ = ObjectName("Mesh");
const ObjectName _Label_ = ObjectName("Label");
const ObjectName _ParticleSystem_ = ObjectName("ParticleSystem");

RenderSystemES20::RenderSystemES20()
   : RenderSystem(nullptr, false)
{
   const GLubyte* glVendor = glGetString(GL_VENDOR);
   const GLubyte* glRenderer = glGetString(GL_RENDERER);

   Log::log(LogType::Info, "Graphics Card: %s (%s)", glRenderer, glVendor);

#if defined (GE_PLATFORM_DESKTOP)
   GLint graphicsCardValues[4] = { 0 };
   glGetIntegerv(0x9049, graphicsCardValues); // GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
   glGetIntegerv(0x87FC, graphicsCardValues); // TEXTURE_FREE_MEMORY_ATI
   glGetError(); // (one of the two calls should have set the error state)
   mVRAMInMb = (float)graphicsCardValues[0] / 1024.0f;
#endif

   // enable face culling
   glEnable(GL_CULL_FACE);
   
   // generate buffers
   createBuffers();

   // enable vertex attributes
   glEnableVertexAttribArray((GLuint)VertexAttributes::Position);
   glEnableVertexAttribArray((GLuint)VertexAttributes::Normal);
   glEnableVertexAttribArray((GLuint)VertexAttributes::TextureCoord0);
   glEnableVertexAttribArray((GLuint)VertexAttributes::Color);
   
   // load shaders
   loadShaders();

   // load built-it rendering resources
   loadDefaultRenderingResources();
}

RenderSystemES20::~RenderSystemES20()
{
   releaseBuffers();

   GLuint depthTexture = (GLuint)((GLuintPtrSize)gDepthTexture->getHandler());
   glDeleteTextures(1, &depthTexture);
   GEInvokeDtor(Texture, gDepthTexture);
   Allocator::free(gDepthTexture);
}

void RenderSystemES20::createBuffers()
{
   // buffers for geometry
   for(uint i = 0; i < GeometryGroup::Count; i++)
   {
      uint iVertexBuffer = 0;
      uint iIndexBuffer = 0;

      glGenBuffers(1, &iVertexBuffer);
      glGenBuffers(1, &iIndexBuffer);

      sGPUBufferPairs[i].VertexBuffer = (void*)((uintPtrSize)iVertexBuffer);
      sGPUBufferPairs[i].IndexBuffer = (void*)((uintPtrSize)iIndexBuffer);
      bindBuffers(sGPUBufferPairs[i]);

      glBufferData(GL_ARRAY_BUFFER, kVertexBufferSize, 0, GL_STATIC_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, kIndexBufferSize, 0, GL_STATIC_DRAW);
   }

   // buffer and texture for shadow mapping
   glGenFramebuffers(1, &gFrameBuffer);
   glGenRenderbuffers(1, &gRenderBuffer);
   glBindRenderbuffer(GL_RENDERBUFFER, gRenderBuffer);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kShadowMapSize, kShadowMapSize);
   glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);

   gDepthTexture = Allocator::alloc<Texture>();
   GEInvokeCtor(Texture, gDepthTexture)("Depth", "Texture");
   gDepthTexture->setWidth(kShadowMapSize);
   gDepthTexture->setHeight(kShadowMapSize);
   
   GLuint iDepthTexture;
   glGenTextures(1, &iDepthTexture);
   gDepthTexture->setHandler((void*)((uintPtrSize)iDepthTexture));

   glBindTexture(GL_TEXTURE_2D, iDepthTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kShadowMapSize, kShadowMapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, iDepthTexture, 0);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gRenderBuffer);
   GEAssert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSystemES20::releaseBuffers()
{
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   for(uint i = 0; i < GeometryGroup::Count; i++)
   {
      GLuint iVertexBuffer = (GLuint)((uintPtrSize)sGPUBufferPairs[i].VertexBuffer);
      GLuint iIndexBuffer = (GLuint)((uintPtrSize)sGPUBufferPairs[i].IndexBuffer);

      glDeleteBuffers(1, &iVertexBuffer);
      glDeleteBuffers(1, &iIndexBuffer);
   }

   glDeleteBuffers(1, &gFrameBuffer);
}

void RenderSystem::bindBuffers(const GPUBufferPair& sBuffers)
{
   if(gCurrentVertexBuffer != sBuffers.VertexBuffer)
   {
      gCurrentVertexBuffer = sBuffers.VertexBuffer;
      glBindBuffer(GL_ARRAY_BUFFER, (GLuint)((uintPtrSize)gCurrentVertexBuffer));
   }

   if(gCurrentIndexBuffer != sBuffers.IndexBuffer)
   {
      gCurrentIndexBuffer = sBuffers.IndexBuffer;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)((uintPtrSize)gCurrentIndexBuffer));
   }
}

void RenderSystem::loadTexture(PreloadedTexture* cPreloadedTexture)
{
   GLuint iTexture;
   glGenTextures(1, &iTexture);
   glBindTexture(GL_TEXTURE_2D, iTexture);
   pBoundTexture[(uint)TextureSlot::Diffuse] = cPreloadedTexture->Tex;
   
   // setup texture parameters
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   switch(cPreloadedTexture->Tex->getWrapMode())
   {
   case TextureWrapMode::Clamp:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      break;
   case TextureWrapMode::Repeat:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      break;
   default:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   }

   if(cPreloadedTexture->Data->getFormat() == ImageData::Format::Raw)
   {
      const GLenum glFormat = cPreloadedTexture->Data->getBytesPerPixel() == 4
         ? GL_RGBA
         : GL_RGB;
      glTexImage2D(GL_TEXTURE_2D, 0, glFormat,
         cPreloadedTexture->Data->getWidth(), cPreloadedTexture->Data->getHeight(),
         0, glFormat, GL_UNSIGNED_BYTE, cPreloadedTexture->Data->getData());
   }
   else
   {
      GLenum glFormat = GL_RGBA;
      GLsizei glImageSize = 0;

      const GLsizei glWidth = (GLsizei)cPreloadedTexture->Data->getWidth();
      const GLsizei glHeight = (GLsizei)cPreloadedTexture->Data->getHeight();

#if GL_EXT_texture_compression_s3tc
      if(cPreloadedTexture->Data->getFormat() == ImageData::Format::DDS_DXT1)
      {
         glFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
         glImageSize = ((glWidth + 3) / 4) * ((glHeight + 3) / 4) * 8;
      }
      else if(cPreloadedTexture->Data->getFormat() == ImageData::Format::DDS_DXT3)
      {
         glFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
         glImageSize = ((glWidth + 3) / 4) * ((glHeight + 3) / 4) * 16;
      }
      else if(cPreloadedTexture->Data->getFormat() == ImageData::Format::DDS_DXT5)
      {
         glFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
         glImageSize = ((glWidth + 3) / 4) * ((glHeight + 3) / 4) * 16;
      }
#endif

      glCompressedTexImage2D(GL_TEXTURE_2D, 0, glFormat, glWidth, glHeight,
         0, glImageSize, cPreloadedTexture->Data->getData());
   }

   cPreloadedTexture->Data->unload();

   GEInvokeDtor(ImageData, cPreloadedTexture->Data);
   Allocator::free(cPreloadedTexture->Data);

   cPreloadedTexture->Data = nullptr;
   cPreloadedTexture->Tex->setHandler((void*)((uintPtrSize)iTexture));
}

void RenderSystem::unloadTexture(Texture* pTexture)
{
   GLuint iTexture = (GLuint)((GLuintPtrSize)pTexture->getHandler());
   glBindTexture(GL_TEXTURE_2D, 0);
   glDeleteTextures(1, &iTexture);
   pTexture->setHandler(nullptr);
}

void RenderSystem::loadRenderingData(const GeometryData* pData, GPUBufferPair& pBuffers, uint32_t pIndexSize)
{
   GEProfilerMarker("RenderSystem::loadRenderingData()");

   GEAssert(pData->VertexStride == pBuffers.VertexStride);

   const uint32_t vertexDataSize = pData->NumVertices * pData->VertexStride;
   const uint32_t indicesSize = pData->NumIndices * pIndexSize;

   void* mappedIndices = nullptr;

   if(pIndexSize == 4u)
   {
      const uint32_t baseVertexIndex = pBuffers.CurrentVertexBufferOffset / pData->VertexStride;
      gMappedIndices32.clear();

      uint16_t* currentIndex = pData->Indices;

      for(uint32_t i = 0u; i < pData->NumIndices; i++, currentIndex++)
      {
         gMappedIndices32.push_back(*currentIndex + baseVertexIndex);
      }

      mappedIndices = &gMappedIndices32[0];
   }
   else
   {
      const uint16_t baseVertexIndex = (uint16_t)(pBuffers.CurrentVertexBufferOffset / pData->VertexStride);
      gMappedIndices16.clear();

      uint16_t* currentIndex = pData->Indices;

      for(uint32_t i = 0u; i < pData->NumIndices; i++, currentIndex++)
      {
         gMappedIndices16.push_back((ushort)(*currentIndex + baseVertexIndex));
      }

      mappedIndices = &gMappedIndices16[0];
   }

   bindBuffers(pBuffers);

   glBufferSubData(GL_ARRAY_BUFFER, pBuffers.CurrentVertexBufferOffset, vertexDataSize, pData->VertexData);
   glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, pBuffers.CurrentIndexBufferOffset, indicesSize, mappedIndices);

   pBuffers.CurrentVertexBufferOffset += vertexDataSize;
   pBuffers.CurrentIndexBufferOffset += indicesSize;
}

void RenderSystem::loadShaders()
{
   ContentData cShadersData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Shaders", "Shaders", "xml", &cShadersData);

      pugi::xml_document xml;
      xml.load_buffer(cShadersData.getData(), cShadersData.getDataSize());
      const pugi::xml_node& xmlShaders = xml.child("ShaderProgramList");

      for(const pugi::xml_node& xmlShader : xmlShaders.children("ShaderProgram"))
      {
         const char* sShaderName = xmlShader.attribute("name").value();

         ShaderProgramES20* cShaderProgram = Allocator::alloc<ShaderProgramES20>();
         GEInvokeCtor(ShaderProgramES20, cShaderProgram)(sShaderName);

         cShaderProgram->loadFromXml(xmlShader);

         cShaderProgram->ID = glCreateProgram();
         cShaderProgram->Status = 0;
         cShaderProgram->VS = Allocator::alloc<VertexShader>();
         GEInvokeCtor(VertexShader, cShaderProgram->VS)(cShaderProgram);
         cShaderProgram->FS = Allocator::alloc<FragmentShader>();
         GEInvokeCtor(FragmentShader, cShaderProgram->FS)(cShaderProgram);

         static_cast<RenderSystemES20*>(this)->attachShaders(cShaderProgram);

         mShaderPrograms.add(cShaderProgram);
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Shaders", "Shaders.glsl", "ge", &cShadersData);
      ContentDataMemoryBuffer sMemoryBuffer(cShadersData);
      std::istream sStream(&sMemoryBuffer);

      uint iShadersCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();
      GESTLVector(char) vShaderCode;

      for(uint i = 0; i < iShadersCount; i++)
      {
         ObjectName cShaderName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();

         ShaderProgramES20* cShaderProgram = Allocator::alloc<ShaderProgramES20>();
         GEInvokeCtor(ShaderProgramES20, cShaderProgram)(cShaderName);

         cShaderProgram->loadFromStream(sStream);

         uint iShaderDataSize = Value::fromStream(ValueType::UInt, sStream).getAsUInt();
         vShaderCode.resize(iShaderDataSize + 1);
         sStream.read(&vShaderCode[0], iShaderDataSize);
         vShaderCode[iShaderDataSize] = '\0';

         for(uint j = 0; j < iShaderDataSize; j++)
            vShaderCode[j] += 128;

         cShaderProgram->ID = glCreateProgram();
         cShaderProgram->Status = 0;
         cShaderProgram->VS = Allocator::alloc<VertexShader>();
         GEInvokeCtor(VertexShader, cShaderProgram->VS)(&vShaderCode[0], iShaderDataSize);

         iShaderDataSize = Value::fromStream(ValueType::UInt, sStream).getAsUInt();
         vShaderCode.resize(iShaderDataSize + 1);
         sStream.read(&vShaderCode[0], iShaderDataSize);
         vShaderCode[iShaderDataSize] = '\0';

         for(uint j = 0; j < iShaderDataSize; j++)
            vShaderCode[j] += 128;

         cShaderProgram->FS = Allocator::alloc<FragmentShader>();
         GEInvokeCtor(FragmentShader, cShaderProgram->FS)(&vShaderCode[0], iShaderDataSize);

         static_cast<RenderSystemES20*>(this)->attachShaders(cShaderProgram);

         mShaderPrograms.add(cShaderProgram);
      }
   }
}

void RenderSystemES20::attachShaders(ShaderProgramES20* cProgram)
{
   // attach shaders to the program
   glAttachShader(cProgram->ID, cProgram->VS->getID());
   glAttachShader(cProgram->ID, cProgram->FS->getID());
   
   // bind attributes
   if(GEHasFlag(cProgram->getVertexElements(), VertexElementsBitMask::Position))
      glBindAttribLocation(cProgram->ID, (GLuint)VertexAttributes::Position, "aPosition");

   if(GEHasFlag(cProgram->getVertexElements(), VertexElementsBitMask::Normal))
      glBindAttribLocation(cProgram->ID, (GLuint)VertexAttributes::Normal, "aNormal");

   if(GEHasFlag(cProgram->getVertexElements(), VertexElementsBitMask::TexCoord))
      glBindAttribLocation(cProgram->ID, (GLuint)VertexAttributes::TextureCoord0, "aTextCoord0");

   if(GEHasFlag(cProgram->getVertexElements(), VertexElementsBitMask::Color))
      glBindAttribLocation(cProgram->ID, (GLuint)VertexAttributes::Color, "aColor");
   
   // link program
   linkProgram(cProgram);
   
   if(!checkProgram(cProgram))
   {
      // get info log length
      GLint iLogLength;
      glGetProgramiv(cProgram->ID, GL_INFO_LOG_LENGTH, &iLogLength);
      
      // get info log contents
      GLchar* sLog = Allocator::alloc<GLchar>(iLogLength);
      glGetProgramInfoLog(cProgram->ID, iLogLength, NULL, sLog);
      
      // show info log
      Log::log(LogType::Error, "Program '%s': linking error\n%s", cProgram->getName().getString(), sLog);
      
      Allocator::free(sLog);
      exit(1);
   }
   
   // get uniforms location
   getUniformsLocation(cProgram);
}

void RenderSystemES20::linkProgram(ShaderProgramES20* cProgram)
{
   glLinkProgram(cProgram->ID);
   glGetProgramiv(cProgram->ID, GL_LINK_STATUS, &(cProgram->Status));
}

bool RenderSystemES20::checkProgram(ShaderProgramES20* cProgram)
{
   return (cProgram->Status != 0);
}

void RenderSystemES20::getUniformsLocation(ShaderProgramES20* cProgram)
{
   // transform uniforms
   cProgram->setUniformLocation((uint)Uniforms::WorldViewProjectionMatrix, glGetUniformLocation(cProgram->ID, "uWorldViewProjectionMatrix"));
   cProgram->setUniformLocation((uint)Uniforms::WorldMatrix, glGetUniformLocation(cProgram->ID, "uWorldMatrix"));
   cProgram->setUniformLocation((uint)Uniforms::InverseTransposeWorldMatrix, glGetUniformLocation(cProgram->ID, "uInverseTransposeWorldMatrix"));
   cProgram->setUniformLocation((uint)Uniforms::LightWorldViewProjectionMatrix, glGetUniformLocation(cProgram->ID, "uDepthMVP"));
   cProgram->setUniformLocation((uint)Uniforms::ViewProjectionMatrix, glGetUniformLocation(cProgram->ID, "uViewProjectionMatrix"));
   
   // material uniforms
   cProgram->setUniformLocation((uint)Uniforms::DiffuseColor, glGetUniformLocation(cProgram->ID, "uDiffuseColor"));
   cProgram->setUniformLocation((uint)Uniforms::SpecularColor, glGetUniformLocation(cProgram->ID, "uSpecularColor"));
   cProgram->setUniformLocation((uint)Uniforms::DiffuseTexture, glGetUniformLocation(cProgram->ID, "uDiffuseTexture"));
   
   // lighting uniforms
   cProgram->setUniformLocation((uint)Uniforms::ShadowTexture, glGetUniformLocation(cProgram->ID, "uShadowTexture"));
   cProgram->setUniformLocation((uint)Uniforms::AmbientLightColor, glGetUniformLocation(cProgram->ID, "uAmbientLightColor"));
   cProgram->setUniformLocation((uint)Uniforms::LightColor, glGetUniformLocation(cProgram->ID, "uLightColor"));
   cProgram->setUniformLocation((uint)Uniforms::EyePosition, glGetUniformLocation(cProgram->ID, "uEyePosition"));
   cProgram->setUniformLocation((uint)Uniforms::LightType, glGetUniformLocation(cProgram->ID, "uLightType"));
   cProgram->setUniformLocation((uint)Uniforms::LightPosition, glGetUniformLocation(cProgram->ID, "uLightPosition"));
   cProgram->setUniformLocation((uint)Uniforms::Attenuation, glGetUniformLocation(cProgram->ID, "uAttenuation"));
   cProgram->setUniformLocation((uint)Uniforms::LightDirection, glGetUniformLocation(cProgram->ID, "uLightDirection"));
   cProgram->setUniformLocation((uint)Uniforms::SpotAngle, glGetUniformLocation(cProgram->ID, "uSpotAngle"));
   cProgram->setUniformLocation((uint)Uniforms::ShadowIntensity, glGetUniformLocation(cProgram->ID, "uShadowIntensity"));

   // custom parameters
   cProgram->setUniformLocation((uint)Uniforms::VertexParameters, glGetUniformLocation(cProgram->ID, "uVertexParameters"));
   cProgram->setUniformLocation((uint)Uniforms::FragmentParameters, glGetUniformLocation(cProgram->ID, "uFragmentParameters"));
}

void RenderSystemES20::setVertexDeclaration(const RenderOperation& cRenderOperation)
{
   const int vertexStride = cRenderOperation.mData->VertexStride;

   ShaderProgramES20* cShaderProgram =
      static_cast<ShaderProgramES20*>(mShaderPrograms.get(cRenderOperation.mRenderMaterialPass->getMaterial()->getShaderProgram()));
   GEAssert(cShaderProgram);

   uintPtrSize offset = 0u;

   if(GEHasFlag(cShaderProgram->getVertexElements(), VertexElementsBitMask::Position))
   {
      glVertexAttribPointer((GLuint)VertexAttributes::Position, 3, GL_FLOAT, GL_FALSE, vertexStride, (void*)offset);
      offset += 3u * sizeof(float);
   }
   if(GEHasFlag(cShaderProgram->getVertexElements(), VertexElementsBitMask::Normal))
   {
      glVertexAttribPointer((GLuint)VertexAttributes::Normal, 3, GL_FLOAT, GL_TRUE, vertexStride, (void*)offset);
      offset += 3u * sizeof(float);
   }
   if(GEHasFlag(cShaderProgram->getVertexElements(), VertexElementsBitMask::Color))
   {
      glVertexAttribPointer((GLuint)VertexAttributes::Color, 4, GL_FLOAT, GL_FALSE, vertexStride, (void*)offset);
      offset += 4u * sizeof(float);
   }
   if(GEHasFlag(cShaderProgram->getVertexElements(), VertexElementsBitMask::TexCoord))
   {
      glVertexAttribPointer((GLuint)VertexAttributes::TextureCoord0, 2, GL_FLOAT, GL_FALSE, vertexStride, (void*)offset);
      offset += 2u * sizeof(float);
   }
}

void RenderSystem::bindTexture(TextureSlot eSlot, const Texture* cTexture)
{
   GEAssert((uint)eSlot < (uint)TextureSlot::Count);

   glActiveTexture(GL_TEXTURE0 + (GLuint)eSlot);

   if(pBoundTexture[(uint)eSlot] != cTexture)
   {
      glBindTexture(GL_TEXTURE_2D, (GLuint)((GLuintPtrSize)cTexture->getHandler()));
      pBoundTexture[(uint)eSlot] = const_cast<Texture*>(cTexture);
   }
}

void RenderSystem::useShaderProgram(const Core::ObjectName& cName)
{
   if(iActiveProgram == cName.getID())
      return;

   const ShaderProgramES20* cShaderProgram = static_cast<ShaderProgramES20*>(mShaderPrograms.get(cName));
   GEAssert(cShaderProgram);
   glUseProgram(cShaderProgram->ID);
   iActiveProgram = cName.getID();
   gActiveProgram = const_cast<ShaderProgramES20*>(cShaderProgram);

   setDepthBufferMode(cShaderProgram->getDepthBufferMode());
   setCullingMode(cShaderProgram->getCullingMode());

   glUniform4fv(gActiveProgram->getUniformLocation((uint)Uniforms::AmbientLightColor), 1, &cAmbientLightColor.Red);
}

void RenderSystem::renderShadowMap()
{
   if(vLightsToRender.empty())
      return;

   ComponentLight* cLight = vLightsToRender[0];

   if(cLight->getLightType() != LightType::Directional)
      return;

   GLint glDefaultFrameBuffer = 0;
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &glDefaultFrameBuffer);
   
   GLint glDefaultViewport[4];
   glGetIntegerv(GL_VIEWPORT, glDefaultViewport);
   
   glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);
   glViewport(0, 0, kShadowMapSize, kShadowMapSize);

   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   calculateLightViewProjectionMatrix(cLight);

   if(!vShadowedMeshesToRender.empty())
   {
      useShaderProgram(kShadowMapSolidProgram);

      GESTLVector(RenderOperation)::const_iterator it = vShadowedMeshesToRender.begin();

      for(; it != vShadowedMeshesToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;

         // set uniform
         Matrix4 matLightWVP;
         Matrix4Multiply(matLightViewProjection, sRenderOperation.mWorldTransform, &matLightWVP);
         glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::LightWorldViewProjectionMatrix), 1, 0, matLightWVP.m);

         // bind buffers
         GESTLMap(uint, GeometryRenderInfo)* cGeometryRegistry = nullptr;

         if(sRenderOperation.isStatic())
         {
            cGeometryRegistry = &mStaticGeometryToRender;
            bindBuffers(sGPUBufferPairs[GeometryGroup::MeshStatic]);
         }
         else
         {
            cGeometryRegistry = &mDynamicGeometryToRender;
            bindBuffers(sGPUBufferPairs[GeometryGroup::MeshDynamic]);
         }

         // set vertex declaration
         const int iVertexStride = sRenderOperation.mData->VertexStride;
         glVertexAttribPointer((GLuint)VertexAttributes::Position, 3, GL_FLOAT, GL_FALSE, iVertexStride, 0);

         // draw
         std::map<uint, GeometryRenderInfo>::const_iterator itInfo = cGeometryRegistry->find(sRenderOperation.mGeometryID);
         const GeometryRenderInfo& sGeometryInfo = itInfo->second;
         char* pOffset = (char*)((uintPtrSize)sGeometryInfo.mIndexBufferOffset);

         glDrawElements(GL_TRIANGLES, sRenderOperation.mData->NumIndices, GL_UNSIGNED_INT, pOffset);
      }
   }

   if(!vShadowedParticlesToRender.empty())
   {
      useShaderProgram(kShadowMapAlphaProgram);

      GESTLVector(RenderOperation)::const_iterator it = vShadowedParticlesToRender.begin();

      for(; it != vShadowedParticlesToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;

         // set uniform
         glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::LightWorldViewProjectionMatrix), 1, 0, matLightViewProjection.m);

         // bind diffuse texture
         if(sRenderOperation.mRenderMaterialPass->getMaterial()->getDiffuseTexture())
         {
            bindTexture(TextureSlot::Diffuse, sRenderOperation.mRenderMaterialPass->getMaterial()->getDiffuseTexture());
            glUniform1i(gActiveProgram->getUniformLocation((uint)Uniforms::DiffuseTexture), 0);
         }

         // bind buffers
         bindBuffers(sGPUBufferPairs[GeometryGroup::Particles]);

         // set vertex declaration
         static_cast<RenderSystemES20*>(this)->setVertexDeclaration(sRenderOperation);

         // draw
         std::map<uint, GeometryRenderInfo>::const_iterator itInfo = mDynamicGeometryToRender.find(sRenderOperation.mGeometryID);
         const GeometryRenderInfo& sGeometryInfo = itInfo->second;
         char* pOffset = (char*)((uintPtrSize)sGeometryInfo.mIndexBufferOffset);

         glDrawElements(GL_TRIANGLES, sRenderOperation.mData->NumIndices, GL_UNSIGNED_INT, pOffset);
      }
   }
   
   glBindFramebuffer(GL_FRAMEBUFFER, glDefaultFrameBuffer);
   glViewport(glDefaultViewport[0], glDefaultViewport[1], glDefaultViewport[2], glDefaultViewport[3]);
}

void RenderSystem::renderBegin()
{
   glDepthMask(GL_TRUE);
   glClearColor(cBackgroundColor.Red, cBackgroundColor.Green, cBackgroundColor.Blue, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   iDrawCalls = 0u;
}

void RenderSystem::render(const RenderOperation& sRenderOperation)
{
   // set uniform values for the shaders
   const Matrix4& mViewProjection = GEHasFlag(sRenderOperation.mFlags, RenderOperationFlags::RenderThroughActiveCamera)
      ? cActiveCamera->getViewProjectionMatrix()
      : mat2DViewProjection;
   glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::ViewProjectionMatrix), 1, 0, mViewProjection.m);

   if(GEHasFlag(sRenderOperation.mFlags, RenderOperationFlags::RenderThroughActiveCamera))
   {
      calculate3DTransformMatrix(sRenderOperation.mWorldTransform);
   }
   else
   {
      calculate2DTransformMatrix(sRenderOperation.mWorldTransform);
   }

   glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::WorldMatrix), 1, 0, sRenderOperation.mWorldTransform.m);

   if(GEHasFlag(sRenderOperation.mFlags, RenderOperationFlags::LightingSupport))
   {
      calculate3DInverseTransposeMatrix(sRenderOperation.mWorldTransform);

      glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::InverseTransposeWorldMatrix), 1, 0, matModelInverseTranspose.m);

      if(GEHasFlag(sRenderOperation.mFlags, RenderOperationFlags::BindShadowMap))
      {
         Matrix4 matLightWVP;
         Matrix4Multiply(matLightViewProjection, sRenderOperation.mWorldTransform, &matLightWVP);
         glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::LightWorldViewProjectionMatrix), 1, 0, matLightWVP.m);
      }

      glUniform4fv(gActiveProgram->getUniformLocation((uint)Uniforms::AmbientLightColor), 1, &cAmbientLightColor.Red);
      glUniform3fv(gActiveProgram->getUniformLocation((uint)Uniforms::EyePosition), 1, &cActiveCamera->getTransform()->getPosition().X);
   }

   glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::WorldViewProjectionMatrix), 1, 0, matModelViewProjection.m);

   MaterialPass* cMaterialPass = sRenderOperation.mRenderMaterialPass;
   Material* cMaterial = cMaterialPass->getMaterial();
   Color cDiffuseColor = cMaterial->getDiffuseColor() * sRenderOperation.mColor;

   glUniform4fv(gActiveProgram->getUniformLocation((uint)Uniforms::DiffuseColor), 1, &cDiffuseColor.Red);
   glUniform4fv(gActiveProgram->getUniformLocation((uint)Uniforms::SpecularColor), 1, &cMaterial->getSpecularColor().Red);

   if(GEHasFlag(sRenderOperation.mFlags, RenderOperationFlags::LightingSupport))
   {
      if(vLightsToRender.empty())
      {
         glUniform1i(gActiveProgram->getUniformLocation((uint)Uniforms::LightType), 0);
         glUniform4f(gActiveProgram->getUniformLocation((uint)Uniforms::LightColor), 0.0f, 0.0f, 0.0f, 1.0f);
      }
      else
      {
         ComponentLight* cLight = vLightsToRender[0];
         Vector3 vLightDirection = cLight->getDirection();

         glUniform1i(gActiveProgram->getUniformLocation((uint)Uniforms::LightType), (GE::uint)cLight->getLightType());
         glUniform4fv(gActiveProgram->getUniformLocation((uint)Uniforms::LightColor), 1, &cLight->getColor().Red);
         glUniform3fv(gActiveProgram->getUniformLocation((uint)Uniforms::LightPosition), 1, &cLight->getTransform()->getPosition().X);
         glUniform3fv(gActiveProgram->getUniformLocation((uint)Uniforms::LightDirection), 1, &vLightDirection.X);
         glUniform1f(gActiveProgram->getUniformLocation((uint)Uniforms::Attenuation), cLight->getLinearAttenuation());
         glUniform1f(gActiveProgram->getUniformLocation((uint)Uniforms::SpotAngle), cLight->getSpotAngle());
         glUniform1f(gActiveProgram->getUniformLocation((uint)Uniforms::ShadowIntensity), cLight->getShadowIntensity());
      }
   }

   if(sRenderOperation.mDiffuseTexture)
   {
      bindTexture(TextureSlot::Diffuse, sRenderOperation.mDiffuseTexture);
      glUniform1i(gActiveProgram->getUniformLocation((uint)Uniforms::DiffuseTexture), (uint)TextureSlot::Diffuse);
   }

   if(GEHasFlag(sRenderOperation.mFlags, RenderOperationFlags::BindShadowMap))
   {
      bindTexture(TextureSlot::ShadowMap, gDepthTexture);
      glUniform1i(gActiveProgram->getUniformLocation((uint)Uniforms::ShadowTexture), (uint)TextureSlot::ShadowMap);
   }

   if(cMaterialPass->hasVertexParameters())
   {
      glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::VertexParameters), 1, 0, (const GLfloat*)cMaterialPass->getConstantBufferDataVertex());
   }

   if(cMaterialPass->hasFragmentParameters())
   {
      glUniformMatrix4fv(gActiveProgram->getUniformLocation((uint)Uniforms::FragmentParameters), 1, 0, (const GLfloat*)cMaterialPass->getConstantBufferDataFragment());
   }

   // bind buffers and get index offset
   GESTLMap(uint, GeometryRenderInfo)* mGeometryToRenderMap = sRenderOperation.isStatic()
      ? &mStaticGeometryToRender
      : &mDynamicGeometryToRender;

   bindBuffers(sGPUBufferPairs[sRenderOperation.mGroup]);
   std::map<uint, GeometryRenderInfo>::const_iterator it = mGeometryToRenderMap->find(sRenderOperation.mGeometryID);
   const GeometryRenderInfo& sGeometryInfo = it->second;
   char* pOffset = (char*)((uintPtrSize)sGeometryInfo.mIndexBufferOffset);

   // set vertex declaration
   static_cast<RenderSystemES20*>(this)->setVertexDeclaration(sRenderOperation);

   // draw
   const GLenum glIndexType = sRenderOperation.mVertexIndexSize == 4u
      ? GL_UNSIGNED_INT
      : GL_UNSIGNED_SHORT;
   glDrawElements(GL_TRIANGLES, sRenderOperation.mData->NumIndices, glIndexType, pOffset);
}

void RenderSystem::renderEnd()
{
}

void RenderSystem::createBitmapTexture(const Core::ObjectName& pName, size_t pWidth, size_t pHeight)
{
   Texture* bitmapTexture = Allocator::alloc<Texture>();
   GEInvokeCtor(Texture, bitmapTexture)(pName, "Bitmaps");
   bitmapTexture->setWidth(pWidth);
   bitmapTexture->setHeight(pHeight);
   mTextures.add(bitmapTexture);
   
   GLuint glBitmapTexture;
   glGenTextures(1, &glBitmapTexture);
   bitmapTexture->setHandler((void*)((uintPtrSize)glBitmapTexture));

   glBindTexture(GL_TEXTURE_2D, glBitmapTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
      (GLsizei)pWidth, (GLsizei)pHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void RenderSystem::updateBitmapTexture(const Core::ObjectName& pName, const char* pBitmapData)
{
   Texture* bitmapTexture = mTextures.get(pName);

   if(bitmapTexture)
   {
      bindTexture(TextureSlot::Diffuse, bitmapTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
         (GLsizei)bitmapTexture->getWidth(), (GLsizei)bitmapTexture->getHeight(),
         0, GL_RGBA, GL_UNSIGNED_BYTE, pBitmapData);
   }
}

void RenderSystem::setBlendingMode(BlendingMode Mode)
{
   if(eBlendingMode == Mode)
      return;

   eBlendingMode = Mode;

   switch(Mode)
   {
   case BlendingMode::None:
      glDisable(GL_BLEND);
      break;

   case BlendingMode::Alpha:
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      break;

   case BlendingMode::Additive:
      glBlendFunc(GL_ONE, GL_ONE);
      glEnable(GL_BLEND);
      break;

   default:
      break;
   }
}

void RenderSystem::setDepthBufferMode(DepthBufferMode Mode)
{
   if(eDepthBufferMode == Mode)
      return;

   eDepthBufferMode = Mode;

   switch(Mode)
   {
   case DepthBufferMode::NoDepth:
      glDisable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      break;

   case DepthBufferMode::TestOnly:
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      glDepthFunc(GL_LEQUAL);
      break;

   case DepthBufferMode::TestAndWrite:
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);
      break;

   default:
      break;
   }
}

void RenderSystem::setCullingMode(CullingMode Mode)
{
   if(eCullingMode == Mode)
      return;

   eCullingMode = Mode;

   switch(Mode)
   {
   case CullingMode::Back:
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      break;

   case CullingMode::Front:
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      break;

   case CullingMode::None:
      glDisable(GL_CULL_FACE);
      break;

   default:
      break;
   }
}
