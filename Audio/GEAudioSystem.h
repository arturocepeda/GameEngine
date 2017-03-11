
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

#include "Types/GETypes.h"
#include "Core/GESingleton.h"

#define GE_AUDIO_CHANNELS   24
#define GE_AUDIO_SOUNDS    256

#define GE_AUDIO_UPDATE_FRAMES  30

namespace GE { namespace Audio
{
   struct AudioChannel
   {
      uint AssignmentTime;
      uint AssignedSound;
      bool Free;

      AudioChannel()
         : AssignmentTime(0)
         , AssignedSound(0)
         , Free(true) {}
   };

   class AudioSystem : public Core::Singleton<AudioSystem>
   {
   protected:
       void* pHandler;

       uint iChannels;
       uint iSounds;
      
       AudioChannel* sChannels;
       uint iAssignmentTime;

       virtual void internalInit() = 0;
       virtual void internalPlaySound(uint Sound, uint Channel) = 0;
       virtual void internalStop(uint Channel) = 0;

   public:
       AudioSystem();
       virtual ~AudioSystem();

       void init(uint Channels = GE_AUDIO_CHANNELS, uint Sounds = GE_AUDIO_SOUNDS);
       virtual void update();
       virtual void release() = 0;

       void* getHandler();

       virtual void loadSound(uint Sound, const char* FileName, const char* FileExtension) = 0;
       virtual void unloadSound(uint Sound) = 0;
       void unloadAllSounds();

       uint playSound(uint Sound);
       void playSound(uint Sound, uint Channel);
       void stop(uint Channel);

       virtual bool isPlaying(uint Channel) = 0;

       virtual void setListenerPosition(const Vector3& Position) = 0;
       virtual void setVolume(uint Channel, float Volume) = 0;
       virtual void setPosition(uint Channel, const Vector3& Position) = 0;
   };
}}
