
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Entities
//
//  --- GEComponentSprite.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentSprite.h"
#include "Core/GEAllocator.h"
#include "Core/GEDevice.h"
#include "Core/GEEvents.h"
#include "Rendering/GERenderSystem.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;

//
//  ComponentSprite
//
const ObjectName ComponentSprite::ClassName = ObjectName("Sprite");

ComponentSprite::ComponentSprite(Entity* Owner)
   : ComponentRenderable(Owner)
   , vCenter(0.0f, 0.0f)
   , vSize(1.0f, 1.0f)
   , iLayer(SpriteLayer::GUI)
   , eUVMode(UVMode::Normal)
   , mCenterMode(CenterMode::Absolute)
   , mSizeMode(SizeMode::Absolute)
   , eFullScreenSizeMode(FullScreenSizeMode::None)
   , bVertexDataDirty(true)
{
   mClassNames.push_back(ClassName);

   sGeometryData.NumVertices = 4;
   sGeometryData.VertexStride = (3 + 2) * sizeof(float);
   sGeometryData.VertexData = Allocator::alloc<float>(sGeometryData.VertexStride * sGeometryData.NumVertices);

   sGeometryData.NumIndices = 6;
   sGeometryData.Indices = const_cast<ushort*>(QuadIndices);

#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::connectStaticEventCallback(Events::PropertiesUpdated, this, [this](const EventArgs* args) -> bool
   {
      Serializable* cSerializable = static_cast<Serializable*>(args->Data);

      if(cSerializable == this)
      {
         updateVertexData();
      }

      return false;
   });
   EventHandlingObject::connectStaticEventCallback(Events::RenderingSurfaceChanged, this, [this](const EventArgs* args) -> bool
   {
      if(mSizeMode == SizeMode::Relative || eFullScreenSizeMode != FullScreenSizeMode::None)
      {
         updateVertexData();
      }

      return false;
   });
#endif

   GERegisterProperty(Vector2, Center);
   GERegisterProperty(Vector2, Size);
   GERegisterPropertyEnum(SpriteLayer, Layer);
   GERegisterPropertyEnum(UVMode, UVMode);
   GERegisterPropertyEnum(CenterMode, CenterMode);
   GERegisterPropertyEnum(SizeMode, SizeMode);
   GERegisterPropertyEnum(FullScreenSizeMode, FullScreenSizeMode);
   GERegisterProperty(ObjectName, TextureAtlasName);

#if defined (GE_EDITOR_SUPPORT)
   registerAction("Adjust Height to current Width", [this]
   {
      if(getMaterialPassCount() == 0 || !getMaterialPass(0)->getMaterial())
         return;

      const Texture* cTexture = getMaterialPass(0)->getMaterial()->getDiffuseTexture();

      if(!cTexture)
         return;

      TextureAtlasEntry* cAtlasEntry = cTexture->AtlasUVManager.get(cTextureAtlasName);

      const float fTextureRatio = (float)cTexture->getHeight() / cTexture->getWidth();
      const float fAtlasRatio = (cAtlasEntry->UV.V1 - cAtlasEntry->UV.V0) / (cAtlasEntry->UV.U1 - cAtlasEntry->UV.U0);
      const float fRatio = fTextureRatio * fAtlasRatio;

      mSizeMode = SizeMode::Absolute;
      vSize.Y = vSize.X * fRatio;
      bVertexDataDirty = true;
   });

   registerAction("Adjust Width to current Height", [this]
   {
      if(getMaterialPassCount() == 0 || !getMaterialPass(0)->getMaterial())
         return;

      const Texture* cTexture = getMaterialPass(0)->getMaterial()->getDiffuseTexture();

      if(!cTexture)
         return;

      TextureAtlasEntry* cAtlasEntry = cTexture->AtlasUVManager.get(cTextureAtlasName);

      const float fTextureRatio = (float)cTexture->getWidth() / cTexture->getHeight();
      const float fAtlasRatio = (cAtlasEntry->UV.U1 - cAtlasEntry->UV.U0) / (cAtlasEntry->UV.V1 - cAtlasEntry->UV.V0);
      const float fRatio = fTextureRatio * fAtlasRatio;

      vSize.X = vSize.Y * fRatio;
      bVertexDataDirty = true;
   });
#endif
}

ComponentSprite::~ComponentSprite()
{
#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::disconnectStaticEventCallback(Events::PropertiesUpdated, this);
   EventHandlingObject::disconnectStaticEventCallback(Events::RenderingSurfaceChanged, this);
#endif

   Allocator::free(sGeometryData.VertexData);
   sGeometryData.VertexData = 0;
}

void ComponentSprite::updateVertexData()
{
   // size
   switch(eFullScreenSizeMode)
   {
      case FullScreenSizeMode::None:
         break;

      case FullScreenSizeMode::Stretch:
         {
            vSize = Vector2(2.0f, Device::getAspectRatio() * 2.0f);
         }
         break;

      case FullScreenSizeMode::MatchWidth:
         {
            if(!vMaterialPassList.empty())
            {
               Material* cMaterial = getMaterialPass(0)->getMaterial();

               if(cMaterial && cMaterial->getDiffuseTexture())
               {
                  const Texture* cDiffuseTexture = cMaterial->getDiffuseTexture();
                  const float fWidth = 2.0f;
                  const float fHeight = fWidth * ((float)cDiffuseTexture->getHeight() / cDiffuseTexture->getWidth());
                  vSize = Vector2(fWidth, fHeight);
               }
            }
         }
         break;

      case FullScreenSizeMode::MatchHeight:
         {
            if(!vMaterialPassList.empty())
            {
               Material* cMaterial = getMaterialPass(0)->getMaterial();

               if(cMaterial && cMaterial->getDiffuseTexture())
               {
                  const Texture* cDiffuseTexture = cMaterial->getDiffuseTexture();
                  const float fHeight = Device::getAspectRatio() * 2.0f;
                  const float fWidth = fHeight * ((float)cDiffuseTexture->getWidth() / cDiffuseTexture->getHeight());
                  vSize = Vector2(fWidth, fHeight);
               }
            }
         }
         break;

      default:
         break;
   }

   // position
   float fHalfSizeX = vSize.X * 0.5f;
   float fHalfSizeY = vSize.Y * 0.5f;

   float fCenterX = vCenter.X;
   float fCenterY = vCenter.Y;

   if(mCenterMode == CenterMode::Relative)
   {
      fCenterX *= vSize.X * 0.5f;
      fCenterY *= vSize.Y * 0.5f;
   }

   if(mSizeMode == SizeMode::Relative)
   {
      fHalfSizeY *= Device::getAspectRatio();
      fCenterY *= Device::getAspectRatio();
   }

   float* fVertexData = sGeometryData.VertexData;

   fVertexData[ 0] = -fHalfSizeX - fCenterX;  fVertexData[ 1] = -fHalfSizeY - fCenterY;  fVertexData[ 2] = 0.0f;
   fVertexData[ 5] =  fHalfSizeX - fCenterX;  fVertexData[ 6] = -fHalfSizeY - fCenterY;  fVertexData[ 7] = 0.0f;
   fVertexData[10] = -fHalfSizeX - fCenterX;  fVertexData[11] =  fHalfSizeY - fCenterY;  fVertexData[12] = 0.0f;
   fVertexData[15] =  fHalfSizeX - fCenterX;  fVertexData[16] =  fHalfSizeY - fCenterY;  fVertexData[17] = 0.0f;

   // texture coordinates
   if(vMaterialPassList.empty() || !getMaterialPass(0))
      return;

   Material* cMaterial = getMaterialPass(0)->getMaterial();

   if(!cMaterial)
      return;

#if defined (GE_EDITOR_SUPPORT)
   Property* cTextureAtlasNameProperty = const_cast<Property*>(getProperty("TextureAtlasName"));
   void* pCachedDataPtr = cTextureAtlasNameProperty->DataPtr;
   cTextureAtlasNameProperty->DataPtr = 0;
#endif

   const Texture* cDiffuseTexture = cMaterial->getDiffuseTexture();

   if(!cDiffuseTexture)
      return;

   TextureAtlasEntry* cAtlasEntry = cDiffuseTexture->AtlasUVManager.get(cTextureAtlasName);

   if(!cAtlasEntry)
   {
      cTextureAtlasName = ObjectName::Empty;
   }

   const TextureCoordinates& UV = cAtlasEntry ? cAtlasEntry->UV : cDiffuseTexture->AtlasUV[0].UV;

   switch(eUVMode)
   {
   case UVMode::Normal:
      fVertexData[ 3] = UV.U0;  fVertexData[ 4] = UV.V1;
      fVertexData[ 8] = UV.U1;  fVertexData[ 9] = UV.V1;
      fVertexData[13] = UV.U0;  fVertexData[14] = UV.V0;
      fVertexData[18] = UV.U1;  fVertexData[19] = UV.V0;
      break;

   case UVMode::FlipU:
      fVertexData[ 3] = UV.U1;  fVertexData[ 4] = UV.V1;
      fVertexData[ 8] = UV.U0;  fVertexData[ 9] = UV.V1;
      fVertexData[13] = UV.U1;  fVertexData[14] = UV.V0;
      fVertexData[18] = UV.U0;  fVertexData[19] = UV.V0;
      break;

   case UVMode::FlipV:
      fVertexData[ 3] = UV.U0;  fVertexData[ 4] = UV.V0;
      fVertexData[ 8] = UV.U1;  fVertexData[ 9] = UV.V0;
      fVertexData[13] = UV.U0;  fVertexData[14] = UV.V1;
      fVertexData[18] = UV.U1;  fVertexData[19] = UV.V1;
      break;

   case UVMode::FlipUV:
      fVertexData[ 3] = UV.U1;  fVertexData[ 4] = UV.V0;
      fVertexData[ 8] = UV.U0;  fVertexData[ 9] = UV.V0;
      fVertexData[13] = UV.U1;  fVertexData[14] = UV.V1;
      fVertexData[18] = UV.U0;  fVertexData[19] = UV.V1;
      break;

   default:
      break;
   }

#if defined (GE_EDITOR_SUPPORT)
   cTextureAtlasNameProperty->DataPtr = (void*)cMaterial->getDiffuseTexture()->AtlasUVManager.getObjectRegistry();

   if(cTextureAtlasNameProperty->DataPtr != pCachedDataPtr)
   {
      EventArgs sArgs;
      sArgs.Data = this;
      EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
   }
#endif
}

void ComponentSprite::update()
{
   if(bVertexDataDirty)
   {
      updateVertexData();
      bVertexDataDirty = false;
   }
}

void ComponentSprite::setCenter(const Vector2& Center)
{
   vCenter = Center;
   bVertexDataDirty = true;
}

void ComponentSprite::setSize(const Vector2& Size)
{
   vSize = Size;
   bVertexDataDirty = true;
}

void ComponentSprite::setLayer(SpriteLayer Layer)
{
   iLayer = Layer;
}

void ComponentSprite::setUVMode(UVMode Mode)
{
   if(eUVMode == Mode)
      return;

   eUVMode = Mode;
   bVertexDataDirty = true;
}

void ComponentSprite::setCenterMode(CenterMode pMode)
{
   mCenterMode = pMode;
   bVertexDataDirty = true;
}

void ComponentSprite::setSizeMode(SizeMode pMode)
{
   mSizeMode = pMode;
   bVertexDataDirty = true;
}

void ComponentSprite::setFullScreenSizeMode(FullScreenSizeMode Mode)
{
   eFullScreenSizeMode = Mode;

   if(eFullScreenSizeMode != FullScreenSizeMode::None)
   {
      mSizeMode = SizeMode::Absolute;
   }

   bVertexDataDirty = true;
}

void ComponentSprite::setTextureAtlasName(const Core::ObjectName& AtlasName)
{
   cTextureAtlasName = AtlasName;
   bVertexDataDirty = true;
}

bool ComponentSprite::isOver(const Vector2& ScreenPosition) const
{
   if(eRenderingMode == RenderingMode::_2D &&
      cTransform->getWorldRotation().getQuaternion().isIdentity())
   {
      Vector3 vPosition = cTransform->getWorldPosition();
      Vector3 vScale = cTransform->getWorldScale();

      float fHalfSizeX = vSize.X * vScale.X * 0.5f;
      float fHalfSizeY = vSize.Y * vScale.Y * 0.5f;

      float fCenterX = vCenter.X;
      float fCenterY = vCenter.Y;

      if(mCenterMode == CenterMode::Relative)
      {
         fCenterX *= vSize.X * 0.5f;
         fCenterY *= vSize.Y * 0.5f;
      }

      if(mSizeMode == SizeMode::Relative)
      {
         fHalfSizeY *= Device::getAspectRatio();
         fCenterY *= Device::getAspectRatio();
      }

      return
         ScreenPosition.X > (vPosition.X - fCenterX - fHalfSizeX) &&
         ScreenPosition.X < (vPosition.X - fCenterX + fHalfSizeX) &&
         ScreenPosition.Y > (vPosition.Y - fCenterY - fHalfSizeY) &&
         ScreenPosition.Y < (vPosition.Y - fCenterY + fHalfSizeY);
   }

   Vector3 vVertices[4];
   float* fVertexData = sGeometryData.VertexData;

   memcpy(&vVertices[0], fVertexData + 0, sizeof(Vector3));
   memcpy(&vVertices[1], fVertexData + 5, sizeof(Vector3));
   memcpy(&vVertices[2], fVertexData + 10, sizeof(Vector3));
   memcpy(&vVertices[3], fVertexData + 15, sizeof(Vector3));

   ComponentTransform* cTransform = cOwner->getComponent<ComponentTransform>();
   const Matrix4& mWorldMatrix = cTransform->getGlobalWorldMatrix();

   Matrix4Transform(mWorldMatrix, &vVertices[0]);
   Matrix4Transform(mWorldMatrix, &vVertices[1]);
   Matrix4Transform(mWorldMatrix, &vVertices[2]);
   Matrix4Transform(mWorldMatrix, &vVertices[3]);

   Physics::Ray sRay = Physics::Ray(Vector3(ScreenPosition.X, ScreenPosition.Y, 0.0f), -Vector3::UnitZ);

   if(eRenderingMode == RenderingMode::_3D)
   {
      ComponentCamera* cCamera = RenderSystem::getInstance()->getActiveCamera();

      if(!cCamera)
         return false;

      sRay = cCamera->getScreenRay(ScreenPosition);
   }

   for(uint i = 0; i < 2; i++)
   {
      Vector3 e1 = vVertices[i + 1] - vVertices[i];
      Vector3 e2 = vVertices[i + 2] - vVertices[i];

      Vector3 vPVec = sRay.Direction.crossProduct(e2);
      float fDet = vPVec.dotProduct(e1);

      if(fabsf(fDet) < GE_EPSILON)
         continue;

      float fInvDet = 1.0f / fDet;
      Vector3 vTVec = sRay.Origin - vVertices[i];
      float u = fInvDet * vTVec.dotProduct(vPVec);

      if(u < 0.0f || u > 1.0f)
         continue;

      Vector3 vQVec = vTVec.crossProduct(e1);
      float v = fInvDet * vQVec.dotProduct(sRay.Direction);

      if(v >= 0.0f && (u + v) <= 1.0f)
         return true;
   }

   return false;
}
