
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Entities
//
//  --- GEComponentParticleSystem.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentParticleSystem.h"
#include "GEComponentTransform.h"
#include "GEComponentCamera.h"
#include "GEEntity.h"
#include "Core/GEAllocator.h"
#include "Core/GETime.h"
#include "Core/GERand.h"
#include "Core/GEProfiler.h"
#include "Content/GEMesh.h"
#include "Rendering/GERenderSystem.h"

#include <algorithm>

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;
using namespace GE::Content;


#define GERegisterValueProvider(pPropertyBaseName, pCategoryName, pDefaultValue) \
   m##pPropertyBaseName##Type = ValueProviderType::Constant; \
   m##pPropertyBaseName##Value = pDefaultValue; \
   m##pPropertyBaseName##ValueMax = pDefaultValue; \
   m##pPropertyBaseName##Curve = nullptr; \
   GERegisterPropertyEnum(ValueProviderType, pPropertyBaseName##Type)->setClass(#pCategoryName); \
   GERegisterProperty(Float, pPropertyBaseName##Value)->setClass(#pCategoryName); \
   GERegisterProperty(Float, pPropertyBaseName##ValueMax)->setClass(#pCategoryName); \
   GERegisterProperty(ObjectName, pPropertyBaseName##Curve)->setClass(#pCategoryName); \
   set##pPropertyBaseName##Type(ValueProviderType::Constant);


//
//  ComponentParticleSystem
//
static const float kFontSizeScale = 0.0001f;

RandFloat cRandFloat01(0.0f, 1.0f);

const ObjectName ParticleEmissionName = ObjectName("ParticleEmission");
const ObjectName ParticleSizeName = ObjectName("ParticleSize");
const ObjectName ParticleVelocityName = ObjectName("ParticleVelocity");

const ObjectName ComponentParticleSystem::ClassName = ObjectName("ParticleSystem");

ComponentParticleSystem::ComponentParticleSystem(Entity* Owner)
   : ComponentRenderable(Owner)
   , iMaxParticles(256)
   , fElapsedTimeSinceLastEmission(0.0f)
   , bVertexDataReallocationPending(true)
   , mBurstPending(false)
   , mParticleType(ParticleType::Billboard)
   , eEmitterType(ParticleEmitterType::Point)
   , fEmitterRadius(0.0f)
   , mParticleMesh(nullptr)
   , cEmitterMesh(nullptr)
   , cEmitterMeshEntity(nullptr)
   , mParticleTextFont(nullptr)
   , mParticleTextSize(12.0f)
   , bEmitterActive(false)
   , mSettings(0u)
   , fEmissionRate(0.0f)
   , iEmissionBurstCount(1)
   , fParticleLifeTimeMin(0.0f)
   , fParticleLifeTimeMax(0.0f)
   , fParticleSizeMultiplier(1.0f)
   , mFrictionFactor(0.0f)
{
   mClassNames.push_back(ClassName);

   GERegisterProperty(UInt, MaxParticles);
   GERegisterPropertyReadonly(UInt, ParticlesCount);

   GERegisterPropertyEnum(ParticleType, ParticleType);
   GERegisterProperty(ObjectName, ParticleMesh);
   GERegisterProperty(String, ParticleText);
   GERegisterProperty(ObjectName, ParticleTextFontName);
   GERegisterProperty(Float, ParticleTextSize);

   GERegisterPropertyBitMask(ParticleSystemSettingsBitMask, Settings);

   GERegisterProperty(Float, ParticleLifeTimeMin);
   GERegisterProperty(Float, ParticleLifeTimeMax);

   GERegisterProperty(Vector3, ParticleInitialAngleMin);
   GERegisterProperty(Vector3, ParticleInitialAngleMax);

   GERegisterProperty(Bool, EmitterActive)->setClass(ParticleEmissionName);
   GERegisterPropertyEnum(ParticleEmitterType, EmitterType)->setClass(ParticleEmissionName);
   GERegisterProperty(Vector3, EmitterPointA)->setClass(ParticleEmissionName);
   GERegisterProperty(Vector3, EmitterPointB)->setClass(ParticleEmissionName);
   GERegisterProperty(Float, EmitterRadius)->setClass(ParticleEmissionName);
   GERegisterProperty(ObjectName, EmitterMesh)->setClass(ParticleEmissionName);
   GERegisterProperty(ObjectName, EmitterMeshEntity)->setClass(ParticleEmissionName);
   GERegisterProperty(Float, EmissionRate)->setClass(ParticleEmissionName);
   GERegisterProperty(UInt, EmissionBurstCount)->setClass(ParticleEmissionName);

   GERegisterValueProvider(ParticleColorR, ParticleColor, 1.0f);
   GERegisterValueProvider(ParticleColorG, ParticleColor, 1.0f);
   GERegisterValueProvider(ParticleColorB, ParticleColor, 1.0f);
   GERegisterValueProvider(ParticleAlpha, ParticleColor, 1.0f);

   GERegisterValueProvider(ParticleSize, ParticleSize, 1.0f);
   GERegisterProperty(Float, ParticleSizeMultiplier)->setClass(ParticleSizeName);
   GERegisterValueProvider(ParticleScaleX, ParticleSize, 1.0f);
   GERegisterValueProvider(ParticleScaleY, ParticleSize, 1.0f);
   GERegisterValueProvider(ParticleScaleZ, ParticleSize, 1.0f);

   GERegisterValueProvider(ParticleLinearVelocityX, ParticleVelocity, 0.0f);
   GERegisterValueProvider(ParticleLinearVelocityY, ParticleVelocity, 0.0f);
   GERegisterValueProvider(ParticleLinearVelocityZ, ParticleVelocity, 0.0f);
   GERegisterValueProvider(ParticleAngularVelocityX, ParticleVelocity, 0.0f);
   GERegisterValueProvider(ParticleAngularVelocityY, ParticleVelocity, 0.0f);
   GERegisterValueProvider(ParticleAngularVelocityZ, ParticleVelocity, 0.0f);
   GERegisterProperty(Vector3, ConstantForce)->setClass(ParticleVelocityName);
   GERegisterProperty(Vector3, ConstantAcceleration)->setClass(ParticleVelocityName);
   GERegisterProperty(Vector3, TurbulenceFactor)->setClass(ParticleVelocityName);
   GERegisterProperty(Float, FrictionFactor)->setClass(ParticleVelocityName);

   GERegisterValueProvider(ParticleTextureAtlasIndex, ParticleTextureAtlas, 0.0f);

   registerAction("Burst", [this] { mBurstPending = true; });
   registerAction("Prewarm", [this] { prewarm(); });
}

ComponentParticleSystem::~ComponentParticleSystem()
{
   if(sGeometryData.VertexData)
   {
      Allocator::free(sGeometryData.VertexData);
   }

   if(sGeometryData.Indices)
   {
      Allocator::free(sGeometryData.Indices);
   }
}

uint ComponentParticleSystem::getMaxParticles() const
{
   return iMaxParticles;
}

void ComponentParticleSystem::setMaxParticles(uint MaxParticles)
{
   if(MaxParticles == iMaxParticles)
      return;

   iMaxParticles = MaxParticles;
   bVertexDataReallocationPending = true;
}

void ComponentParticleSystem::setParticleMesh(const ObjectName& pMeshName)
{
   if(pMeshName.isEmpty())
   {
      mParticleMesh = nullptr;
   }
   else
   {
      mParticleMesh = SerializableResourcesManager::getInstance()->get<Mesh>(pMeshName);
   }

   bVertexDataReallocationPending = true;
}

void ComponentParticleSystem::setParticleText(const char* pText)
{
   mParticleText.assign(pText);

   bVertexDataReallocationPending = true;
}

void ComponentParticleSystem::setParticleTextFontName(const Core::ObjectName& pFontName)
{
   if(pFontName.isEmpty())
   {
      mParticleTextFont = nullptr;
   }
   else
   {
      mParticleTextFont = RenderSystem::getInstance()->getFont(pFontName);
   }
}

void ComponentParticleSystem::setParticleTextSize(float pSize)
{
   mParticleTextSize = pSize;
}

void ComponentParticleSystem::setEmitterPointA(const Vector3& Point)
{
   vEmitterPointA = Point;
}

void ComponentParticleSystem::setEmitterPointB(const Vector3& Point)
{
   vEmitterPointB = Point;
}

void ComponentParticleSystem::setEmitterRadius(float Radius)
{
   fEmitterRadius = Radius;
}

void ComponentParticleSystem::setEmitterMesh(const ObjectName& MeshName)
{
   if(MeshName.isEmpty())
   {
      cEmitterMesh = nullptr;
   }
   else
   {
      cEmitterMesh = SerializableResourcesManager::getInstance()->get<Mesh>(MeshName);
   }
}

void ComponentParticleSystem::setEmitterMeshEntity(const ObjectName& EntityName)
{
   if(EntityName.isEmpty())
   {
      cEmitterMeshEntity = nullptr;
   }
   else
   {
      Scene* cScene = cOwner->getOwner();
      cEmitterMeshEntity = cScene->getEntity(EntityName);
   }
}

const ObjectName& ComponentParticleSystem::getParticleMesh() const
{
   return mParticleMesh ? mParticleMesh->getName() : ObjectName::Empty;
}

const char* ComponentParticleSystem::getParticleText() const
{
   return mParticleText.c_str();
}

const ObjectName& ComponentParticleSystem::getParticleTextFontName() const
{
   return mParticleTextFont ? mParticleTextFont->getName() : ObjectName::Empty;
}

Font* ComponentParticleSystem::getParticleTextFont() const
{
   return mParticleTextFont;
}

float ComponentParticleSystem::getParticleTextSize() const
{
   return mParticleTextSize;
}

const Vector3& ComponentParticleSystem::getEmitterPointA() const
{
   return vEmitterPointA;
}

const Vector3& ComponentParticleSystem::getEmitterPointB() const
{
   return vEmitterPointB;
}

float ComponentParticleSystem::getEmitterRadius() const
{
   return fEmitterRadius;
}

const ObjectName& ComponentParticleSystem::getEmitterMesh() const
{
   return cEmitterMesh ? cEmitterMesh->getName() : ObjectName::Empty;
}

const ObjectName& ComponentParticleSystem::getEmitterMeshEntity() const
{
   return cEmitterMeshEntity ? cEmitterMeshEntity->getFullName() : ObjectName::Empty;
}

void ComponentParticleSystem::emitParticle()
{
   if((uint)lParticles.size() == iMaxParticles)
      return;

   Particle particle;

   switch(eEmitterType)
   {
   case ParticleEmitterType::Point:
      {
         particle.Position = cTransform->getWorldPosition();
      }
      break;

   case ParticleEmitterType::Line:
      {
         Vector3 vEmitterWorldPointA = vEmitterPointA;
         Vector3 vEmitterWorldPointB = vEmitterPointB;
         Matrix4Transform(cTransform->getGlobalWorldMatrix(), &vEmitterWorldPointA);
         Matrix4Transform(cTransform->getGlobalWorldMatrix(), &vEmitterWorldPointB);

         Vector3 vDiff = vEmitterWorldPointB - vEmitterWorldPointA;
         particle.Position = vEmitterWorldPointA + (vDiff * getRandomFloat(0.0f, 1.0f));
      }
      break;

   case ParticleEmitterType::Sphere:
      {
         Vector3 vDir = getRandomVector3(-Vector3::One, Vector3::One);
         vDir.normalize();
         particle.Position = cTransform->getWorldPosition() + (vDir * getRandomFloat(0.0f, fEmitterRadius));
      }
      break;

   case ParticleEmitterType::SphereSurface:
      {
         Vector3 vDir;
         
         do
         {
            vDir = getRandomVector3(-Vector3::One, Vector3::One);
         }
         while(vDir.getSquaredLength() < GE_EPSILON);

         vDir.normalize();
         particle.Position = cTransform->getWorldPosition() + (vDir * fEmitterRadius);
      }
      break;

   case ParticleEmitterType::Mesh:
      {
         const GeometryData* cEmitterMeshGeometryData = 0;
         ComponentTransform* cEmitterMeshTransform = 0;

         if(cEmitterMesh)
         {
            cEmitterMeshGeometryData = &cEmitterMesh->getGeometryData();
            cEmitterMeshTransform = cTransform;
         }
         else if(cEmitterMeshEntity)
         {
            cEmitterMeshGeometryData = &cEmitterMeshEntity->getComponent<ComponentMesh>()->getGeometryData();
            cEmitterMeshTransform = cEmitterMeshEntity->getComponent<ComponentTransform>();
         }
         else
         {
            return;
         }

         const uint iTrianglesCount = cEmitterMeshGeometryData->NumIndices / 3;

         RandInt cRandInt(0, iTrianglesCount - 1);
         uint iTriangleIndex = cRandInt.generate();

         ushort* iIndices = cEmitterMeshGeometryData->Indices + (iTriangleIndex * 3);
         const uint iFloatsPerVertex = cEmitterMeshGeometryData->VertexStride / sizeof(float);

         Vector3 v1 =
            *reinterpret_cast<Vector3*>(cEmitterMeshGeometryData->VertexData + ((uint)(*iIndices++) * iFloatsPerVertex));
         Vector3 v2 =
            *reinterpret_cast<Vector3*>(cEmitterMeshGeometryData->VertexData + ((uint)(*iIndices++) * iFloatsPerVertex));
         Vector3 v3 =
            *reinterpret_cast<Vector3*>(cEmitterMeshGeometryData->VertexData + ((uint)(*iIndices) * iFloatsPerVertex));

         const Matrix4& mWorldTransform = cEmitterMeshTransform->getGlobalWorldMatrix();

         Matrix4Transform(mWorldTransform, &v1);
         Matrix4Transform(mWorldTransform, &v2);
         Matrix4Transform(mWorldTransform, &v3);

         particle.Position = getRandomPointInTriangle(v1, v2, v3);
      }
      break;

   default:
      {
         particle.Position = cTransform->getWorldPosition();
      }
      break;
   }

   particle.Angle = getRandomVector3(mParticleInitialAngleMin * GE_DEG2RAD, mParticleInitialAngleMax * GE_DEG2RAD);
   particle.Scale = Vector3
   (
      getParticleScaleX(particle.LifeTime, particle.RemainingLifeTime),
      getParticleScaleY(particle.LifeTime, particle.RemainingLifeTime),
      getParticleScaleZ(particle.LifeTime, particle.RemainingLifeTime)
   );

   particle.DiffuseColor = Color
   (
      getParticleColorR(particle.LifeTime, particle.RemainingLifeTime),
      getParticleColorG(particle.LifeTime, particle.RemainingLifeTime),
      getParticleColorB(particle.LifeTime, particle.RemainingLifeTime),
      getParticleAlpha(particle.LifeTime, particle.RemainingLifeTime)
   );

   particle.Size = getParticleSize(particle.LifeTime, particle.RemainingLifeTime) * fParticleSizeMultiplier;

   particle.TextureAtlasIndex = (uint32_t)getParticleTextureAtlasIndex(particle.LifeTime, particle.RemainingLifeTime);

   particle.LinearVelocity = Vector3
   (
      getParticleLinearVelocityX(particle.LifeTime, particle.RemainingLifeTime),
      getParticleLinearVelocityY(particle.LifeTime, particle.RemainingLifeTime),
      getParticleLinearVelocityZ(particle.LifeTime, particle.RemainingLifeTime)
   );
   particle.AngularVelocity = Vector3
   (
      getParticleAngularVelocityX(particle.LifeTime, particle.RemainingLifeTime),
      getParticleAngularVelocityY(particle.LifeTime, particle.RemainingLifeTime),
      getParticleAngularVelocityZ(particle.LifeTime, particle.RemainingLifeTime)
   );

   particle.LifeTime = getRandomFloat(fParticleLifeTimeMin, fParticleLifeTimeMax);
   particle.RemainingLifeTime = particle.LifeTime;

   lParticles.push_back(particle);
}

void ComponentParticleSystem::burst(uint NumParticles)
{
   for(uint i = 0; i < NumParticles; i++)
   {
      emitParticle();
   }
}

void ComponentParticleSystem::update()
{
   GEProfilerMarker("ComponentParticleSystem::update()");

   if(bVertexDataReallocationPending)
   {
      allocateVertexData();
      bVertexDataReallocationPending = false;
   }

   if(GEHasFlag(mSettings, ParticleSystemSettingsBitMask::Prewarm) && bEmitterActive && lParticles.empty())
   {
      prewarm();
   }

   if(mBurstPending)
   {
      burst(iEmissionBurstCount);
      mBurstPending = false;
   }

   const float deltaTime = cOwner->getClock()->getDelta();
   simulate(deltaTime);

   if(eRenderingMode == RenderingMode::_3D)
   {
      ComponentCamera* camera = RenderSystem::getInstance()->getActiveCamera();

      if(!camera)
      {
         return;
      }

      const Vector3 cameraWorldPosition = camera->getTransform()->getWorldPosition();

      std::sort(lParticles.begin(), lParticles.end(),
         [&cameraWorldPosition](const Particle& pP1, const Particle& pP2) -> bool
         {
            const Vector3 p1ToCamera = cameraWorldPosition - pP1.Position;
            const Vector3 p2ToCamera = cameraWorldPosition - pP2.Position;
            return p1ToCamera.getSquaredLength() > p2ToCamera.getSquaredLength();
         });
   }

   composeVertexData();
}

void ComponentParticleSystem::simulate(float pDeltaTime)
{
   // active particles
   uint32_t particleIndex = 0;

   Rotation worldRotation;
   
   if(GEHasFlag(mSettings, ParticleSystemSettingsBitMask::LocalSpace))
   {
      worldRotation = cTransform->getWorldRotation();
   }

   while(particleIndex < (uint32_t)lParticles.size())
   {
      Particle& particle = lParticles[particleIndex];
      particle.RemainingLifeTime -= pDeltaTime;

      if(particle.RemainingLifeTime <= 0.0f)
      {
         lParticles.erase(lParticles.begin() + particleIndex);
         continue;
      }

      Vector3 particleTranslation = particle.LinearVelocity + vConstantForce;

      if(GEHasFlag(mSettings, ParticleSystemSettingsBitMask::LocalSpace))
      {
         Matrix4Transform(worldRotation.getRotationMatrix(), &particleTranslation);
      }

      particle.Position += particleTranslation * pDeltaTime;
      particle.Angle += particle.AngularVelocity * GE_DEG2RAD * pDeltaTime;

      if(mParticleSizeType == ValueProviderType::Curve)
      {
         particle.Size = getParticleSize(particle.LifeTime, particle.RemainingLifeTime) * fParticleSizeMultiplier;
      }
      if(mParticleScaleXType == ValueProviderType::Curve)
      {
         particle.Scale.X = getParticleScaleX(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleScaleYType == ValueProviderType::Curve)
      {
         particle.Scale.Y = getParticleScaleY(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleScaleZType == ValueProviderType::Curve)
      {
         particle.Scale.Z = getParticleScaleZ(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleColorRType == ValueProviderType::Curve)
      {
         particle.DiffuseColor.Red = getParticleColorR(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleColorGType == ValueProviderType::Curve)
      {
         particle.DiffuseColor.Green = getParticleColorG(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleColorBType == ValueProviderType::Curve)
      {
         particle.DiffuseColor.Blue = getParticleColorB(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleAlphaType == ValueProviderType::Curve)
      {
         particle.DiffuseColor.Alpha = getParticleAlpha(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleLinearVelocityXType == ValueProviderType::Curve)
      {
         particle.LinearVelocity.X = getParticleLinearVelocityX(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleLinearVelocityYType == ValueProviderType::Curve)
      {
         particle.LinearVelocity.Y = getParticleLinearVelocityY(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleLinearVelocityZType == ValueProviderType::Curve)
      {
         particle.LinearVelocity.Z = getParticleLinearVelocityZ(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleAngularVelocityXType == ValueProviderType::Curve)
      {
         particle.AngularVelocity.X = getParticleAngularVelocityX(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleAngularVelocityYType == ValueProviderType::Curve)
      {
         particle.AngularVelocity.Y = getParticleAngularVelocityY(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleAngularVelocityZType == ValueProviderType::Curve)
      {
         particle.AngularVelocity.Z = getParticleAngularVelocityZ(particle.LifeTime, particle.RemainingLifeTime);
      }
      if(mParticleTextureAtlasIndexType == ValueProviderType::Curve)
      {
         particle.TextureAtlasIndex = (uint)getParticleTextureAtlasIndex(particle.LifeTime, particle.RemainingLifeTime);
      }

      particle.LinearVelocity += vConstantAcceleration * pDeltaTime;

      if(mTurbulenceFactor.X > GE_EPSILON)
      {
         particle.LinearVelocity.X +=
            getRandomFloat(-mTurbulenceFactor.X, mTurbulenceFactor.X) * pDeltaTime;
      }
      if(mTurbulenceFactor.Y > GE_EPSILON)
      {
         particle.LinearVelocity.Y +=
            getRandomFloat(-mTurbulenceFactor.Y, mTurbulenceFactor.Y) * pDeltaTime;
      }
      if(mTurbulenceFactor.Z > GE_EPSILON)
      {
         particle.LinearVelocity.Z +=
            getRandomFloat(-mTurbulenceFactor.Z, mTurbulenceFactor.Z) * pDeltaTime;
      }

      if(mFrictionFactor > GE_EPSILON)
      {
         particle.LinearVelocity =
            particle.LinearVelocity * (1.0f - (mFrictionFactor * pDeltaTime));
      }

      particleIndex++;
   }

   // new particles
   if(bEmitterActive)
   {
      fElapsedTimeSinceLastEmission += pDeltaTime;

      float fTimeToEmitParticle = 1.0f / fEmissionRate;

      while(fElapsedTimeSinceLastEmission > fTimeToEmitParticle)
      {
         if(iEmissionBurstCount == 1)
         {
            emitParticle();
         }
         else
         {
            burst(iEmissionBurstCount);
         }

         fElapsedTimeSinceLastEmission -= fTimeToEmitParticle;
      }
   }
}

void ComponentParticleSystem::setParticleType(ParticleType pParticleType)
{
   if(mParticleType != pParticleType)
   {
      mParticleType = pParticleType;
      bVertexDataReallocationPending = true;
   }
}

void ComponentParticleSystem::setEmitterActive(bool Active)
{
   if(bEmitterActive != Active)
   {
      bEmitterActive = Active;

      if(bEmitterActive)
      {
         fElapsedTimeSinceLastEmission = fEmissionRate > GE_EPSILON
            ? 1.0f / fEmissionRate
            : 0.0f;
      }
   }
}

void ComponentParticleSystem::prewarm()
{
   for(uint32_t i = 0u; i < iMaxParticles; i++)
   {
      simulate(0.1f);
   }
}

void ComponentParticleSystem::allocateVertexData()
{
   // Billboard
   if(mParticleType == ParticleType::Billboard)
   {
      sGeometryData.NumVertices = 4u * iMaxParticles;
      sGeometryData.VertexStride = (3 + 4 + 2) * sizeof(float);
      sGeometryData.VertexData =
         Allocator::realloc<float>(sGeometryData.VertexData, sGeometryData.VertexStride * sGeometryData.NumVertices);

      sGeometryData.NumIndices = 6u * iMaxParticles;
      sGeometryData.Indices =
         Allocator::realloc<ushort>(sGeometryData.Indices, sGeometryData.NumIndices);

      ushort* indices = sGeometryData.Indices;
      ushort currentVertexOffset = 0u;

      for(uint32_t i = 0u; i < iMaxParticles; i++)
      {
         *indices++ = QuadIndices[0] + currentVertexOffset;
         *indices++ = QuadIndices[1] + currentVertexOffset;
         *indices++ = QuadIndices[2] + currentVertexOffset;
         *indices++ = QuadIndices[3] + currentVertexOffset;
         *indices++ = QuadIndices[4] + currentVertexOffset;
         *indices++ = QuadIndices[5] + currentVertexOffset;

         currentVertexOffset += 4u;
      }
   }
   // Mesh
   else if(mParticleType == ParticleType::Mesh)
   {
      sGeometryData.NumVertices = 0u;
      sGeometryData.NumIndices = 0u;

      if(mParticleMesh)
      {
         sGeometryData.NumVertices = mParticleMesh->getVertexCount() * iMaxParticles;
         sGeometryData.VertexStride = (3 + 4 + 2) * sizeof(float);
         sGeometryData.VertexData =
            Allocator::realloc<float>(sGeometryData.VertexData, sGeometryData.VertexStride * sGeometryData.NumVertices);

         sGeometryData.NumIndices = mParticleMesh->getGeometryData().NumIndices * iMaxParticles;
         sGeometryData.Indices =
            Allocator::realloc<ushort>(sGeometryData.Indices, sGeometryData.NumIndices);

         ushort* indices = sGeometryData.Indices;
         ushort currentVertexOffset = 0u;

         for(uint32_t i = 0u; i < iMaxParticles; i++)
         {
            for(uint32_t j = 0u; j < mParticleMesh->getGeometryData().NumIndices; j++)
            {
               *indices++ = mParticleMesh->getGeometryData().Indices[j] + currentVertexOffset;
            }

            currentVertexOffset += mParticleMesh->getVertexCount();
         }
      }
   }
   // Text
   else
   {
      sGeometryData.NumVertices = 0u;
      sGeometryData.NumIndices = 0u;

      if(mParticleTextFont)
      {
         const uint32_t textLength = getParticleTextLength();

         if(textLength > 0u)
         {
            sGeometryData.NumVertices = 4u * textLength * iMaxParticles;
            sGeometryData.VertexStride = (3 + 4 + 2) * sizeof(float);
            sGeometryData.VertexData =
               Allocator::realloc<float>(sGeometryData.VertexData, sGeometryData.VertexStride * sGeometryData.NumVertices);

            sGeometryData.NumIndices = 6u * textLength * iMaxParticles;
            sGeometryData.Indices =
               Allocator::realloc<ushort>(sGeometryData.Indices, sGeometryData.NumIndices);

            ushort* indices = sGeometryData.Indices;
            ushort currentVertexOffset = 0u;

            for(uint32_t i = 0u; i < (textLength * iMaxParticles); i++)
            {
               *indices++ = QuadIndices[0] + currentVertexOffset;
               *indices++ = QuadIndices[1] + currentVertexOffset;
               *indices++ = QuadIndices[2] + currentVertexOffset;
               *indices++ = QuadIndices[3] + currentVertexOffset;
               *indices++ = QuadIndices[4] + currentVertexOffset;
               *indices++ = QuadIndices[5] + currentVertexOffset;

               currentVertexOffset += 4u;
            }
         }
      }
   }

   if(lParticles.size() > iMaxParticles)
   {
      lParticles.resize(iMaxParticles);
      lParticles.shrink_to_fit();
   }
}

void ComponentParticleSystem::composeVertexData()
{
   if(mParticleType == ParticleType::Billboard)
   {
      composeBillboardVertexData();
   }
   else if(mParticleType == ParticleType::Mesh)
   {
      composeMeshVertexData();
   }
   else
   {
      composeTextVertexData();
   }
}

void ComponentParticleSystem::composeBillboardVertexData()
{
   Vector3 cameraRight;
   Vector3 cameraUp;
   Vector3 cameraForward;

   if(eRenderingMode == RenderingMode::_3D)
   {
      ComponentCamera* camera = RenderSystem::getInstance()->getActiveCamera();
      cameraRight = camera->getTransform()->getRightVector();
      cameraUp = camera->getTransform()->getUpVector();
      cameraForward = camera->getTransform()->getForwardVector();
   }
   else
   {
      cameraRight = Vector3::UnitX;
      cameraUp = Vector3::UnitY;
      cameraForward = Vector3::UnitZ;
   }

   const Texture* diffuseTexture = nullptr;

   if(!vMaterialPassList.empty())
   {
      Material* material = getMaterialPass(0)->getMaterial();

      if(material)
      {
         diffuseTexture = material->getDiffuseTexture();
      }
   }

   float* vertexData = sGeometryData.VertexData;

   for(ParticleList::iterator it = lParticles.begin(); it != lParticles.end(); it++)
   {
      Particle& particle = *it;

      const float particleHalfSizeX = particle.Size * particle.Scale.X * 0.5f;
      const float particleHalfSizeY = particle.Size * particle.Scale.Y * 0.5f;

      Vector3 topLeftPosition = -(cameraRight * particleHalfSizeX) + (cameraUp * particleHalfSizeY);
      Vector3 topRightPosition = (cameraRight * particleHalfSizeX) + (cameraUp * particleHalfSizeY);
      Vector3 bottomLeftPosition = -(cameraRight * particleHalfSizeX) - (cameraUp * particleHalfSizeY);
      Vector3 bottomRightPosition = (cameraRight * particleHalfSizeX) - (cameraUp * particleHalfSizeY);

      Matrix4 mTransform;

      if(fabsf(particle.Angle.Z) < GE_EPSILON)
      {
         Matrix4MakeIdentity(&mTransform);
      }
      else
      {
         Rotation cRotation = Rotation(cameraForward, particle.Angle.Z);
         mTransform = cRotation.getRotationMatrix();
      }

      mTransform.m[GE_M4_1_4] = particle.Position.X;
      mTransform.m[GE_M4_2_4] = particle.Position.Y;
      mTransform.m[GE_M4_3_4] = particle.Position.Z;

      Matrix4Transform(mTransform, &topLeftPosition);
      Matrix4Transform(mTransform, &topRightPosition);
      Matrix4Transform(mTransform, &bottomLeftPosition);
      Matrix4Transform(mTransform, &bottomRightPosition);

      TextureCoordinates textureCoordinates;

      if(particle.TextureAtlasIndex > 0u && diffuseTexture)
      {
         uint32_t textureAtlasIndex = particle.TextureAtlasIndex;

         if(textureAtlasIndex > (uint32_t)(diffuseTexture->AtlasUV.size() - 1u))
         {
            textureAtlasIndex = (uint32_t)(diffuseTexture->AtlasUV.size() - 1u);
         }

         textureCoordinates = diffuseTexture->AtlasUV[textureAtlasIndex].UV;
      }
      else
      {
         textureCoordinates.U0 = 0.0f;
         textureCoordinates.V0 = 0.0f;
         textureCoordinates.U1 = 1.0f;
         textureCoordinates.V1 = 1.0f;
      }

      // Top left
      *vertexData++ = topLeftPosition.X;
      *vertexData++ = topLeftPosition.Y;
      *vertexData++ = topLeftPosition.Z;

      *vertexData++ = particle.DiffuseColor.Red;
      *vertexData++ = particle.DiffuseColor.Green;
      *vertexData++ = particle.DiffuseColor.Blue;
      *vertexData++ = particle.DiffuseColor.Alpha;

      *vertexData++ = textureCoordinates.U0;
      *vertexData++ = textureCoordinates.V0;

      // Bottom left
      *vertexData++ = bottomLeftPosition.X;
      *vertexData++ = bottomLeftPosition.Y;
      *vertexData++ = bottomLeftPosition.Z;

      *vertexData++ = particle.DiffuseColor.Red;
      *vertexData++ = particle.DiffuseColor.Green;
      *vertexData++ = particle.DiffuseColor.Blue;
      *vertexData++ = particle.DiffuseColor.Alpha;

      *vertexData++ = textureCoordinates.U0;
      *vertexData++ = textureCoordinates.V1;

      // Top right
      *vertexData++ = topRightPosition.X;
      *vertexData++ = topRightPosition.Y;
      *vertexData++ = topRightPosition.Z;

      *vertexData++ = particle.DiffuseColor.Red;
      *vertexData++ = particle.DiffuseColor.Green;
      *vertexData++ = particle.DiffuseColor.Blue;
      *vertexData++ = particle.DiffuseColor.Alpha;

      *vertexData++ = textureCoordinates.U1;
      *vertexData++ = textureCoordinates.V0;

      // Bottom right
      *vertexData++ = bottomRightPosition.X;
      *vertexData++ = bottomRightPosition.Y;
      *vertexData++ = bottomRightPosition.Z;

      *vertexData++ = particle.DiffuseColor.Red;
      *vertexData++ = particle.DiffuseColor.Green;
      *vertexData++ = particle.DiffuseColor.Blue;
      *vertexData++ = particle.DiffuseColor.Alpha;

      *vertexData++ = textureCoordinates.U1;
      *vertexData++ = textureCoordinates.V1;
   }

   sGeometryData.NumVertices = (uint32_t)lParticles.size() * 4u;
   sGeometryData.NumIndices = (uint32_t)lParticles.size() * 6u;
}

void ComponentParticleSystem::composeMeshVertexData()
{
   if(!mParticleMesh)
   {
      return;
   }

   const Texture* diffuseTexture = nullptr;

   if(!vMaterialPassList.empty())
   {
      Material* material = getMaterialPass(0)->getMaterial();

      if(material)
      {
         diffuseTexture = material->getDiffuseTexture();
      }
   }

   float* vertexData = sGeometryData.VertexData;

   for(ParticleList::iterator it = lParticles.begin(); it != lParticles.end(); it++)
   {
      Particle& particle = *it;

      Rotation rotation(particle.Angle);
      Matrix4 transform = rotation.getRotationMatrix();

      transform.m[GE_M4_1_4] = particle.Position.X;
      transform.m[GE_M4_2_4] = particle.Position.Y;
      transform.m[GE_M4_3_4] = particle.Position.Z;

      const Vector3 particleSize = particle.Scale * particle.Size;
      Matrix4Scale(&transform, particleSize);

      const uint32_t meshFloatsPerVertex = (uint32_t)(mParticleMesh->getGeometryData().VertexStride / sizeof(float));

      for(uint32_t i = 0u; i < mParticleMesh->getVertexCount(); i++)
      {
         const uint32_t meshVertexOffset = i * meshFloatsPerVertex;
         const float* meshVertexData = mParticleMesh->getGeometryData().VertexData + meshVertexOffset;

         Vector3 vertexPosition(meshVertexData[0], meshVertexData[1], meshVertexData[2]);
         Matrix4Transform(transform, &vertexPosition);

         TextureCoordinates textureCoordinates;

         if(particle.TextureAtlasIndex > 0u && diffuseTexture)
         {
            uint32_t textureAtlasIndex = particle.TextureAtlasIndex;

            if(textureAtlasIndex > (uint32_t)(diffuseTexture->AtlasUV.size() - 1u))
            {
               textureAtlasIndex = (uint32_t)(diffuseTexture->AtlasUV.size() - 1u);
            }

            textureCoordinates = diffuseTexture->AtlasUV[textureAtlasIndex].UV;
         }
         else
         {
            textureCoordinates.U0 = 0.0f;
            textureCoordinates.V0 = 0.0f;
            textureCoordinates.U1 = 1.0f;
            textureCoordinates.V1 = 1.0f;
         }

         *vertexData++ = vertexPosition.X;
         *vertexData++ = vertexPosition.Y;
         *vertexData++ = vertexPosition.Z;

         *vertexData++ = particle.DiffuseColor.Red;
         *vertexData++ = particle.DiffuseColor.Green;
         *vertexData++ = particle.DiffuseColor.Blue;
         *vertexData++ = particle.DiffuseColor.Alpha;

         *vertexData++ =
            textureCoordinates.U0 + (meshVertexData[6] * (textureCoordinates.U1 - textureCoordinates.U0));
         *vertexData++ =
            textureCoordinates.V0 + (meshVertexData[7] * (textureCoordinates.V1 - textureCoordinates.V0));
      }
   }

   sGeometryData.NumVertices = (uint32_t)lParticles.size() * mParticleMesh->getVertexCount();
   sGeometryData.NumIndices = (uint32_t)lParticles.size() * mParticleMesh->getGeometryData().NumIndices;
}

void ComponentParticleSystem::composeTextVertexData()
{
   if(mParticleText.empty() || !mParticleTextFont)
   {
      return;
   }

   Vector3 cameraRight;
   Vector3 cameraUp;
   Vector3 cameraForward;

   if(mParticleType == ParticleType::TextBillboard)
   {
      if(eRenderingMode == RenderingMode::_3D)
      {
         ComponentCamera* camera = RenderSystem::getInstance()->getActiveCamera();
         cameraRight = camera->getTransform()->getRightVector();
         cameraUp = camera->getTransform()->getUpVector();
         cameraForward = camera->getTransform()->getForwardVector();
      }
      else
      {
         cameraRight = Vector3::UnitX;
         cameraUp = Vector3::UnitY;
         cameraForward = Vector3::UnitZ;
      }
   }

   const float characterSize = mParticleTextSize * kFontSizeScale;

   float textWidth = 0.0f;

   for(size_t i = 0u; i < mParticleText.size(); i++)
   {
      textWidth += getParticleTextCharWidth(i);
   }

   float* vertexData = sGeometryData.VertexData;

   for(ParticleList::iterator it = lParticles.begin(); it != lParticles.end(); it++)
   {
      Particle& particle = *it;

      Matrix4 transform;

      if(mParticleType == ParticleType::TextBillboard)
      {
         if(fabsf(particle.Angle.Z) < GE_EPSILON)
         {
            Matrix4MakeIdentity(&transform);
         }
         else
         {
            Rotation rotation(cameraForward, particle.Angle.Z);
            transform = rotation.getRotationMatrix();
         }
      }
      else
      {
         Rotation rotation(particle.Angle);
         transform = rotation.getRotationMatrix();
      }

      transform.m[GE_M4_1_4] = particle.Position.X;
      transform.m[GE_M4_2_4] = particle.Position.Y;
      transform.m[GE_M4_3_4] = particle.Position.Z;

      const Vector3 particleSize = particle.Scale * particle.Size;
      Matrix4Scale(&transform, particleSize);

      float posX = textWidth * -0.5f;

      for(size_t i = 0u; i < mParticleText.size(); i++)
      {
         const char textChar = mParticleText[i];

         const uint16_t glyphIndex = (uint16_t)textChar;
         const uint16_t charSetIndex = 0u;
         const Glyph& glyph = mParticleTextFont->getGlyph(charSetIndex, glyphIndex);

         if(textChar != ' ')
         {
            const float glyphWidth = glyph.Width * characterSize;
            const float glyphHeight = glyph.Height * characterSize;

            const float glyphOffsetX = glyph.OffsetX * characterSize;
            const float glyphOffsetY = glyph.OffsetY * characterSize;

            Vector3 topLeftPosition;
            Vector3 topRightPosition;
            Vector3 bottomLeftPosition;
            Vector3 bottomRightPosition;

            if(mParticleType == ParticleType::TextBillboard)
            {
               topLeftPosition =
                  (cameraRight * (posX + glyphOffsetX)) + (cameraUp * (glyphOffsetY - glyphHeight));
               topRightPosition =
                  (cameraRight * (posX + glyphOffsetX + glyphWidth)) + (cameraUp * (glyphOffsetY - glyphHeight));
               bottomLeftPosition =
                  (cameraRight * (posX + glyphOffsetX)) + (cameraUp * glyphOffsetY);
               bottomRightPosition =
                  (cameraRight * (posX + glyphOffsetX + glyphWidth)) + (cameraUp * glyphOffsetY);
            }
            else
            {
               topLeftPosition.X = bottomLeftPosition.X = posX + glyphOffsetX;
               topRightPosition.X = bottomRightPosition.X = posX + glyphOffsetX + glyphWidth;
               topLeftPosition.Y = topRightPosition.Y = glyphOffsetY - glyphHeight;
               bottomLeftPosition.Y = bottomRightPosition.Y = glyphOffsetY;
            }

            Matrix4Transform(transform, &topLeftPosition);
            Matrix4Transform(transform, &topRightPosition);
            Matrix4Transform(transform, &bottomLeftPosition);
            Matrix4Transform(transform, &bottomRightPosition);

            // Top left
            *vertexData++ = topLeftPosition.X;
            *vertexData++ = topLeftPosition.Y;
            *vertexData++ = topLeftPosition.Z;

            *vertexData++ = particle.DiffuseColor.Red;
            *vertexData++ = particle.DiffuseColor.Green;
            *vertexData++ = particle.DiffuseColor.Blue;
            *vertexData++ = particle.DiffuseColor.Alpha;

            *vertexData++ = glyph.UV.U0;
            *vertexData++ = glyph.UV.V1;

            // Top right
            *vertexData++ = topRightPosition.X;
            *vertexData++ = topRightPosition.Y;
            *vertexData++ = topRightPosition.Z;

            *vertexData++ = particle.DiffuseColor.Red;
            *vertexData++ = particle.DiffuseColor.Green;
            *vertexData++ = particle.DiffuseColor.Blue;
            *vertexData++ = particle.DiffuseColor.Alpha;

            *vertexData++ = glyph.UV.U1;
            *vertexData++ = glyph.UV.V1;

            // Bottom left
            *vertexData++ = bottomLeftPosition.X;
            *vertexData++ = bottomLeftPosition.Y;
            *vertexData++ = bottomLeftPosition.Z;

            *vertexData++ = particle.DiffuseColor.Red;
            *vertexData++ = particle.DiffuseColor.Green;
            *vertexData++ = particle.DiffuseColor.Blue;
            *vertexData++ = particle.DiffuseColor.Alpha;

            *vertexData++ = glyph.UV.U0;
            *vertexData++ = glyph.UV.V0;

            // Bottom right
            *vertexData++ = bottomRightPosition.X;
            *vertexData++ = bottomRightPosition.Y;
            *vertexData++ = bottomRightPosition.Z;

            *vertexData++ = particle.DiffuseColor.Red;
            *vertexData++ = particle.DiffuseColor.Green;
            *vertexData++ = particle.DiffuseColor.Blue;
            *vertexData++ = particle.DiffuseColor.Alpha;

            *vertexData++ = glyph.UV.U1;
            *vertexData++ = glyph.UV.V0;
         }

         posX += getParticleTextCharWidth(i);
      }
   }

   const uint32_t textLength = getParticleTextLength();

   sGeometryData.NumVertices = (uint32_t)lParticles.size() * textLength * 4u;
   sGeometryData.NumIndices = (uint32_t)lParticles.size() * textLength * 6u;
}

uint32_t ComponentParticleSystem::getParticleTextLength() const
{
   uint32_t textLength = 0u;

   for(size_t i = 0u; i < mParticleText.size(); i++)
   {
      if(mParticleText[i] != ' ')
      {
         textLength++;
      }
   }

   return textLength;
}

float ComponentParticleSystem::getParticleTextCharWidth(size_t pCharIndex) const
{
   GEAssert(pCharIndex < mParticleText.size());

   const char textChar = mParticleText[pCharIndex];
   float textCharWidth = 0.0f;

   const uint16_t glyphIndex = (uint16_t)textChar;
   const uint16_t charSetIndex = 0u;
   const Glyph& glyph = mParticleTextFont->getGlyph(charSetIndex, glyphIndex);
   const float characterSize = mParticleTextSize * kFontSizeScale;
   textCharWidth += glyph.AdvanceX * characterSize;

   if(pCharIndex > 0u)
   {
      const uint16_t previousGlyphIndex = (uint16_t)mParticleText[pCharIndex - 1u];
      const float kerning =
         mParticleTextFont->getKerning(charSetIndex, previousGlyphIndex, glyphIndex);
      textCharWidth += kerning * characterSize;
   }

   return textCharWidth;
}

float ComponentParticleSystem::getRandomFloat(float fMin, float fMax)
{
   float fDiff = fMax - fMin;

   if(fabsf(fDiff) < GE_EPSILON)
   {
      return fMin;
   }

   return fMin + (cRandFloat01.generate() * fDiff);
}

Vector3 ComponentParticleSystem::getRandomVector3(const Vector3& vMin, const Vector3& vMax)
{
   return Vector3
   (
      getRandomFloat(vMin.X, vMax.X),
      getRandomFloat(vMin.Y, vMax.Y),
      getRandomFloat(vMin.Z, vMax.Z)
   );
}

Color ComponentParticleSystem::getRandomColor(const Color& cMin, const Color& cMax)
{
   return Color
   (
      getRandomFloat(cMin.Red, cMax.Red),
      getRandomFloat(cMin.Green, cMax.Green),
      getRandomFloat(cMin.Blue, cMax.Blue),
      getRandomFloat(cMin.Alpha, cMax.Alpha)
   );
}

Vector3 ComponentParticleSystem::getRandomPointInTriangle(const Vector3& P1, const Vector3& P2, const Vector3& P3)
{
   float r1 = getRandomFloat(0.0f, 1.0f);
   float r1sqrt = sqrtf(r1);
   float r2 = getRandomFloat(0.0f, 1.0f);

   return (P1 * (1.0f - r1sqrt)) + (P2 * (r1sqrt * (1.0f - r2))) + (P3 * (r2 * r1sqrt));
}
