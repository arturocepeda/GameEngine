
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


   GESerializableEnum(ParticleSettingsBitMask)
   {
      VaryAlpha  =  1 << 0,
      VaryColor  =  1 << 1,
      VarySize   =  1 << 2,

      Count = 3
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
      float SizeVariation;
      Color DiffuseColorVariation;

      float LifeTime;
      float RemainingLifeTime;
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
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"Value"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"ValueMax"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           const_cast<Core::Property*>(getProperty(#PropertyBaseName"Curve"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           break; \
         case ValueProviderType::Random: \
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
      float get##PropertyBaseName##(float TimePosition) \
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
               return c##PropertyBaseName##Curve->getValue(TimePosition); \
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

      uint8_t eParticleSettings;

      float fParticleLifeTimeMin;
      float fParticleLifeTimeMax;

      float fParticleInitialSizeMin;
      float fParticleInitialSizeMax;
      Color cParticleInitialColorMin;
      Color cParticleInitialColorMax;
      float fParticleInitialAngleMin;
      float fParticleInitialAngleMax;

      Vector3 vParticleLinearVelocityMin;
      Vector3 vParticleLinearVelocityMax;
      float fParticleAngularVelocityMin;
      float fParticleAngularVelocityMax;

      float fParticleFinalSizeMin;
      float fParticleFinalSizeMax;
      Color cParticleFinalColorMin;
      Color cParticleFinalColorMax;

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

      uint8_t getParticleSettings() const { return eParticleSettings; }

      float getParticleLifeTimeMin() const { return fParticleLifeTimeMin; }
      float getParticleLifeTimeMax() const { return fParticleLifeTimeMax; }

      float getParticleInitialSizeMin() const { return fParticleInitialSizeMin; }
      float getParticleInitialSizeMax() const { return fParticleInitialSizeMax; }
      const Color& getParticleInitialColorMin() const { return cParticleInitialColorMin; }
      const Color& getParticleInitialColorMax() const { return cParticleInitialColorMax; }
      float getParticleInitialAngleMin() const { return fParticleInitialAngleMin; }
      float getParticleInitialAngleMax() const { return fParticleInitialAngleMax; }

      const Vector3& getParticleLinearVelocityMin() const { return vParticleLinearVelocityMin; }
      const Vector3& getParticleLinearVelocityMax() const { return vParticleLinearVelocityMax; }
      float getParticleAngularVelocityMin() const { return fParticleAngularVelocityMin; }
      float getParticleAngularVelocityMax() const { return fParticleAngularVelocityMax; }

      float getParticleFinalSizeMin() const { return fParticleFinalSizeMin; }
      float getParticleFinalSizeMax() const { return fParticleFinalSizeMax; }
      const Color& getParticleFinalColorMin() const { return cParticleFinalColorMin; }
      const Color& getParticleFinalColorMax() const { return cParticleFinalColorMax; }

      uint getParticleTextureAtlasIndexMin() const { return iParticleTextureAtlasIndexMin; }
      uint getParticleTextureAtlasIndexMax() const { return iParticleTextureAtlasIndexMax; }

      const Vector3& getConstantForce() const { return vConstantForce; }
      const Vector3& getConstantAcceleration() const { return vConstantAcceleration; }

      void setEmitterType(ParticleEmitterType EmitterType) { eEmitterType = EmitterType; }
      void setEmitterActive(bool Active) { bEmitterActive = Active; }
      void setDynamicShadows(bool Active) { bDynamicShadows = Active; }
      void setEmissionRate(float Rate) { fEmissionRate = Rate; }
      void setEmissionBurstCount(uint Value) { iEmissionBurstCount = Value; }

      void setParticleSettings(uint8_t Value) { eParticleSettings = Value; }

      void setParticleLifeTimeMin(float Value) { fParticleLifeTimeMin = Value; }
      void setParticleLifeTimeMax(float Value) { fParticleLifeTimeMax = Value; }

      void setParticleInitialSizeMin(float Value) { fParticleInitialSizeMin = Value; }
      void setParticleInitialSizeMax(float Value) { fParticleInitialSizeMax = Value; }
      void setParticleInitialColorMin(const Color& Value) { cParticleInitialColorMin = Value; }
      void setParticleInitialColorMax(const Color& Value) { cParticleInitialColorMax = Value; }
      void setParticleInitialAngleMin(float Value) { fParticleInitialAngleMin = Value; }
      void setParticleInitialAngleMax(float Value) { fParticleInitialAngleMax = Value; }

      void setParticleLinearVelocityMin(const Vector3& Value) { vParticleLinearVelocityMin = Value; }
      void setParticleLinearVelocityMax(const Vector3& Value) { vParticleLinearVelocityMax = Value; }
      void setParticleAngularVelocityMin(float Value) { fParticleAngularVelocityMin = Value; }
      void setParticleAngularVelocityMax(float Value) { fParticleAngularVelocityMax = Value; }

      void setParticleFinalSizeMin(float Value) { fParticleFinalSizeMin = Value; }
      void setParticleFinalSizeMax(float Value) { fParticleFinalSizeMax = Value; }
      void setParticleFinalColorMin(const Color& Value) { cParticleFinalColorMin = Value; }
      void setParticleFinalColorMax(const Color& Value) { cParticleFinalColorMax = Value; }

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

      GEPropertyBitMask(ParticleSettingsBitMask, ParticleSettings)

      GEProperty(Float, ParticleLifeTimeMin)
      GEProperty(Float, ParticleLifeTimeMax)

      GEProperty(Float, ParticleInitialSizeMin)
      GEProperty(Float, ParticleInitialSizeMax)
      GEProperty(Color, ParticleInitialColorMin)
      GEProperty(Color, ParticleInitialColorMax)
      GEProperty(Float, ParticleInitialAngleMin)
      GEProperty(Float, ParticleInitialAngleMax)

      GEProperty(Vector3, ParticleLinearVelocityMin)
      GEProperty(Vector3, ParticleLinearVelocityMax)
      GEProperty(Float, ParticleAngularVelocityMin)
      GEProperty(Float, ParticleAngularVelocityMax)

      GEProperty(Float, ParticleFinalSizeMin)
      GEProperty(Float, ParticleFinalSizeMax)
      GEProperty(Color, ParticleFinalColorMin)
      GEProperty(Color, ParticleFinalColorMax)

      GEProperty(UInt, ParticleTextureAtlasIndexMin)
      GEProperty(UInt, ParticleTextureAtlasIndexMax)

      GEProperty(Vector3, ConstantForce)
      GEProperty(Vector3, ConstantAcceleration)

      GEValueProvider(Test)
      GEValueProvider(Foo)
   };
}}
