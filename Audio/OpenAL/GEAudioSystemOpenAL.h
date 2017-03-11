
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio System (OpenAL)
//
//  --- GEAudioSystemOpenAL.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Audio/GEAudioSystem.h"
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/ExtendedAudioFile.h>

namespace GE { namespace Audio
{
   struct SOpenALSource
   {
      ALuint Source;
      unsigned int EntityID;
      unsigned int Assignment;
      bool Free;
      
      SOpenALSource()
      {
         Source = 0;
         EntityID = 0;
         Assignment = 0;
         Free = true;
      }
   };

   class AudioSystemOpenAL : public AudioSystem
   {
   private:
      ALCdevice* alDevice;
      ALCcontext* alContext;
      
      ALuint alSourcesAlloc[GE_AUDIO_CHANNELS];
      SOpenALSource alSources[GE_AUDIO_CHANNELS];
      ALuint alBuffers[GE_AUDIO_SOUNDS];
      
      void internalInit();
      void internalPlaySound(unsigned int SoundIndex, unsigned int SourceIndex);
      void internalStop(unsigned int SourceIndex);
      
      bool loadAudioFile(CFURLRef cfURL, ALenum* alFormat, ALvoid** alData, ALsizei* alSize, 
                         ALsizei* alFreq);

   public:
      void update();
      void release();
      
      void loadSound(unsigned int SoundIndex, const char* FileName, const char* FileExtension);
      void unloadSound(unsigned int SoundIndex);

      bool isPlaying(unsigned int SourceIndex);
      
      void moveListener(const Vector3& Delta);
      void moveSource(unsigned int SourceIndex, const Vector3& Delta);
      
      void setListenerPosition(const Vector3& Position);
      void setVolume(unsigned int SourceIndex, float Volume);
      void setPosition(unsigned int SourceIndex, const Vector3& Position);
      void setDirection(unsigned int SourceIndex, const Vector3& Direction);
   };   
}}
