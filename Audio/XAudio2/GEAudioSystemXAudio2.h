
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio (XAudio2)
//
//  --- GEAudioSystemXAudio2.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Audio/GEAudioSystem.h"
#include <xaudio2.h>

namespace GE { namespace Audio
{
   class SourceVoiceCallback;


   struct SourceVoice
   {
      IXAudio2SourceVoice* Handle;
      SourceVoiceCallback* Callback;
      bool Playing;

      SourceVoice() : Handle(0), Callback(0), Playing(false) {}
   };


   class SourceVoiceCallback : public IXAudio2VoiceCallback
   {
   private:
      SourceVoice& sSourceVoiceRef;

   public:
      SourceVoiceCallback(SourceVoice& SourceVoiceRef);
      ~SourceVoiceCallback();

      void OnStreamEnd();
      void OnVoiceProcessingPassEnd();
      void OnVoiceProcessingPassStart(UINT32 SamplesRequired);
      void OnBufferEnd(void* pBufferContext);
      void OnBufferStart(void* pBufferContext);
      void OnLoopEnd(void* pBufferContext);
      void OnVoiceError(void* pBufferContext, HRESULT Error);
   };


   class AudioSystemXAudio2 : public AudioSystem
   {
   protected:
      IXAudio2* xaEngine;
      IXAudio2MasteringVoice* xaMasteringVoice;

      SourceVoice xaSourceVoices[GE_AUDIO_CHANNELS];
      XAUDIO2_BUFFER xaBuffers[GE_AUDIO_SOUNDS];

      void internalInit();

   public:
      AudioSystemXAudio2();
      ~AudioSystemXAudio2();

      void release();
   
      void loadSound(unsigned int SoundIndex, const char* FileName, const char* FileExtension);
      void unloadSound(unsigned int SoundIndex);
      void unloadAllSounds();
   
      void internalPlaySound(unsigned int SoundIndex, unsigned int SourceIndex);
      void internalStop(unsigned int SourceIndex);

      bool isPlaying(unsigned int SourceIndex);
   
      void moveListener(const Vector3& Delta);
      void moveSource(unsigned int SourceIndex, const Vector3& Delta);
   
      void setListenerPosition(const Vector3& Position);
      void setVolume(unsigned int SourceIndex, float Volume);
      void setPosition(unsigned int SourceIndex, const Vector3& Position);
      void setDirection(unsigned int SourceIndex, const Vector3& Direction);
   };
}}
