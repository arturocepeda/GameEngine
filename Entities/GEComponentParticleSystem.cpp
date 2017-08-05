
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
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
#include "Content/GEResourcesManager.h"
#include "Rendering/GERenderSystem.h"

#include <algorithm>

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;
using namespace GE::Content;


//
//  ComponentParticleSystem
//
#if defined (GE_EDITOR_SUPPORT)
const uint MaxParticles = 8192;
#else
const uint MaxParticles = 1024;
#endif

RandFloat cRandFloat01(0.0f, 1.0f);

ComponentParticleSystem::ComponentParticleSystem(Entity* Owner)
   : ComponentRenderable(Owner, RenderableType::ParticleSystem, GeometryType::Dynamic)
   , fElapsedTimeSinceLastEmission(0.0f)
   , eEmitterType(ParticleEmitterType::Point)
   , fEmitterRadius(0.0f)
   , cEmitterMesh(0)
   , cEmitterMeshEntity(0)
   , bEmitterActive(false)
   , bDynamicShadows(false)
   , fEmissionRate(0.0f)
   , iEmissionBurstCount(1)
   , eParticleSettings((1 << (uint8_t)ParticleSettingsBitMask::Count) - 1)
   , fParticleLifeTimeMin(0.0f)
   , fParticleLifeTimeMax(0.0f)
   , fParticleInitialSizeMin(0.0f)
   , fParticleInitialSizeMax(0.0f)
   , fParticleInitialAngleMin(0.0f)
   , fParticleInitialAngleMax(0.0f)
   , fParticleAngularVelocityMin(0.0f)
   , fParticleAngularVelocityMax(0.0f)
   , fParticleFinalSizeMin(0.0f)
   , fParticleFinalSizeMax(0.0f)
   , iParticleTextureAtlasIndexMin(0)
   , iParticleTextureAtlasIndexMax(0)
{
   cClassName = ObjectName("ParticleSystem");

   eRenderingMode = RenderSystem::getInstance()->getActiveCamera()
      ? RenderingMode::_3D
      : RenderingMode::_2D;

   sGeometryData.NumVertices = 4 * MaxParticles;
   sGeometryData.VertexStride = (3 + 4 + 2) * sizeof(float);
   sGeometryData.VertexData = Allocator::alloc<float>(sGeometryData.VertexStride * sGeometryData.NumVertices);

   sGeometryData.NumIndices = 6 * MaxParticles;
   sGeometryData.Indices = Allocator::alloc<ushort>(sGeometryData.NumIndices);

   ushort* pIndices = sGeometryData.Indices;
   ushort iCurrentVertexOffset = 0;

   for(uint i = 0; i < MaxParticles; i++)
   {
      *pIndices++ = QuadIndices[0] + iCurrentVertexOffset;
      *pIndices++ = QuadIndices[1] + iCurrentVertexOffset;
      *pIndices++ = QuadIndices[2] + iCurrentVertexOffset;
      *pIndices++ = QuadIndices[3] + iCurrentVertexOffset;
      *pIndices++ = QuadIndices[4] + iCurrentVertexOffset;
      *pIndices++ = QuadIndices[5] + iCurrentVertexOffset;

      iCurrentVertexOffset += 4;
   }

   GERegisterPropertyEnum(ParticleEmitterType, EmitterType);

   GERegisterProperty(Vector3, EmitterPointA);
   GERegisterProperty(Vector3, EmitterPointB);
   GERegisterProperty(Float, EmitterRadius);
   GERegisterPropertyResource(ObjectName, EmitterMesh, Mesh);
   GERegisterProperty(ObjectName, EmitterMeshEntity);

   GERegisterProperty(Bool, EmitterActive);
   GERegisterProperty(Bool, DynamicShadows);
   GERegisterProperty(Float, EmissionRate);
   GERegisterPropertyMinMax(UInt, EmissionBurstCount, 1, 256);

   GERegisterPropertyBitMask(ParticleSettingsBitMask, ParticleSettings);

   GERegisterProperty(Float, ParticleLifeTimeMin);
   GERegisterProperty(Float, ParticleLifeTimeMax);

   GERegisterProperty(Float, ParticleInitialSizeMin);
   GERegisterProperty(Float, ParticleInitialSizeMax);
   GERegisterProperty(Color, ParticleInitialColorMin);
   GERegisterProperty(Color, ParticleInitialColorMax);
   GERegisterProperty(Float, ParticleInitialAngleMin);
   GERegisterProperty(Float, ParticleInitialAngleMax);

   GERegisterProperty(Vector3, ParticleLinearVelocityMin);
   GERegisterProperty(Vector3, ParticleLinearVelocityMax);
   GERegisterProperty(Float, ParticleAngularVelocityMin);
   GERegisterProperty(Float, ParticleAngularVelocityMax);

   GERegisterProperty(Float, ParticleFinalSizeMin);
   GERegisterProperty(Float, ParticleFinalSizeMax);
   GERegisterProperty(Color, ParticleFinalColorMin);
   GERegisterProperty(Color, ParticleFinalColorMax);

   GERegisterProperty(UInt, ParticleTextureAtlasIndexMin);
   GERegisterProperty(UInt, ParticleTextureAtlasIndexMax);

   GERegisterProperty(Vector3, ConstantForce);
   GERegisterProperty(Vector3, ConstantAcceleration);

#if defined (GE_EDITOR_SUPPORT)
   registerAction("Burst", [this] { burst(iEmissionBurstCount); });
#endif
}

ComponentParticleSystem::~ComponentParticleSystem()
{
   Allocator::free(sGeometryData.VertexData);
   Allocator::free(sGeometryData.Indices);
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

void ComponentParticleSystem::setEmitterMesh(const Core::ObjectName& MeshName)
{
   if(MeshName.isEmpty())
      return;

   cEmitterMesh = ResourcesManager::getInstance()->get<Mesh>(MeshName);
   GEAssert(cEmitterMesh);
}

void ComponentParticleSystem::setEmitterMeshEntity(const Core::ObjectName& EntityName)
{
   if(EntityName.isEmpty())
      return;

   Scene* cScene = cOwner->getOwner();
   cEmitterMeshEntity = cScene->getEntity(EntityName);
   GEAssert(cEmitterMeshEntity);
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
   GEAssert(lParticles.size() < MaxParticles);

   Particle sParticle;

   switch(eEmitterType)
   {
   case ParticleEmitterType::Point:
      {
         sParticle.Position = cTransform->getWorldPosition();
      }
      break;

   case ParticleEmitterType::Line:
      {
         Vector3 vEmitterWorldPointA = vEmitterPointA;
         Vector3 vEmitterWorldPointB = vEmitterPointB;
         Matrix4Transform(cTransform->getGlobalWorldMatrix(), &vEmitterWorldPointA);
         Matrix4Transform(cTransform->getGlobalWorldMatrix(), &vEmitterWorldPointB);

         Vector3 vDiff = vEmitterWorldPointB - vEmitterWorldPointA;
         sParticle.Position = vEmitterWorldPointA + (vDiff * getRandomFloat(0.0f, 1.0f));
      }
      break;

   case ParticleEmitterType::Sphere:
      {
         Vector3 vDir = getRandomVector3(-Vector3::One, Vector3::One);
         vDir.normalize();
         sParticle.Position = cTransform->getWorldPosition() + (vDir * getRandomFloat(0.0f, fEmitterRadius));
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
         sParticle.Position = cTransform->getWorldPosition() + (vDir * fEmitterRadius);
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
         else
         {
            cEmitterMeshGeometryData = &cEmitterMeshEntity->getComponent<ComponentMesh>()->getGeometryData();
            cEmitterMeshTransform = cEmitterMeshEntity->getComponent<ComponentTransform>();
         }

         GEAssert(cEmitterMeshGeometryData);
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

         sParticle.Position = getRandomPointInTriangle(v1, v2, v3);
      }
      break;

   default:
      {
         sParticle.Position = cTransform->getWorldPosition();
      }
      break;
   }

   sParticle.LifeTime = getRandomFloat(fParticleLifeTimeMin, fParticleLifeTimeMax);
   sParticle.RemainingLifeTime = sParticle.LifeTime;

   sParticle.Size = getRandomFloat(fParticleInitialSizeMin, fParticleInitialSizeMax);
   sParticle.DiffuseColor = getRandomColor(cParticleInitialColorMin, cParticleInitialColorMax);
   sParticle.Angle = getRandomFloat(fParticleInitialAngleMin * GE_DEG2RAD, fParticleInitialAngleMax * GE_DEG2RAD);

   if(iParticleTextureAtlasIndexMin > 0 || iParticleTextureAtlasIndexMax > 0)
   {
      RandInt cRandInt(iParticleTextureAtlasIndexMin, iParticleTextureAtlasIndexMax);
      sParticle.TextureAtlasIndex = cRandInt.generate();
   }
   else
   {
      sParticle.TextureAtlasIndex = 0;
   }

   sParticle.LinearVelocity = getRandomVector3(vParticleLinearVelocityMin, vParticleLinearVelocityMax);
   sParticle.AngularVelocity = getRandomFloat(fParticleAngularVelocityMin, fParticleAngularVelocityMax);

   float fFinalSize = getRandomFloat(fParticleFinalSizeMin, fParticleFinalSizeMax);
   Color cFinalColor = getRandomColor(cParticleFinalColorMin, cParticleFinalColorMax);
   float fInverseLifeTime = 1.0f / sParticle.LifeTime;

   sParticle.SizeVariation = (fFinalSize - sParticle.Size) * fInverseLifeTime;
   sParticle.DiffuseColorVariation = (cFinalColor - sParticle.DiffuseColor) * fInverseLifeTime;

   lParticles.push_back(sParticle);
}

void ComponentParticleSystem::burst(uint NumParticles)
{
   for(uint i = 0; i < NumParticles; i++)
      emitParticle();
}

void ComponentParticleSystem::update()
{
   GEProfilerMarker("ComponentParticleSystem::update()");

   const float fDeltaTime = Time::getClock(cOwner->getClockIndex()).getDelta();

   // active particles
   ParticleList::iterator it = lParticles.begin();

   while(it != lParticles.end())
   {
      Particle& sParticle = *it;
      sParticle.RemainingLifeTime -= fDeltaTime;

      if(sParticle.RemainingLifeTime <= 0.0f)
      {
         it = lParticles.erase(it);
         continue;
      }

      sParticle.Position += sParticle.LinearVelocity * fDeltaTime;
      sParticle.Angle += sParticle.AngularVelocity * GE_DEG2RAD * fDeltaTime;

      if(GEHasFlag(eParticleSettings, ParticleSettingsBitMask::VarySize))
      {
         sParticle.Size += sParticle.SizeVariation * fDeltaTime;
      }

      if(GEHasFlag(eParticleSettings, ParticleSettingsBitMask::VaryColor))
      {
         sParticle.DiffuseColor.Red += sParticle.DiffuseColorVariation.Red * fDeltaTime;
         sParticle.DiffuseColor.Green += sParticle.DiffuseColorVariation.Green * fDeltaTime;
         sParticle.DiffuseColor.Blue += sParticle.DiffuseColorVariation.Blue * fDeltaTime;
      }

      if(GEHasFlag(eParticleSettings, ParticleSettingsBitMask::VaryAlpha))
      {
         sParticle.DiffuseColor.Alpha += sParticle.DiffuseColorVariation.Alpha * fDeltaTime;
      }

      sParticle.Position += vConstantForce * fDeltaTime;
      sParticle.LinearVelocity += vConstantAcceleration * fDeltaTime;

      it++;
   }

   // new particles
   if(bEmitterActive)
   {
      fElapsedTimeSinceLastEmission += fDeltaTime;

      float fTimeToEmitParticle = 1.0f / fEmissionRate;

      while(fElapsedTimeSinceLastEmission > fTimeToEmitParticle)
      {
         if(iEmissionBurstCount == 1)
            emitParticle();
         else
            burst(iEmissionBurstCount);

         fElapsedTimeSinceLastEmission -= fTimeToEmitParticle;
      }
   }

   composeVertexData();
}

void ComponentParticleSystem::composeVertexData()
{
   Vector3 vCameraRight;
   Vector3 vCameraUp;
   Vector3 vCameraForward;

   if(eRenderingMode == RenderingMode::_3D)
   {
      ComponentCamera* cCamera = RenderSystem::getInstance()->getActiveCamera();
      const Vector3& vCameraWorldPosition = cCamera->getTransform()->getWorldPosition();

      vCameraRight = cCamera->getTransform()->getRightVector();
      vCameraUp = cCamera->getTransform()->getUpVector();
      vCameraForward = cCamera->getTransform()->getForwardVector();

      std::sort(lParticles.begin(), lParticles.end(), [&](const Particle& P1, const Particle& P2) -> bool
      {
         Vector3 vP1ToCamera = vCameraWorldPosition - P1.Position;
         Vector3 vP2ToCamera = vCameraWorldPosition - P2.Position;
         return vP1ToCamera.getSquaredLength() > vP2ToCamera.getSquaredLength();
      });
   }
   else
   {
      vCameraRight = Vector3::UnitX;
      vCameraUp = Vector3::UnitY;
      vCameraForward = Vector3::UnitZ;
   }

   const Texture* cDiffuseTexture = 0;

   if(!vMaterialPassList.empty())
   {
      Material* cMaterial = getMaterialPass(0)->getMaterial();

      if(cMaterial)
      {
         cDiffuseTexture = cMaterial->getDiffuseTexture();
      }
   }

   float* pVertexData = sGeometryData.VertexData;

   for(ParticleList::iterator it = lParticles.begin(); it != lParticles.end(); it++)
   {
      Particle& sParticle = *it;

      const float fParticleHalfSize = sParticle.Size * 0.5f;

      Vector3 vTopLeftPosition = -(vCameraRight * fParticleHalfSize) + (vCameraUp * fParticleHalfSize);
      Vector3 vTopRightPosition = (vCameraRight * fParticleHalfSize) + (vCameraUp * fParticleHalfSize);
      Vector3 vBottomLeftPosition = -(vCameraRight * fParticleHalfSize) - (vCameraUp * fParticleHalfSize);
      Vector3 vBottomRightPosition = (vCameraRight * fParticleHalfSize) - (vCameraUp * fParticleHalfSize);

      Matrix4 mTransform;

      if(fabsf(sParticle.Angle) < GE_EPSILON)
      {
         Matrix4MakeIdentity(&mTransform);
      }
      else
      {
         Rotation cRotation = Rotation(vCameraForward, sParticle.Angle);
         mTransform = cRotation.getRotationMatrix();
      }

      mTransform.m[GE_M4_1_4] = sParticle.Position.X;
      mTransform.m[GE_M4_2_4] = sParticle.Position.Y;
      mTransform.m[GE_M4_3_4] = sParticle.Position.Z;

      Matrix4Transform(mTransform, &vTopLeftPosition);
      Matrix4Transform(mTransform, &vTopRightPosition);
      Matrix4Transform(mTransform, &vBottomLeftPosition);
      Matrix4Transform(mTransform, &vBottomRightPosition);

      TextureCoordinates sTextureCoordinates;

      if(sParticle.TextureAtlasIndex > 0 && cDiffuseTexture)
      {
         uint iTextureAtlasIndex = sParticle.TextureAtlasIndex;

         if(iTextureAtlasIndex > (uint)(cDiffuseTexture->AtlasUV.size() - 1))
            iTextureAtlasIndex = (uint)(cDiffuseTexture->AtlasUV.size() - 1);

         sTextureCoordinates = cDiffuseTexture->AtlasUV[iTextureAtlasIndex].UV;
      }
      else
      {
         sTextureCoordinates.U0 = 0.0f;
         sTextureCoordinates.V0 = 0.0f;
         sTextureCoordinates.U1 = 1.0f;
         sTextureCoordinates.V1 = 1.0f;
      }

      *pVertexData++ = vTopLeftPosition.X; *pVertexData++ = vTopLeftPosition.Y; *pVertexData++ = vTopLeftPosition.Z;
      *pVertexData++ = sParticle.DiffuseColor.Red; *pVertexData++ = sParticle.DiffuseColor.Green; *pVertexData++ = sParticle.DiffuseColor.Blue; *pVertexData++ = sParticle.DiffuseColor.Alpha;
      *pVertexData++ = sTextureCoordinates.U0; *pVertexData++ = sTextureCoordinates.V0;

      *pVertexData++ = vBottomLeftPosition.X; *pVertexData++ = vBottomLeftPosition.Y; *pVertexData++ = vBottomLeftPosition.Z;
      *pVertexData++ = sParticle.DiffuseColor.Red; *pVertexData++ = sParticle.DiffuseColor.Green; *pVertexData++ = sParticle.DiffuseColor.Blue; *pVertexData++ = sParticle.DiffuseColor.Alpha;
      *pVertexData++ = sTextureCoordinates.U0; *pVertexData++ = sTextureCoordinates.V1;

      *pVertexData++ = vTopRightPosition.X; *pVertexData++ = vTopRightPosition.Y; *pVertexData++ = vTopRightPosition.Z;
      *pVertexData++ = sParticle.DiffuseColor.Red; *pVertexData++ = sParticle.DiffuseColor.Green; *pVertexData++ = sParticle.DiffuseColor.Blue; *pVertexData++ = sParticle.DiffuseColor.Alpha;
      *pVertexData++ = sTextureCoordinates.U1; *pVertexData++ = sTextureCoordinates.V0;

      *pVertexData++ = vBottomRightPosition.X; *pVertexData++ = vBottomRightPosition.Y; *pVertexData++ = vBottomRightPosition.Z;
      *pVertexData++ = sParticle.DiffuseColor.Red; *pVertexData++ = sParticle.DiffuseColor.Green; *pVertexData++ = sParticle.DiffuseColor.Blue; *pVertexData++ = sParticle.DiffuseColor.Alpha;
      *pVertexData++ = sTextureCoordinates.U1; *pVertexData++ = sTextureCoordinates.V1;
   }

   sGeometryData.NumVertices = (uint)lParticles.size() * 4;
   sGeometryData.NumIndices = (uint)lParticles.size() * 6;
}

float ComponentParticleSystem::getRandomFloat(float fMin, float fMax)
{
   float fDiff = fMax - fMin;

   if(fabsf(fDiff) < GE_EPSILON)
      return fMin;

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
