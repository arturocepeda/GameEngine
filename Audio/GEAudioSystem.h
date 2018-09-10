
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Audio
//
//  --- GEAudioSystem.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEAudioBank.h"

#include "Types/GETypes.h"
#include "Core/GESingleton.h"
#include "Core/GEObjectManager.h"

#define GE_AUDIO_CHANNELS    24
#define GE_AUDIO_BUFFERS    256

namespace GE { namespace Content
{
   class AudioData;
}};

namespace GE { namespace Audio
{
   typedef uint32_t BufferID;
   typedef uint32_t ChannelID;

   struct AudioChannel
   {
      uint32_t AssignmentIndex;
      BufferID AssignedBuffer;
      bool Free;

      AudioChannel()
         : AssignmentIndex(0)
         , AssignedBuffer(0)
         , Free(true) {}
   };

   struct AudioBuffer
   {
      uint32_t AssignedFileID;
      uint32_t References;

      AudioBuffer()
         : AssignedFileID(0)
         , References(0) {}
   };

   struct AudioEventInstance
   {
      AudioEvent* Event;
      ChannelID Channel;
      bool Active;

      AudioEventInstance()
         : Event(0)
         , Channel(0)
         , Active(false) {}
   };

   class AudioSystem : public Core::Singleton<AudioSystem>
   {
   protected:
      static const uint32_t AudioEventInstancesCount = 256;

      void* mHandler;

      uint32_t mChannelsCount;
      uint32_t mBuffersCount;

      Core::ObjectManager<AudioBank> mAudioBanks;
      Core::ObjectManager<AudioEvent> mAudioEvents;
      
      AudioChannel* mChannels;
      uint32_t mChannelAssignmentIndex;

      AudioBuffer* mBuffers;

      AudioEventInstance mAudioEventInstances[AudioEventInstancesCount];
      uint32_t mAudioEventInstanceAssignmentIndex;

      GESTLVector(AudioEventInstance*) mActiveAudioEventInstances;

      GEMutex mMutex;

      float mTimeSinceLastUpdate;

      Vector3 mListenerPosition;
      Rotation mListenerOrientation;

      void loadAudioEventEntries();
      void loadAudioBankEntries();

      void releaseChannel(ChannelID pChannel);
      void releaseAudioBankFiles(AudioBank* pAudioBank);

      // platform specific methods
      void platformInit();
      void platformUpdate();
      void platformRelease();

      void platformLoadSound(BufferID pBuffer, Content::AudioData* pAudioData);
      void platformUnloadSound(BufferID pBuffer);

      void platformPlaySound(ChannelID pChannel, BufferID pBuffer);
      void platformStop(ChannelID pChannel);
      void platformPause(ChannelID pChannel);
      void platformResume(ChannelID pChannel);
      bool platformIsPlaying(ChannelID pChannel) const;
      bool platformIsPaused(ChannelID pChannel) const;
      bool platformIsInUse(ChannelID pChannel) const;

      void platformSetVolume(ChannelID pChannel, float pVolume);

      void platformSetPosition(ChannelID pChannel, const Vector3& pPosition);
      void platformSetOrientation(ChannelID pChannel, const Rotation& pOrientation);
      void platformSetMinDistance(ChannelID pChannel, float pDistance);
      void platformSetMaxDistance(ChannelID pChannel, float pDistance);

      void platformSetListenerPosition(const Vector3& pPosition);
      void platformSetListenerOrientation(const Rotation& pOrientation);

   public:
      AudioSystem();
      ~AudioSystem();

      void* getHandler() { return mHandler; }

      void init(uint32_t pChannelsCount = GE_AUDIO_CHANNELS, uint32_t pBuffersCount = GE_AUDIO_BUFFERS);
      void update();
      void release();

      void loadAudioBank(const Core::ObjectName& pAudioBankName);
      void unloadAudioBank(const Core::ObjectName& pAudioBankName);
      void unloadAllAudioBanks();

      AudioEventInstance* playAudioEvent(const Core::ObjectName& pAudioBankName, const Core::ObjectName& pAudioEventName);
      void stop(AudioEventInstance* pAudioEventInstance);
      void pause(AudioEventInstance* pAudioEventInstance);
      void resume(AudioEventInstance* pAudioEventInstance);
      bool isPlaying(AudioEventInstance* pAudioEventInstance) const;
      bool isPaused(AudioEventInstance* pAudioEventInstance) const;

      void setVolume(AudioEventInstance* pAudioEventInstance, float pVolume);

      void setPosition(AudioEventInstance* pAudioEventInstance, const Vector3& pPosition);
      void setOrientation(AudioEventInstance* pAudioEventInstance, const Rotation& pOrientation);
      void setMinDistance(AudioEventInstance* pAudioEventInstance, float pDistance);
      void setMaxDistance(AudioEventInstance* pAudioEventInstance, float pDistance);

      void setListenerPosition(const Vector3& pPosition);
      void setListenerOrientation(const Rotation& pOrientation);

      const Vector3& getListenerPosition() const { return mListenerPosition; }
      const Rotation& getListenerOrientation() const { return mListenerOrientation; }
   };
}}
