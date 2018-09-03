
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
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
      uint32_t AssignmentTime;
      BufferID AssignedBuffer;
      bool Free;

      AudioChannel()
         : AssignmentTime(0)
         , AssignedBuffer(0)
         , Free(true) {}
   };

   class AudioSystem : public Core::Singleton<AudioSystem>
   {
   protected:
       void* mHandler;

       uint32_t mChannelsCount;
       uint32_t mBuffersCount;

       Core::ObjectManager<AudioBank> mAudioBanks;
       Core::ObjectManager<AudioEvent> mAudioEvents;

       GESTLMap(uint32_t, BufferID) mAudioBuffers;
       BufferID mNextBufferToAssign;
      
       AudioChannel* mChannels;
       uint32_t mAssignmentTime;
       float mTimeSinceLastUpdate;

       Vector3 mListenerPosition;
       Quaternion mListenerOrientation;

       void loadAudioEventEntries();
       void loadAudioBankEntries();

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

       void platformSetListenerPosition(const Vector3& pPosition);
       void platformSetListenerOrientation(const Quaternion& pOrientation);

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

       ChannelID playAudioEvent(const Core::ObjectName& pAudioBankName, const Core::ObjectName& pAudioEventName);
       void playAudioEvent(const Core::ObjectName& pAudioBankName, const Core::ObjectName& pAudioEventName, ChannelID pChannel);
       void stop(ChannelID pChannel);
       void pause(ChannelID pChannel);
       void resume(ChannelID pChannel);
       bool isPlaying(ChannelID pChannel) const;
       bool isPaused(ChannelID pChannel) const;
       void setVolume(ChannelID pChannel, float pVolume);
       void setPosition(ChannelID pChannel, const Vector3& pPosition);

       void setListenerPosition(const Vector3& pPosition);
       void setListenerOrientation(const Quaternion& pOrientation);

       const Vector3& getListenerPosition() const { return mListenerPosition; }
       const Quaternion& getListenerOrientation() const { return mListenerOrientation; }
   };
}}
