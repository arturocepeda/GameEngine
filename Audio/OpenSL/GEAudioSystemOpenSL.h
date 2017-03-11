
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio System (OpenSL)
//
//  --- GEAudioSystemOpenSL.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Audio/GEAudioSystem.h"
#include "Content/GEAudioData.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace GE { namespace Audio
{
   class AudioSystemOpenSL : public AudioSystem
   {
   private:
      SLObjectItf slEngineObject;
      SLEngineItf slEngine;
      SLObjectItf slOutputMix;

      SLDataLocator_OutputMix slDataLocatorOut;
      SLDataSink slDataSink;

      struct OpenSLSource
      {
         SLObjectItf AudioPlayer;
         SLBufferQueueItf BufferQueue;
         SLPlayItf PlaybackState;
         SLVolumeItf VolumeController;
      };

      OpenSLSource* slSources;
      Content::AudioData* slBuffers;

      void internalInit();

      SLmillibel linearToMillibel(float fGain);
      SLpermille floatToPermille(float fPanning);

      static void bufferCallback(SLBufferQueueItf slBufferQueue, void* pContext);

   public:
      AudioSystemOpenSL();
      ~AudioSystemOpenSL();

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
