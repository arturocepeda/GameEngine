
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio (FMOD)
//
//  --- GEAudioSystemFMOD.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Audio/GEAudioSystem.h"
#include "Externals/FMOD/include/fmod.hpp"
#include "Externals/FMOD/include/fmod_errors.h"

namespace GE { namespace Audio
{
   class AudioSystemFMOD : public AudioSystem
   {
   private:
      FMOD::System* fmodSystem;
      FMOD::Channel* fmodChannels[GE_AUDIO_CHANNELS];
      FMOD::Sound* fmodSounds[GE_AUDIO_SOUNDS];

      float fVolume[GE_AUDIO_CHANNELS];
      Vector3 vPosition[GE_AUDIO_CHANNELS];

      void ERRCHECK(FMOD_RESULT result);

   public:
      AudioSystemFMOD();
      ~AudioSystemFMOD();

      void internalInit() override;
      void update() override;
      void release() override;

      void loadSound(uint Sound, const char* FileName, const char* FileExtension) override;
      void unloadSound(uint Sound) override;

      void internalPlaySound(uint Sound, uint Channel) override;
      void internalStop(uint Channel) override;

      bool isPlaying(uint Channel) override;

      void setListenerPosition(const Vector3& Position) override;
      void setVolume(uint Channel, float Volume) override;
      void setPosition(uint Channel, const Vector3& Position) override;
   };
}}
