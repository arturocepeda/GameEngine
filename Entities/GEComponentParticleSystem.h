
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
#include "Content/GEResourcesManager.h"
#include "Core/GEThreads.h"
#include "Core/GEEvents.h"
#include "Types/GECurve.h"

namespace GE { namespace Content
{
   class Mesh;
}}

namespace GE { namespace Rendering
{
   class Font;
}}

namespace GE { namespace Entities
{
   GESerializableEnum(ParticleType)
   {
      Billboard,
      Mesh,
      TextBillboard,
      Text3D,

      Count
   };


   GESerializableEnum(ParticleSystemSettingsBitMask)
   {
      Prewarm         = 1 << 0,
      DynamicShadows  = 1 << 1,
      LocalSpace      = 1 << 2,

      Count = 3
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
      Vector3 Angle;
      Vector3 Scale;
      Color DiffuseColor;
      float Size;
      uint32_t TextureAtlasIndex;

      Vector3 LinearVelocity;
      Vector3 AngularVelocity;

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
# define GEValueProvider(pPropertyBaseName) \
   private: \
      ValueProviderType m##pPropertyBaseName##Type; \
      float m##pPropertyBaseName##Value; \
      float m##pPropertyBaseName##ValueMax; \
      Curve* m##pPropertyBaseName##Curve; \
   public: \
      ValueProviderType get##pPropertyBaseName##Type() const { return m##pPropertyBaseName##Type; } \
      float get##pPropertyBaseName##Value() const { return m##pPropertyBaseName##Value; } \
      float get##pPropertyBaseName##ValueMax() const { return m##pPropertyBaseName##ValueMax; } \
      const Core::ObjectName& get##pPropertyBaseName##Curve() const \
      { \
         return m##pPropertyBaseName##Curve ? m##pPropertyBaseName##Curve->getName() : Core::ObjectName::Empty; \
      } \
      void set##pPropertyBaseName##Type(ValueProviderType v) \
      { \
         m##pPropertyBaseName##Type = v; \
         switch(m##pPropertyBaseName##Type) \
         { \
         case ValueProviderType::Constant: \
           m##pPropertyBaseName##Curve = nullptr; \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"Value"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"ValueMax"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"Curve"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           break; \
         case ValueProviderType::Random: \
           m##pPropertyBaseName##Curve = nullptr; \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"Value"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"ValueMax"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"Curve"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           break; \
         case ValueProviderType::Curve: \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"Value"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"ValueMax"))->Flags |= (uint)Core::PropertyFlags::Hidden; \
           const_cast<Core::Property*>(getProperty(#pPropertyBaseName"Curve"))->Flags &= ~((uint)Core::PropertyFlags::Hidden); \
           break; \
         } \
         Core::EventArgs args; \
         args.Data = this; \
         Core::EventHandlingObject::triggerEventStatic(Core::Events::PropertiesUpdated, &args); \
      } \
      void set##pPropertyBaseName##Value(float v) { m##pPropertyBaseName##Value = v; } \
      void set##pPropertyBaseName##ValueMax(float v) { m##pPropertyBaseName##ValueMax = v; } \
      void set##pPropertyBaseName##Curve(const Core::ObjectName& v) \
      { \
         const Core::ObjectRegistry* registry = Content::ResourcesManager::getInstance()->getObjectRegistry("Curve"); \
         Core::ObjectRegistry::const_iterator it = registry->find(v.getID()); \
         if(it != registry->end()) \
         { \
            m##pPropertyBaseName##Curve = static_cast<Curve*>(it->second); \
         } \
      } \
      float get##pPropertyBaseName(float pLifeTime, float pRemainingLifeTime) \
      { \
         switch(m##pPropertyBaseName##Type) \
         { \
         case ValueProviderType::Constant: \
            return m##pPropertyBaseName##Value; \
         case ValueProviderType::Random: \
            return getRandomFloat(m##pPropertyBaseName##Value, m##pPropertyBaseName##ValueMax); \
         case ValueProviderType::Curve: \
            if(m##pPropertyBaseName##Curve) \
            { \
               const float curveLength = m##pPropertyBaseName##Curve->getLength(); \
               if(curveLength > GE_EPSILON) \
               { \
                  const float relativeTimePosition = (pLifeTime - pRemainingLifeTime) / pLifeTime; \
                  return m##pPropertyBaseName##Curve->getValue(relativeTimePosition * curveLength); \
               } \
            } \
            break; \
         } \
         return m##pPropertyBaseName##Value; \
      }
#else
# define GEValueProvider(pPropertyBaseName) \
   private: \
      ValueProviderType m##pPropertyBaseName##Type; \
      float m##pPropertyBaseName##Value; \
      float m##pPropertyBaseName##ValueMax; \
      Curve* m##pPropertyBaseName##Curve; \
   public: \
      ValueProviderType get##pPropertyBaseName##Type() const { return m##pPropertyBaseName##Type; } \
      float get##pPropertyBaseName##Value() const { return m##pPropertyBaseName##Value; } \
      float get##pPropertyBaseName##ValueMax() const { return m##pPropertyBaseName##ValueMax; } \
      const Core::ObjectName& get##pPropertyBaseName##Curve() const \
      { \
         return m##pPropertyBaseName##Curve ? m##pPropertyBaseName##Curve->getName() : Core::ObjectName::Empty; \
      } \
      void set##pPropertyBaseName##Type(ValueProviderType v) \
      { \
         m##pPropertyBaseName##Type = v; \
         switch(m##pPropertyBaseName##Type) \
         { \
         case ValueProviderType::Constant: \
         case ValueProviderType::Random: \
           m##pPropertyBaseName##Curve = nullptr; \
           break; \
         } \
         Core::EventArgs args; \
         args.Data = this; \
         Core::EventHandlingObject::triggerEventStatic(Core::Events::PropertiesUpdated, &args); \
      } \
      void set##pPropertyBaseName##Value(float v) { m##pPropertyBaseName##Value = v; } \
      void set##pPropertyBaseName##ValueMax(float v) { m##pPropertyBaseName##ValueMax = v; } \
      void set##pPropertyBaseName##Curve(const Core::ObjectName& v) \
      { \
         const Core::ObjectRegistry* registry = Content::ResourcesManager::getInstance()->getObjectRegistry("Curve"); \
         Core::ObjectRegistry::const_iterator it = registry->find(v.getID()); \
         if(it != registry->end()) \
         { \
            m##pPropertyBaseName##Curve = static_cast<Curve*>(it->second); \
         } \
      } \
      float get##pPropertyBaseName(float pLifeTime, float pRemainingLifeTime) \
      { \
         switch(m##pPropertyBaseName##Type) \
         { \
         case ValueProviderType::Constant: \
            return m##pPropertyBaseName##Value; \
         case ValueProviderType::Random: \
            return getRandomFloat(m##pPropertyBaseName##Value, m##pPropertyBaseName##ValueMax); \
         case ValueProviderType::Curve: \
            if(m##pPropertyBaseName##Curve) \
            { \
               const float curveLength = m##pPropertyBaseName##Curve->getLength(); \
               if(curveLength > GE_EPSILON) \
               { \
                  const float relativeTimePosition = (pLifeTime - pRemainingLifeTime) / pLifeTime; \
                  return m##pPropertyBaseName##Curve->getValue(relativeTimePosition * curveLength); \
               } \
            } \
            break; \
         } \
         return m##pPropertyBaseName##Value; \
      }
#endif


   class ComponentParticleSystem : public ComponentRenderable
   {
   private:
      typedef GESTLVector(Particle) ParticleList;
      ParticleList lParticles;
      uint iMaxParticles;
      float fElapsedTimeSinceLastEmission;
      bool bVertexDataReallocationPending;
      std::atomic<bool> mBurstPending;

      ParticleType mParticleType;
      ParticleEmitterType eEmitterType;
      Vector3 vEmitterPointA;
      Vector3 vEmitterPointB;
      float fEmitterRadius;
      Content::Mesh* mParticleMesh;
      Content::Mesh* cEmitterMesh;
      Entity* cEmitterMeshEntity;
      GESTLString mParticleText;
      Rendering::Font* mParticleTextFont;
      float mParticleTextSize;

      bool bEmitterActive;
      uint8_t mSettings;
      float fEmissionRate;
      uint iEmissionBurstCount;

      float fParticleLifeTimeMin;
      float fParticleLifeTimeMax;

      Vector3 mParticleInitialAngleMin;
      Vector3 mParticleInitialAngleMax;

      float fParticleSizeMultiplier;

      Vector3 vConstantForce;
      Vector3 vConstantAcceleration;
      Vector3 mTurbulenceFactor;
      float mFrictionFactor;

      void simulate(float pDeltaTime);
      void prewarm();

      void allocateVertexData();
      void composeVertexData();
      void composeBillboardVertexData();
      void composeMeshVertexData();
      void composeTextVertexData();

      uint32_t getParticleTextLength() const;
      float getParticleTextCharWidth(size_t pCharIndex) const;

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

      void setParticleMesh(const Core::ObjectName& pMeshName);
      void setParticleText(const char* pText);
      void setParticleTextFontName(const Core::ObjectName& pFontName);
      void setParticleTextSize(float pSize);

      void setEmitterPointA(const Vector3& Point);
      void setEmitterPointB(const Vector3& Point);
      void setEmitterRadius(float Radius);
      void setEmitterMesh(const Core::ObjectName& MeshName);
      void setEmitterMeshEntity(const Core::ObjectName& EntityName);

      const Core::ObjectName& getParticleMesh() const;
      const char* getParticleText() const;
      const Core::ObjectName& getParticleTextFontName() const;
      Rendering::Font* getParticleTextFont() const;
      float getParticleTextSize() const;

      const Vector3& getEmitterPointA() const;
      const Vector3& getEmitterPointB() const;
      float getEmitterRadius() const;
      const Core::ObjectName& getEmitterMesh() const;
      const Core::ObjectName& getEmitterMeshEntity() const;

      void emitParticle();
      void burst(uint NumParticles);

      void update();

      GEDefaultGetter(ParticleType, ParticleType, m);
      ParticleEmitterType getEmitterType() const { return eEmitterType; }
      bool getEmitterActive() const { return bEmitterActive; }
      uint8_t getSettings() const { return mSettings; }
      float getEmissionRate() const { return fEmissionRate; }
      uint getEmissionBurstCount() const { return iEmissionBurstCount; }

      float getParticleLifeTimeMin() const { return fParticleLifeTimeMin; }
      float getParticleLifeTimeMax() const { return fParticleLifeTimeMax; }

      const Vector3& getParticleInitialAngleMin() const { return mParticleInitialAngleMin; }
      const Vector3& getParticleInitialAngleMax() const { return mParticleInitialAngleMax; }

      float getParticleSizeMultiplier() const { return fParticleSizeMultiplier; }

      const Vector3& getConstantForce() const { return vConstantForce; }
      const Vector3& getConstantAcceleration() const { return vConstantAcceleration; }
      GEDefaultGetter(const Vector3&, TurbulenceFactor, m);
      GEDefaultGetter(float, FrictionFactor, m);

      void setParticleType(ParticleType pParticleType);
      void setEmitterType(ParticleEmitterType EmitterType) { eEmitterType = EmitterType; }
      void setEmitterActive(bool Active);
      void setSettings(uint8_t pValue) { mSettings = pValue; }
      void setEmissionRate(float Rate) { fEmissionRate = Rate; }
      void setEmissionBurstCount(uint Value) { iEmissionBurstCount = Value; }

      void setParticleLifeTimeMin(float Value) { fParticleLifeTimeMin = Value; }
      void setParticleLifeTimeMax(float Value) { fParticleLifeTimeMax = Value; }

      void setParticleInitialAngleMin(const Vector3& Value) { mParticleInitialAngleMin = Value; }
      void setParticleInitialAngleMax(const Vector3& Value) { mParticleInitialAngleMax = Value; }

      void setParticleSizeMultiplier(float Value) { fParticleSizeMultiplier = Value; }

      void setConstantForce(const Vector3& Value) { vConstantForce = Value; }
      void setConstantAcceleration(const Vector3& Value) { vConstantAcceleration = Value; }
      GEDefaultSetter(const Vector3&, TurbulenceFactor, m);
      GEDefaultSetter(float, FrictionFactor, m);

      GEValueProvider(ParticleColorR)
      GEValueProvider(ParticleColorG)
      GEValueProvider(ParticleColorB)
      GEValueProvider(ParticleAlpha)
      GEValueProvider(ParticleSize)
      GEValueProvider(ParticleScaleX)
      GEValueProvider(ParticleScaleY)
      GEValueProvider(ParticleScaleZ)
      GEValueProvider(ParticleLinearVelocityX)
      GEValueProvider(ParticleLinearVelocityY)
      GEValueProvider(ParticleLinearVelocityZ)
      GEValueProvider(ParticleAngularVelocityX)
      GEValueProvider(ParticleAngularVelocityY)
      GEValueProvider(ParticleAngularVelocityZ)
      GEValueProvider(ParticleTextureAtlasIndex)
   };
}}
