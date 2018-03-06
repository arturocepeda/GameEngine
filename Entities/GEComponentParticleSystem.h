
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentParticleSystem.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentRenderable.h"
#include "Content/GEMesh.h"
#include "Content/GEResourcesManager.h"
#include "Core/GEThreads.h"
#include "Core/GEEvents.h"
#include "Types/GECurve.h"

namespace GE { namespace Entities
{
   GESerializableEnum(ParticleEmitterType)
   {
      Point,
      Sphere,
      SphereSurface,
      Line,
      Mesh,

      Count
   };


   enum class ParticleSettingsBitMask : unsigned char
   {
      VaryColorR           =  1 << 0,
      VaryColorG           =  1 << 1,
      VaryColorB           =  1 << 2,
      VaryAlpha            =  1 << 3,
      VarySize             =  1 << 4,
      VaryLinearVelocity   =  1 << 5,
      VaryAngularVelocity  =  1 << 6,
   };


   struct Particle
   {
      Vector3 Position;
      float Size;
      float Angle;
      Color DiffuseColor;
      uint TextureAtlasIndex;

      Vector3 LinearVelocity;
      float AngularVelocity;
      Color DiffuseColorVariation;

      float LifeTime;
      float RemainingLifeTime;

      uint8_t Settings;
   };


   GESerializableEnum(ValueProviderType)
   {
      Constant,
      Random,
      Curve,

      Count
   };


#define GEValueProvider(PropertyBaseName) \
   private: \
      ValueProviderType e##PropertyBaseName##Type; \
      float f##PropertyBaseName##Value; \
      float f##PropertyBaseName##ValueMax; \
      Curve* c##PropertyBaseName##Curve; \
   public: \
      ValueProviderType get##PropertyBaseName##Type() const { return e##PropertyBaseName##Type; } \
      float get##PropertyBaseName##Value() const { return f##PropertyBaseName##Value; } \
      float get##PropertyBaseName##ValueMax() const { return f##PropertyBaseName##ValueMax; } \
      const Core::ObjectName& get##PropertyBaseName##Curve() const \
      { \
         return c##PropertyBaseName##Curve ? c##PropertyBaseName##Curve->getName() : Core::ObjectName::Empty; \
      } \
      void set##PropertyBaseName##Type(ValueProviderType v) \
      { \
         e##PropertyBaseName##Type = v; \
         switch(e##PropertyBaseName##Type) \
         { \
         case ValueProviderType::Constant: \
           c##PropertyBaseName##Curve = 0; \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"Value"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"ValueMax"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"Curve"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           break; \
         case ValueProviderType::Random: \
           c##PropertyBaseName##Curve = 0; \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"Value"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"ValueMax"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"Curve"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           break; \
         case ValueProviderType::Curve: \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"Value"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"ValueMax"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"Curve"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           break; \
         } \
         Core::EventHandlingObject::triggerEventStatic(Core::Events::PropertiesUpdated); \
      } \
      void set##PropertyBaseName##Value(float v) { f##PropertyBaseName##Value = v; } \
      void set##PropertyBaseName##ValueMax(float v) { f##PropertyBaseName##ValueMax = v; } \
      void set##PropertyBaseName##Curve(const Core::ObjectName& v) \
      { \
         const Core::ObjectRegistry* cRegistry = Content::ResourcesManager::getInstance()->getObjectRegistry("Curve"); \
         Core::ObjectRegistry::const_iterator it = cRegistry->find(v.getID()); \
         if(it != cRegistry->end()) \
         { \
            c##PropertyBaseName##Curve = static_cast<Curve*>(it->second); \
         } \
      } \
      float get##PropertyBaseName##(float LifeTime, float RemainingLifeTime) \
      { \
         switch(e##PropertyBaseName##Type) \
         { \
         case ValueProviderType::Constant: \
            return f##PropertyBaseName##Value; \
         case ValueProviderType::Random: \
            return getRandomFloat(f##PropertyBaseName##Value, f##PropertyBaseName##ValueMax); \
         case ValueProviderType::Curve: \
            if(c##PropertyBaseName##Curve) \
            { \
               const float fCurveLength = c##PropertyBaseName##Curve->getLength(); \
               if(fCurveLength > GE_EPSILON) \
               { \
                  const float fRelativeTimePosition = (LifeTime - RemainingLifeTime) / LifeTime; \
                  return c##PropertyBaseName##Curve->getValue(fRelativeTimePosition * fCurveLength); \
               } \
            } \
            break; \
         } \
         return f##PropertyBaseName##Value; \
      } \
      GEPropertyEnum(ValueProviderType, PropertyBaseName##Type) \
      GEProperty(Float, PropertyBaseName##Value) \
      GEProperty(Float, PropertyBaseName##ValueMax) \
      GEProperty(ObjectName, PropertyBaseName##Curve)


   class ComponentParticleSystem : public ComponentRenderable
   {
   private:
      typedef GESTLVector(Particle) ParticleList;
      ParticleList lParticles;
      uint iMaxParticles;
      bool bVertexDataReallocationPending;
      float fElapsedTimeSinceLastEmission;

      ParticleEmitterType eEmitterType;
      Vector3 vEmitterPointA;
      Vector3 vEmitterPointB;
      float fEmitterRadius;
      Content::Mesh* cEmitterMesh;
      Entity* cEmitterMeshEntity;

      bool bEmitterActive;
      bool bDynamicShadows;
      float fEmissionRate;
      uint iEmissionBurstCount;

      float fParticleLifeTimeMin;
      float fParticleLifeTimeMax;

      float fParticleInitialAngleMin;
      float fParticleInitialAngleMax;

      Vector3 vParticleLinearVelocityMin;
      Vector3 vParticleLinearVelocityMax;
      float fParticleAngularVelocityMin;
      float fParticleAngularVelocityMax;

      uint iParticleTextureAtlasIndexMin;
      uint iParticleTextureAtlasIndexMax;

      Vector3 vConstantForce;
      Vector3 vConstantAcceleration;

      void allocateVertexData();
      void composeVertexData();

      static float getRandomFloat(float fMin, float fMax);
      static Vector3 getRandomVector3(const Vector3& vMin, const Vector3& vMax);
      static Color getRandomColor(const Color& cMin, const Color& cMax);

      static Vector3 getRandomPointInTriangle(const Vector3& P1, const Vector3& P2, const Vector3& P3);

   public:
      ComponentParticleSystem(Entity* Owner);
      ~ComponentParticleSystem();

      uint getMaxParticles() const;
      void setMaxParticles(uint MaxParticles);

      uint getParticlesCount() const { return (uint)lParticles.size(); }

      void setEmitterPointA(const Vector3& Point);
      void setEmitterPointB(const Vector3& Point);
      void setEmitterRadius(float Radius);
      void setEmitterMesh(const Core::ObjectName& MeshName);
      void setEmitterMeshEntity(const Core::ObjectName& EntityName);

      const Vector3& getEmitterPointA() const;
      const Vector3& getEmitterPointB() const;
      float getEmitterRadius() const;
      const Core::ObjectName& getEmitterMesh() const;
      const Core::ObjectName& getEmitterMeshEntity() const;

      void emitParticle();
      void burst(uint NumParticles);

      void update();

      ParticleEmitterType getEmitterType() const { return eEmitterType; }
      bool getEmitterActive() const { return bEmitterActive; }
      bool getDynamicShadows() const { return bDynamicShadows; }
      float getEmissionRate() const { return fEmissionRate; }
      uint getEmissionBurstCount() const { return iEmissionBurstCount; }

      float getParticleLifeTimeMin() const { return fParticleLifeTimeMin; }
      float getParticleLifeTimeMax() const { return fParticleLifeTimeMax; }

      float getParticleInitialAngleMin() const { return fParticleInitialAngleMin; }
      float getParticleInitialAngleMax() const { return fParticleInitialAngleMax; }

      const Vector3& getParticleLinearVelocityMin() const { return vParticleLinearVelocityMin; }
      const Vector3& getParticleLinearVelocityMax() const { return vParticleLinearVelocityMax; }
      float getParticleAngularVelocityMin() const { return fParticleAngularVelocityMin; }
      float getParticleAngularVelocityMax() const { return fParticleAngularVelocityMax; }

      uint getParticleTextureAtlasIndexMin() const { return iParticleTextureAtlasIndexMin; }
      uint getParticleTextureAtlasIndexMax() const { return iParticleTextureAtlasIndexMax; }

      const Vector3& getConstantForce() const { return vConstantForce; }
      const Vector3& getConstantAcceleration() const { return vConstantAcceleration; }

      void setEmitterType(ParticleEmitterType EmitterType) { eEmitterType = EmitterType; }
      void setEmitterActive(bool Active) { bEmitterActive = Active; }
      void setDynamicShadows(bool Active) { bDynamicShadows = Active; }
      void setEmissionRate(float Rate) { fEmissionRate = Rate; }
      void setEmissionBurstCount(uint Value) { iEmissionBurstCount = Value; }

      void setParticleLifeTimeMin(float Value) { fParticleLifeTimeMin = Value; }
      void setParticleLifeTimeMax(float Value) { fParticleLifeTimeMax = Value; }

      void setParticleInitialAngleMin(float Value) { fParticleInitialAngleMin = Value; }
      void setParticleInitialAngleMax(float Value) { fParticleInitialAngleMax = Value; }

      void setParticleLinearVelocityMin(const Vector3& Value) { vParticleLinearVelocityMin = Value; }
      void setParticleLinearVelocityMax(const Vector3& Value) { vParticleLinearVelocityMax = Value; }
      void setParticleAngularVelocityMin(float Value) { fParticleAngularVelocityMin = Value; }
      void setParticleAngularVelocityMax(float Value) { fParticleAngularVelocityMax = Value; }

      void setParticleTextureAtlasIndexMin(uint Value) { iParticleTextureAtlasIndexMin = Value; }
      void setParticleTextureAtlasIndexMax(uint Value) { iParticleTextureAtlasIndexMax = Value; }

      void setConstantForce(const Vector3& Value) { vConstantForce = Value; }
      void setConstantAcceleration(const Vector3& Value) { vConstantAcceleration = Value; }

      GEProperty(UInt, MaxParticles)
      GEPropertyReadonly(UInt, ParticlesCount)

      GEPropertyEnum(ParticleEmitterType, EmitterType)

      // line emitters
      GEProperty(Vector3, EmitterPointA)
      GEProperty(Vector3, EmitterPointB)
      // sphere emitters
      GEProperty(Float, EmitterRadius)
      // mesh emitters
      GEProperty(ObjectName, EmitterMesh)
      GEProperty(ObjectName, EmitterMeshEntity)

      GEProperty(Bool, EmitterActive)
      GEProperty(Bool, DynamicShadows)
      GEProperty(Float, EmissionRate)
      GEProperty(UInt, EmissionBurstCount)

      GEProperty(Float, ParticleLifeTimeMin)
      GEProperty(Float, ParticleLifeTimeMax)

      GEProperty(Float, ParticleInitialAngleMin)
      GEProperty(Float, ParticleInitialAngleMax)

      GEProperty(Vector3, ParticleLinearVelocityMin)
      GEProperty(Vector3, ParticleLinearVelocityMax)
      GEProperty(Float, ParticleAngularVelocityMin)
      GEProperty(Float, ParticleAngularVelocityMax)

      GEProperty(UInt, ParticleTextureAtlasIndexMin)
      GEProperty(UInt, ParticleTextureAtlasIndexMax)

      GEProperty(Vector3, ConstantForce)
      GEProperty(Vector3, ConstantAcceleration)

      GEValueProvider(ParticleColorR)
      GEValueProvider(ParticleColorG)
      GEValueProvider(ParticleColorB)
      GEValueProvider(ParticleAlpha)
      GEValueProvider(ParticleSize)
   };
}}
