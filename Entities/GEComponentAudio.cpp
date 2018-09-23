
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentAudio.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentAudio.h"
#include "GEComponentTransform.h"
#include "GEEntity.h"

#include "Audio/GEAudioSystem.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Audio;


//
//  ComponentAudio
//
ComponentAudio::ComponentAudio(Entity* pOwner)
   : Component(pOwner)
{
}


//
//  ComponentAudioListener
//
ComponentAudioListener::ComponentAudioListener(Entity* pOwner)
   : ComponentAudio(pOwner)
   , mActive(true)
{
   cClassName = ObjectName("AudioListener");

   GERegisterProperty(Bool, Active);
}

ComponentAudioListener::~ComponentAudioListener()
{
}

void ComponentAudioListener::update()
{
   if(!mActive)
      return;

   ComponentTransform* transform = cOwner->getComponent<ComponentTransform>();
   AudioSystem* audioSystem = AudioSystem::getInstance();

   audioSystem->setListenerPosition(transform->getWorldPosition());
   audioSystem->setListenerOrientation(transform->getRotation().getQuaternion());
}


//
//  ComponentAudioSource
//
const ObjectName PlayAudioEventName = ObjectName("PlayAudioEvent");
const ObjectName PauseAllAudioEventsName = ObjectName("PauseAllAudioEvents");
const ObjectName ResumeAllAudioEventsName = ObjectName("ResumeAllAudioEvents");
const ObjectName StopAllAudioEventsName = ObjectName("StopAllAudioEvents");

ComponentAudioSource::ComponentAudioSource(Entity* pOwner)
   : ComponentAudio(pOwner)
{
   cClassName = ObjectName("AudioSource");

   GERegisterProperty(ObjectName, AudioBankName);
   GERegisterProperty(ObjectName, AudioEventName);

   registerAction(PlayAudioEventName, [this]
   {
      playAudioEvent(mAudioEventName);
   });
   registerAction(PauseAllAudioEventsName, [this]
   {
      pauseAllAudioEvents();
   });
   registerAction(ResumeAllAudioEventsName, [this]
   {
      resumeAllAudioEvents();
   });
   registerAction(StopAllAudioEventsName, [this]
   {
      stopAllAudioEvents();
   });
}

ComponentAudioSource::~ComponentAudioSource()
{
   stopAllAudioEvents();
}

AudioEventInstance* ComponentAudioSource::playAudioEvent(const Core::ObjectName& pAudioEventName)
{
   AudioSystem* audioSystem = AudioSystem::getInstance();
   AudioEventInstance* audioEventInstance = audioSystem->playAudioEvent(mAudioBankName, pAudioEventName);

   if(!audioEventInstance)
      return 0;

   mAudioEventInstances.push_back(audioEventInstance);

   onAudioEventPlayed(audioEventInstance);

   return audioEventInstance;
}

void ComponentAudioSource::pauseAllAudioEvents()
{
   AudioSystem* audioSystem = AudioSystem::getInstance();

   for(size_t i = 0; i < mAudioEventInstances.size(); i++)
   {
      audioSystem->pause(mAudioEventInstances[i]);
   }
}

void ComponentAudioSource::resumeAllAudioEvents()
{
   AudioSystem* audioSystem = AudioSystem::getInstance();

   for(size_t i = 0; i < mAudioEventInstances.size(); i++)
   {
      audioSystem->resume(mAudioEventInstances[i]);
   }
}

void ComponentAudioSource::stopAllAudioEvents()
{
   AudioSystem* audioSystem = AudioSystem::getInstance();

   for(size_t i = 0; i < mAudioEventInstances.size(); i++)
   {
      audioSystem->stop(mAudioEventInstances[i]);
   }
}

void ComponentAudioSource::update()
{
   for(size_t i = 0; i < mAudioEventInstances.size(); )
   {
      if(mAudioEventInstances[i]->State == AudioEventInstanceState::Free)
      {
         mAudioEventInstances[i] = mAudioEventInstances[mAudioEventInstances.size() - 1];
         mAudioEventInstances.pop_back();
      }
      else
      {
         i++;
      }
   }
}


//
//  ComponentAudioSource2D
//
ComponentAudioSource2D::ComponentAudioSource2D(Entity* pOwner)
   : ComponentAudioSource(pOwner)
   , mVolume(1.0f)
{
   cClassName = ObjectName("AudioSource2D");

   GERegisterProperty(Float, Volume);
}

ComponentAudioSource2D::~ComponentAudioSource2D()
{
}

void ComponentAudioSource2D::onAudioEventPlayed(AudioEventInstance* pAudioEventInstance)
{
   AudioSystem* audioSystem = AudioSystem::getInstance();
   audioSystem->setVolume(pAudioEventInstance, mVolume);
}

void ComponentAudioSource2D::setVolume(float pValue)
{
   if(fabsf(mVolume - pValue) > GE_EPSILON)
   {
      mVolume = pValue;

      AudioSystem* audioSystem = AudioSystem::getInstance();

      for(size_t i = 0; i < mAudioEventInstances.size(); i++)
      {
         audioSystem->setVolume(mAudioEventInstances[i], mVolume);
      }
   }
}


//
//  ComponentAudioSource3D
//
ComponentAudioSource3D::ComponentAudioSource3D(Entity* pOwner)
   : ComponentAudioSource(pOwner)
   , mMinDistance(2.0f)
   , mMaxDistance(20.0f)
{
   cClassName = ObjectName("AudioSource3D");

   GERegisterProperty(Float, MinDistance);
   GERegisterProperty(Float, MaxDistance);
}

ComponentAudioSource3D::~ComponentAudioSource3D()
{
}

void ComponentAudioSource3D::onAudioEventPlayed(AudioEventInstance* pAudioEventInstance)
{
   update3DAttributes(pAudioEventInstance);

   AudioSystem* audioSystem = AudioSystem::getInstance();
   audioSystem->setMinDistance(pAudioEventInstance, mMinDistance);
   audioSystem->setMaxDistance(pAudioEventInstance, mMaxDistance);
}

void ComponentAudioSource3D::update3DAttributes(AudioEventInstance* pAudioEventInstance)
{
   ComponentTransform* transform = cOwner->getComponent<ComponentTransform>();
   const Vector3& worldPosition = transform->getWorldPosition();
   Rotation worldRotation = transform->getWorldRotation();

   AudioSystem* audioSystem = AudioSystem::getInstance();
   audioSystem->setPosition(pAudioEventInstance, worldPosition);
   audioSystem->setOrientation(pAudioEventInstance, worldRotation);
}

void ComponentAudioSource3D::update()
{
   if(mAudioEventInstances.empty())
      return;

   ComponentAudioSource::update();

   AudioSystem* audioSystem = AudioSystem::getInstance();

   for(size_t i = 0; i < mAudioEventInstances.size(); i++)
   {
      update3DAttributes(mAudioEventInstances[i]);
   }
}

void ComponentAudioSource3D::setMinDistance(float pValue)
{
   if(fabsf(mMinDistance - pValue) > GE_EPSILON)
   {
      mMinDistance = pValue;

      AudioSystem* audioSystem = AudioSystem::getInstance();

      for(size_t i = 0; i < mAudioEventInstances.size(); i++)
      {
         audioSystem->setMinDistance(mAudioEventInstances[i], mMinDistance);
      }
   }
}

void ComponentAudioSource3D::setMaxDistance(float pValue)
{
   if(fabsf(mMaxDistance - pValue) > GE_EPSILON)
   {
      mMaxDistance = pValue;

      AudioSystem* audioSystem = AudioSystem::getInstance();

      for(size_t i = 0; i < mAudioEventInstances.size(); i++)
      {
         audioSystem->setMaxDistance(mAudioEventInstances[i], mMaxDistance);
      }
   }
}
