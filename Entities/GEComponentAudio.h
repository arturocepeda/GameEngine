
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentAudio.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponent.h"
#include "GEComponentType.h"


namespace GE { namespace Audio
{
   struct AudioEventInstance;
}}


namespace GE { namespace Entities
{
   class ComponentAudio : public Component
   {
   protected:
      ComponentAudio(Entity* Owner);

   public:
      static ComponentType getType() { return ComponentType::Audio; }

      virtual void update() = 0;
   };


   class ComponentAudioListener : public ComponentAudio
   {
   private:
      bool mActive;

   public:
      ComponentAudioListener(Entity* Owner);
      ~ComponentAudioListener();

      GEDefaultGetter(bool, Active)
      GEDefaultSetter(bool, Active)

      virtual void update() override;
   };


   class ComponentAudioSource : public ComponentAudio
   {
   protected:
      Core::ObjectName mAudioBankName;
      Core::ObjectName mAudioEventName;

      GESTLVector(Audio::AudioEventInstance*) mAudioEventInstances;

      ComponentAudioSource(Entity* Owner);
      virtual ~ComponentAudioSource();

      virtual void onAudioEventPlayed(Audio::AudioEventInstance* pAudioEventInstance) = 0;

   public:
      GEDefaultGetter(const Core::ObjectName&, AudioBankName)
      GEDefaultGetter(const Core::ObjectName&, AudioEventName)

      GEDefaultSetter(const Core::ObjectName&, AudioBankName)
      GEDefaultSetter(const Core::ObjectName&, AudioEventName)

      Audio::AudioEventInstance* playAudioEvent(const Core::ObjectName& pAudioEventName);
      void stopAllAudioEvents();

      virtual void update() override;
   };


   class ComponentAudioSource2D : public ComponentAudioSource
   {
   protected:
      float mVolume;

      virtual void onAudioEventPlayed(Audio::AudioEventInstance* pAudioEventInstance) override;

   public:
      ComponentAudioSource2D(Entity* pOwner);
      ~ComponentAudioSource2D();

      GEDefaultGetter(float, Volume)

      void setVolume(float pValue);
   };


   class ComponentAudioSource3D : public ComponentAudioSource
   {
   protected:
      float mMinDistance;
      float mMaxDistance;

      virtual void onAudioEventPlayed(Audio::AudioEventInstance* pAudioEventInstance) override;

      void update3DAttributes(Audio::AudioEventInstance* pAudioEventInstance);

   public:
      ComponentAudioSource3D(Entity* pOwner);
      ~ComponentAudioSource3D();

      virtual void update() override;

      GEDefaultGetter(float, MinDistance)
      GEDefaultGetter(float, MaxDistance)

      void setMinDistance(float pValue);
      void setMaxDistance(float pValue);
   };
}}
