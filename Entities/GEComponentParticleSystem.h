
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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
   GESerializableEnum(ParticleSystemSettingsBitMask)
   {
      Prewarm         = 1 << 0,
      DynamicShadows  = 1 << 1,

      Count = 2
   };


   GESerializableEnum(ParticleEmitterType)
   {
      Point,
      Sphere,
      SphereSurface,
      Line,
      Mesh,

      Count
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


#if defined (GE_EDITOR_SUPPORT)
# define GEValueProvider(PropertyBaseName) \
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
         Core::EventArgs sArgs; \
         sArgs.Data = this; \
         Core::EventHandlingObject::triggerEventStatic(Core::Events::PropertiesUpdated, &sArgs); \
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
      float get##PropertyBaseName(float LifeTime, float RemainingLifeTime) \
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
      }
#else
# define GEValueProvider(PropertyBaseName) \
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
         case ValueProviderType::Random: \
           c##PropertyBaseName##Curve = 0; \
           break; \
         } \
         Core::EventArgs sArgs; \
         sArgs.Data = this; \
         Core::EventHandlingObject::triggerEventStatic(Core::Events::PropertiesUpdated, &sArgs); \
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
      float get##PropertyBaseName(float LifeTime, float RemainingLifeTime) \
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
      }
#endif


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
      uint8_t mSettings;
      float fEmissionRate;
      uint iEmissionBurstCount;

      float fParticleLifeTimeMin;
      float fParticleLifeTimeMax;

      float fParticleInitialAngleMin;
      float fParticleInitialAngleMax;

      float fParticleSizeMultiplier;

      Vector3 vConstantForce;
      Vector3 vConstantAcceleration;

      void simulate(float pDeltaTime);
      void prewarm();

      void allocateVertexData();
      void composeVertexData();

      static float getRandomFloat(float fMin, float fMax);
      static Vector3 getRandomVector3(const Vector3& vMin, const Vector3& vMax);
      static Color getRandomColor(const Color& cMin, const Color& cMax);

      static Vector3 getRandomPointInTriangle(const Vector3& P1, const Vector3& P2, const Vector3& P3);

   public:
      static const Core::ObjectName ClassName;

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
      uint8_t getSettings() const { return mSettings; }
      float getEmissionRate() const { return fEmissionRate; }
      uint getEmissionBurstCount() const { return iEmissionBurstCount; }

      float getParticleLifeTimeMin() const { return fParticleLifeTimeMin; }
      float getParticleLifeTimeMax() const { return fParticleLifeTimeMax; }

      float getParticleInitialAngleMin() const { return fParticleInitialAngleMin; }
      float getParticleInitialAngleMax() const { return fParticleInitialAngleMax; }

      float getParticleSizeMultiplier() const { return fParticleSizeMultiplier; }

      const Vector3& getConstantForce() const { return vConstantForce; }
      const Vector3& getConstantAcceleration() const { return vConstantAcceleration; }

      void setEmitterType(ParticleEmitterType EmitterType) { eEmitterType = EmitterType; }
      void setEmitterActive(bool Active) { bEmitterActive = Active; fElapsedTimeSinceLastEmission = fEmissionRate; }
      void setSettings(uint8_t pValue) { mSettings = pValue; }
      void setEmissionRate(float Rate) { fEmissionRate = Rate; }
      void setEmissionBurstCount(uint Value) { iEmissionBurstCount = Value; }

      void setParticleLifeTimeMin(float Value) { fParticleLifeTimeMin = Value; }
      void setParticleLifeTimeMax(float Value) { fParticleLifeTimeMax = Value; }

      void setParticleInitialAngleMin(float Value) { fParticleInitialAngleMin = Value; }
      void setParticleInitialAngleMax(float Value) { fParticleInitialAngleMax = Value; }

      void setParticleSizeMultiplier(float Value) { fParticleSizeMultiplier = Value; }

      void setConstantForce(const Vector3& Value) { vConstantForce = Value; }
      void setConstantAcceleration(const Vector3& Value) { vConstantAcceleration = Value; }

      GEValueProvider(ParticleColorR)
      GEValueProvider(ParticleColorG)
      GEValueProvider(ParticleColorB)
      GEValueProvider(ParticleAlpha)
      GEValueProvider(ParticleSize)
      GEValueProvider(ParticleLinearVelocityX)
      GEValueProvider(ParticleLinearVelocityY)
      GEValueProvider(ParticleLinearVelocityZ)
      GEValueProvider(ParticleAngularVelocity)
      GEValueProvider(ParticleTextureAtlasIndex)
   };
}}
