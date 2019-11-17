
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
#include "Entities/GEComponentUIElement.h"
#include "pugixml/pugixml.hpp"
#include "GEOpenGLES20.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Entities;
using namespace GE::Rendering;

// buffers
void* iCurrentVertexBuffer = 0;
void* iCurrentIndexBuffer = 0;

GESTLVector(ushort) vMappedIndicesUshort;
GESTLVector(uint) vMappedIndicesUint;

// shadow mapping
uint iFrameBuffer = 0;
uint iRenderBuffer = 0;
Texture* cDepthTexture = 0;

// shaders
ShaderProgramES20* cActiveProgram = 0;

const ObjectName _Mesh_ = ObjectName("Mesh");
const ObjectName _Label_ = ObjectName("Label");
const ObjectName _ParticleSystem_ = ObjectName("ParticleSystem");

RenderSystemES20::RenderSystemES20()
   : RenderSystem(nullptr, false, Device::getTouchPadWidth(), Device::getTouchPadHeight())
{  
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

   GLuint iDepthTexture = (GLuint)((GLuintPtrSize)cDepthTexture->getHandler());
   glDeleteTextures(1, &iDepthTexture);
   GEInvokeDtor(Texture, cDepthTexture);
   Allocator::free(cDepthTexture);
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

      glBufferData(GL_ARRAY_BUFFER, VertexBufferSize, 0, GL_STATIC_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize, 0, GL_STATIC_DRAW);
   }

   // buffer and texture for shadow mapping
   glGenFramebuffers(1, &iFrameBuffer);
   glGenRenderbuffers(1, &iRenderBuffer);
   glBindRenderbuffer(GL_RENDERBUFFER, iRenderBuffer);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ShadowMapSize, ShadowMapSize);
   glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer);

   cDepthTexture = Allocator::alloc<Texture>();
   GEInvokeCtor(Texture, cDepthTexture)("Depth", "Texture");
   cDepthTexture->setWidth(ShadowMapSize);
   cDepthTexture->setHeight(ShadowMapSize);
   
   GLuint iDepthTexture;
   glGenTextures(1, &iDepthTexture);
   cDepthTexture->setHandler((void*)((uintPtrSize)iDepthTexture));

   glBindTexture(GL_TEXTURE_2D, iDepthTexture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ShadowMapSize, ShadowMapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, iDepthTexture, 0);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iRenderBuffer);
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

   glDeleteBuffers(1, &iFrameBuffer);
}

void RenderSystem::bindBuffers(const GPUBufferPair& sBuffers)
{
   if(iCurrentVertexBuffer != sBuffers.VertexBuffer)
   {
      iCurrentVertexBuffer = sBuffers.VertexBuffer;
      glBindBuffer(GL_ARRAY_BUFFER, (GLuint)((uintPtrSize)iCurrentVertexBuffer));
   }

   if(iCurrentIndexBuffer != sBuffers.IndexBuffer)
   {
      iCurrentIndexBuffer = sBuffers.IndexBuffer;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)((uintPtrSize)iCurrentIndexBuffer));
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

   cPreloadedTexture->Data = 0;
   cPreloadedTexture->Tex->setHandler((void*)((uintPtrSize)iTexture));
}

void RenderSystem::unloadTexture(Texture* pTexture)
{
   GLuint iTexture = (GLuint)((GLuintPtrSize)pTexture->getHandler());
   glBindTexture(GL_TEXTURE_2D, 0);
   glDeleteTextures(1, &iTexture);
   pTexture->setHandler(nullptr);
}

void RenderSystem::loadRenderingData(const GeometryData& sData, GPUBufferPair& sBuffers, uint iIndexSize)
{
   GEProfilerMarker("RenderSystem::loadRenderingData()");

   GEAssert(sData.VertexStride == sBuffers.VertexStride);

   uint iVertexDataSize = sData.NumVertices * sData.VertexStride;
   uint iIndicesSize = sData.NumIndices * iIndexSize;

   void* pMappedIndices = 0;

   if(iIndexSize == 4)
   {
      uint iBaseVertexIndex = sBuffers.CurrentVertexBufferOffset / sData.VertexStride;
      vMappedIndicesUint.clear();

      ushort* pCurrentIndex = sData.Indices;

      for(uint i = 0; i < sData.NumIndices; i++, pCurrentIndex++)
         vMappedIndicesUint.push_back(*pCurrentIndex + iBaseVertexIndex);

      pMappedIndices = &vMappedIndicesUint[0];
   }
   else
   {
      ushort iBaseVertexIndex = (ushort)(sBuffers.CurrentVertexBufferOffset / sData.VertexStride);
      vMappedIndicesUshort.clear();

      ushort* pCurrentIndex = sData.Indices;

      for(uint i = 0; i < sData.NumIndices; i++, pCurrentIndex++)
         vMappedIndicesUshort.push_back((ushort)(*pCurrentIndex + iBaseVertexIndex));

      pMappedIndices = &vMappedIndicesUshort[0];
   }

   bindBuffers(sBuffers);

   glBufferSubData(GL_ARRAY_BUFFER, sBuffers.CurrentVertexBufferOffset, iVertexDataSize, sData.VertexData);
   glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sBuffers.CurrentIndexBufferOffset, iIndicesSize, pMappedIndices);

   sBuffers.CurrentVertexBufferOffset += iVertexDataSize;
   sBuffers.CurrentIndexBufferOffset += iIndicesSize;
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
   const int iVertexStride = cRenderOperation.Data.VertexStride;

   ShaderProgramES20* cShaderProgram =
      static_cast<ShaderProgramES20*>(mShaderPrograms.get(cRenderOperation.RenderMaterialPass->getMaterial()->getShaderProgram()));
   GEAssert(cShaderProgram);

   uintPtrSize iOffset = 0;

   if(GEHasFlag(cShaderProgram->getVertexElements(), VertexElementsBitMask::Position))
   {
      glVertexAttribPointer((GLuint)VertexAttributes::Position, 3, GL_FLOAT, GL_FALSE, iVertexStride, (void*)iOffset);
      iOffset += 3 * sizeof(float);
   }
   if(GEHasFlag(cShaderProgram->getVertexElements(), VertexElementsBitMask::Normal))
   {
      glVertexAttribPointer((GLuint)VertexAttributes::Normal, 3, GL_FLOAT, GL_TRUE, iVertexStride, (void*)iOffset);
      iOffset += 3 * sizeof(float);
   }
   if(GEHasFlag(cShaderProgram->getVertexElements(), VertexElementsBitMask::Color))
   {
      glVertexAttribPointer((GLuint)VertexAttributes::Color, 4, GL_FLOAT, GL_FALSE, iVertexStride, (void*)iOffset);
      iOffset += 4 * sizeof(float);
   }
   if(GEHasFlag(cShaderProgram->getVertexElements(), VertexElementsBitMask::TexCoord))
   {
      glVertexAttribPointer((GLuint)VertexAttributes::TextureCoord0, 2, GL_FLOAT, GL_FALSE, iVertexStride, (void*)iOffset);
      iOffset += 2 * sizeof(float);
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
   cActiveProgram = const_cast<ShaderProgramES20*>(cShaderProgram);

   setDepthBufferMode(cShaderProgram->getDepthBufferMode());
   setCullingMode(cShaderProgram->getCullingMode());
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
   
   glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer);
   glViewport(0, 0, ShadowMapSize, ShadowMapSize);

   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
#if defined (GE_PLATFORM_ANDROID) || defined (GE_PLATFORM_IOS)
   glClearDepthf(1.0f);
#else
   glClearDepth(1.0);
#endif
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   calculateLightViewProjectionMatrix(cLight);

   if(!vShadowedMeshesToRender.empty())
   {
      useShaderProgram(ShadowMapSolidProgram);

      GESTLVector(RenderOperation)::const_iterator it = vShadowedMeshesToRender.begin();

      for(; it != vShadowedMeshesToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;
         Entity* cEntity = sRenderOperation.Renderable->getOwner();

         // set uniform
         const Matrix4& matModel = cEntity->getComponent<ComponentTransform>()->getGlobalWorldMatrix();
         Matrix4 matLightWVP;
         Matrix4Multiply(matLightViewProjection, matModel, &matLightWVP);
         glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::LightWorldViewProjectionMatrix), 1, 0, matLightWVP.m);

         // bind buffers
         GESTLMap(uint, GeometryRenderInfo)* cGeometryRegistry = 0;

         if(sRenderOperation.Renderable->getGeometryType() == GeometryType::Static)
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
         const int iVertexStride = sRenderOperation.Renderable->getGeometryData().VertexStride;
         glVertexAttribPointer((GLuint)VertexAttributes::Position, 3, GL_FLOAT, GL_FALSE, iVertexStride, 0);

         // draw
         std::map<uint, GeometryRenderInfo>::const_iterator itInfo = cGeometryRegistry->find(cEntity->getFullName().getID());
         const GeometryRenderInfo& sGeometryInfo = itInfo->second;
         char* pOffset = (char*)((uintPtrSize)sGeometryInfo.IndexBufferOffset);

         glDrawElements(GL_TRIANGLES, sRenderOperation.Renderable->getGeometryData().NumIndices, GL_UNSIGNED_INT, pOffset);
      }
   }

   if(!vShadowedParticlesToRender.empty())
   {
      useShaderProgram(ShadowMapAlphaProgram);

      GESTLVector(RenderOperation)::const_iterator it = vShadowedParticlesToRender.begin();

      for(; it != vShadowedParticlesToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;
         Entity* cEntity = sRenderOperation.Renderable->getOwner();

         // set uniform
         glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::LightWorldViewProjectionMatrix), 1, 0, matLightViewProjection.m);

         // bind diffuse texture
         if(sRenderOperation.RenderMaterialPass->getMaterial()->getDiffuseTexture())
         {
            bindTexture(TextureSlot::Diffuse, sRenderOperation.RenderMaterialPass->getMaterial()->getDiffuseTexture());
            glUniform1i(cActiveProgram->getUniformLocation((uint)Uniforms::DiffuseTexture), 0);
         }

         // bind buffers
         bindBuffers(sGPUBufferPairs[GeometryGroup::Particles]);

         // set vertex declaration
         static_cast<RenderSystemES20*>(this)->setVertexDeclaration(sRenderOperation);

         // draw
         std::map<uint, GeometryRenderInfo>::const_iterator itInfo = mDynamicGeometryToRender.find(cEntity->getFullName().getID());
         const GeometryRenderInfo& sGeometryInfo = itInfo->second;
         char* pOffset = (char*)((uintPtrSize)sGeometryInfo.IndexBufferOffset);

         glDrawElements(GL_TRIANGLES, sRenderOperation.Renderable->getGeometryData().NumIndices, GL_UNSIGNED_INT, pOffset);
      }
   }
   
   glBindFramebuffer(GL_FRAMEBUFFER, glDefaultFrameBuffer);
   glViewport(glDefaultViewport[0], glDefaultViewport[1], glDefaultViewport[2], glDefaultViewport[3]);
}

void RenderSystem::renderBegin()
{
   glDepthMask(GL_TRUE);
   glClearColor(cBackgroundColor.Red, cBackgroundColor.Green, cBackgroundColor.Blue, 1.0f);
#if defined (GE_PLATFORM_ANDROID) || defined (GE_PLATFORM_IOS)
   glClearDepthf(1.0f);
#else
   glClearDepth(1.0);
#endif
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   iDrawCalls = 0;
}

void RenderSystem::render(const RenderOperation& sRenderOperation)
{
   MaterialPass* cMaterialPass = sRenderOperation.RenderMaterialPass;
   ComponentRenderable* cRenderable = sRenderOperation.Renderable;
   ComponentMesh* cMesh = 0;

   // set uniform values for the shaders
   const Matrix4& mViewProjection = cRenderable->getRenderingMode() == RenderingMode::_3D
      ? cActiveCamera->getViewProjectionMatrix()
      : mat2DViewProjection;
   glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::ViewProjectionMatrix), 1, 0, mViewProjection.m);

   if(cRenderable->getClassName() == _ParticleSystem_)
   {
      matModelViewProjection = mViewProjection;

      glUniform4fv(cActiveProgram->getUniformLocation((uint)Uniforms::AmbientLightColor), 1, &cAmbientLightColor.Red);
   }
   else
   {
      ComponentTransform* cTransform = cRenderable->getTransform();
      const Matrix4& matModel = cTransform->getGlobalWorldMatrix();

      if(cRenderable->getRenderingMode() == RenderingMode::_2D)
         calculate2DTransformMatrix(matModel);
      else
         calculate3DTransformMatrix(matModel);

      glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::WorldMatrix), 1, 0, matModel.m);

      if(cRenderable->getClassName() == _Mesh_)
      {
         cMesh = static_cast<ComponentMesh*>(cRenderable);
         calculate3DInverseTransposeMatrix(matModel);

         glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::InverseTransposeWorldMatrix), 1, 0, matModelInverseTranspose.m);

         if(GEHasFlag(cMesh->getDynamicShadows(), DynamicShadowsBitMask::Receive))
         {
            Matrix4 matLightWVP;
            Matrix4Multiply(matLightViewProjection, matModel, &matLightWVP);
            glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::LightWorldViewProjectionMatrix), 1, 0, matLightWVP.m);
         }

         glUniform4fv(cActiveProgram->getUniformLocation((uint)Uniforms::AmbientLightColor), 1, &cAmbientLightColor.Red);
         glUniform3fv(cActiveProgram->getUniformLocation((uint)Uniforms::EyePosition), 1, &cActiveCamera->getTransform()->getPosition().X);
      }
   }

   glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::WorldViewProjectionMatrix), 1, 0, matModelViewProjection.m);

   Material* cMaterial = cMaterialPass->getMaterial();
   Color cDiffuseColor = cMaterial->getDiffuseColor() * cRenderable->getColor();

   ComponentUIElement* cUIElement = cRenderable->getOwner()->getComponent<ComponentUIElement>();
   
   if(cUIElement)
      cDiffuseColor.Alpha *= cUIElement->getAlphaInHierarchy();

   glUniform4fv(cActiveProgram->getUniformLocation((uint)Uniforms::DiffuseColor), 1, &cDiffuseColor.Red);
   glUniform4fv(cActiveProgram->getUniformLocation((uint)Uniforms::SpecularColor), 1, &cMaterial->getSpecularColor().Red);

   if(cRenderable->getClassName() == _Mesh_)
   {
      if(vLightsToRender.empty())
      {
         glUniform1i(cActiveProgram->getUniformLocation((uint)Uniforms::LightType), 0);
         glUniform4f(cActiveProgram->getUniformLocation((uint)Uniforms::LightColor), 0.0f, 0.0f, 0.0f, 1.0f);
      }
      else
      {
         ComponentLight* cLight = vLightsToRender[0];
         Vector3 vLightDirection = cLight->getDirection();

         glUniform1i(cActiveProgram->getUniformLocation((uint)Uniforms::LightType), (GE::uint)cLight->getLightType());
         glUniform4fv(cActiveProgram->getUniformLocation((uint)Uniforms::LightColor), 1, &cLight->getColor().Red);
         glUniform3fv(cActiveProgram->getUniformLocation((uint)Uniforms::LightPosition), 1, &cLight->getTransform()->getPosition().X);
         glUniform3fv(cActiveProgram->getUniformLocation((uint)Uniforms::LightDirection), 1, &vLightDirection.X);
         glUniform1f(cActiveProgram->getUniformLocation((uint)Uniforms::Attenuation), cLight->getLinearAttenuation());
         glUniform1f(cActiveProgram->getUniformLocation((uint)Uniforms::SpotAngle), cLight->getSpotAngle());
         glUniform1f(cActiveProgram->getUniformLocation((uint)Uniforms::ShadowIntensity), cLight->getShadowIntensity());
      }
   }

   if(cRenderable->getClassName() == _Label_)
   {
      bindTexture(TextureSlot::Diffuse, static_cast<ComponentLabel*>(cRenderable)->getFont()->getTexture());
      glUniform1i(cActiveProgram->getUniformLocation((uint)Uniforms::DiffuseTexture), (uint)TextureSlot::Diffuse);
   }
   else if(cMaterial->getDiffuseTexture())
   {
      bindTexture(TextureSlot::Diffuse, cMaterial->getDiffuseTexture());
      glUniform1i(cActiveProgram->getUniformLocation((uint)Uniforms::DiffuseTexture), (uint)TextureSlot::Diffuse);
   }

   if(cMesh && GEHasFlag(cMesh->getDynamicShadows(), DynamicShadowsBitMask::Receive))
   {
      bindTexture(TextureSlot::ShadowMap, cDepthTexture);
      glUniform1i(cActiveProgram->getUniformLocation((uint)Uniforms::ShadowTexture), (uint)TextureSlot::ShadowMap);
   }

   if(cMaterialPass->hasVertexParameters())
   {
      glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::VertexParameters), 1, 0, (const GLfloat*)cMaterialPass->getConstantBufferDataVertex());
   }

   if(cMaterialPass->hasFragmentParameters())
   {
      glUniformMatrix4fv(cActiveProgram->getUniformLocation((uint)Uniforms::FragmentParameters), 1, 0, (const GLfloat*)cMaterialPass->getConstantBufferDataFragment());
   }

   // bind buffers and get index offset
   GESTLMap(uint, GeometryRenderInfo)* mGeometryToRenderMap = cRenderable->getGeometryType() == GeometryType::Static
      ? &mStaticGeometryToRender
      : &mDynamicGeometryToRender;

   bindBuffers(sGPUBufferPairs[sRenderOperation.Group]);
   std::map<uint, GeometryRenderInfo>::const_iterator it = mGeometryToRenderMap->find(cRenderable->getOwner()->getFullName().getID());
   const GeometryRenderInfo& sGeometryInfo = it->second;
   char* pOffset = (char*)((uintPtrSize)sGeometryInfo.IndexBufferOffset);

   // set vertex declaration
   static_cast<RenderSystemES20*>(this)->setVertexDeclaration(sRenderOperation);

   // draw
   const GLenum glIndexType = cRenderable->getClassName() == _Mesh_ || cRenderable->getClassName() == _ParticleSystem_
      ? GL_UNSIGNED_INT
      : GL_UNSIGNED_SHORT;
   glDrawElements(GL_TRIANGLES, sRenderOperation.Data.NumIndices, glIndexType, pOffset);
   iDrawCalls++;
}

void RenderSystem::renderEnd()
{
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
