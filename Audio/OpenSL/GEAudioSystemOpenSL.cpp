
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio System (OpenSL)
//
//  --- GEAudioSystemOpenSL.cpp ---
//
//////////////////////////////////////////////////////////////////

#if 0

#include "GEAudioSystemOpenSL.h"
#include "Core/GEDevice.h"
#include <iostream>
#include <fstream>

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

using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;

AudioSystemOpenSL::AudioSystemOpenSL()
   : slSources(0)
   , slBuffers(0)
{
}

AudioSystemOpenSL::~AudioSystemOpenSL()
{
}

void AudioSystemOpenSL::internalInit()
{
   if(iChannels > 0)
      slSources = new OpenSLSource[iChannels];

   if(iSounds > 0)
      slBuffers = new AudioData[iSounds];

   // create engine and output mix
   slCreateEngine(&slEngineObject, 0, NULL, 0, NULL, NULL);
   (*slEngineObject)->Realize(slEngineObject, SL_BOOLEAN_FALSE);
   (*slEngineObject)->GetInterface(slEngineObject, SL_IID_ENGINE, (void*)&slEngine);

   const SLuint32 slEngineIdCount = 1;
   const SLInterfaceID slEngineIds[1] = { SL_IID_ENVIRONMENTALREVERB };
   const SLboolean slEngineReq[1] = { SL_BOOLEAN_FALSE };
   (*slEngine)->CreateOutputMix(slEngine, &slOutputMix, slEngineIdCount, slEngineIds, slEngineReq);
   (*slOutputMix)->Realize(slOutputMix, SL_BOOLEAN_FALSE);

   // define format
   SLDataLocator_AndroidSimpleBufferQueue slDataLocatorIn;
   slDataLocatorIn.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
   slDataLocatorIn.numBuffers = 1;

   SLDataFormat_PCM slDataFormat;
   slDataFormat.formatType = SL_DATAFORMAT_PCM;
   slDataFormat.numChannels = 1;
   slDataFormat.samplesPerSec = SL_SAMPLINGRATE_44_1;
   slDataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
   slDataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
   slDataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
   slDataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

   SLDataSource slDataSource;
   slDataSource.pLocator = &slDataLocatorIn;
   slDataSource.pFormat = &slDataFormat;

   slDataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
   slDataLocatorOut.outputMix = slOutputMix;

   slDataSink.pLocator = &slDataLocatorOut;
   slDataSink.pFormat = NULL;

   // create audio players (sources)
   const SLuint32 slAudioPlayerIdCount = 3;
   const SLInterfaceID slAudioPlayerIds[] = { SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
   const SLboolean slAudioPlayerReq[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

   for(unsigned int i = 0; i < iChannels; i++)
   {
      (*slEngine)->CreateAudioPlayer(slEngine, &slSources[i].AudioPlayer, &slDataSource, &slDataSink, slAudioPlayerIdCount, slAudioPlayerIds, slAudioPlayerReq);
      (*slSources[i].AudioPlayer)->Realize(slSources[i].AudioPlayer, SL_BOOLEAN_FALSE);

      (*slSources[i].AudioPlayer)->GetInterface(slSources[i].AudioPlayer, SL_IID_PLAY, &slSources[i].PlaybackState);
      (*slSources[i].AudioPlayer)->GetInterface(slSources[i].AudioPlayer, SL_IID_BUFFERQUEUE, &slSources[i].BufferQueue);
      (*slSources[i].AudioPlayer)->GetInterface(slSources[i].AudioPlayer, SL_IID_VOLUME, &slSources[i].VolumeController);

      (*slSources[i].BufferQueue)->RegisterCallback(slSources[i].BufferQueue, bufferCallback, this);

      (*slSources[i].VolumeController)->EnableStereoPosition(slSources[i].VolumeController, true);
      (*slSources[i].VolumeController)->SetVolumeLevel(slSources[i].VolumeController, 0.0f);
      (*slSources[i].VolumeController)->SetStereoPosition(slSources[i].VolumeController, 0.0f);
   }
}

void AudioSystemOpenSL::bufferCallback(SLBufferQueueItf slBufferQueue, void* pContext)
{
   AudioSystemOpenSL* cAudio = static_cast<AudioSystemOpenSL*>(pContext);

   for(unsigned int i = 0; i < cAudio->iChannels; i++)
   {
      if(slBufferQueue == cAudio->slSources[i].BufferQueue)
      {
         (*cAudio->slSources[i].PlaybackState)->SetPlayState(cAudio->slSources[i].PlaybackState, SL_PLAYSTATE_STOPPED);
         cAudio->sChannels[i].Free = true;
         return;
      }
   }
}

void AudioSystemOpenSL::release()
{
   if(slSources)
      delete[] slSources;

   if(slBuffers)
      delete[] slBuffers;

   (*slOutputMix)->Destroy(slOutputMix);
   (*slEngineObject)->Destroy(slEngineObject);
}

void AudioSystemOpenSL::loadSound(unsigned int SoundIndex, const char* FileName, const char* FileExtension)
{
   if(SoundIndex >= iSounds)
      return;
   
   Device::readContentFile(ContentType::Audio, "Sounds", FileName, FileExtension, &slBuffers[SoundIndex]);
}

void AudioSystemOpenSL::unloadSound(unsigned int SoundIndex)
{
   slBuffers[SoundIndex].unload();
}

void AudioSystemOpenSL::internalPlaySound(unsigned int SoundIndex, unsigned int SourceIndex)
{
   (*slSources[SourceIndex].BufferQueue)->Clear(slSources[SourceIndex].BufferQueue);
   (*slSources[SourceIndex].BufferQueue)->Enqueue(slSources[SourceIndex].BufferQueue, slBuffers[SoundIndex].getData(), slBuffers[SoundIndex].getDataSize());
   (*slSources[SourceIndex].PlaybackState)->SetPlayState(slSources[SourceIndex].PlaybackState, SL_PLAYSTATE_PLAYING);
}

void AudioSystemOpenSL::internalStop(unsigned int SourceIndex)
{
   (*slSources[SourceIndex].PlaybackState)->SetPlayState(slSources[SourceIndex].PlaybackState, SL_PLAYSTATE_STOPPED);
}

bool AudioSystemOpenSL::isPlaying(unsigned int SourceIndex)
{
   SLuint32 iPlaybackState = 0;
   (*slSources[SourceIndex].PlaybackState)->GetPlayState(slSources[SourceIndex].PlaybackState, &iPlaybackState);
   return iPlaybackState == SL_PLAYSTATE_PLAYING;
}

void AudioSystemOpenSL::moveListener(const Vector3& Delta)
{

}

void AudioSystemOpenSL::moveSource(unsigned int SourceIndex, const Vector3& Delta)
{

}

void AudioSystemOpenSL::setListenerPosition(const Vector3& Position)
{

}

void AudioSystemOpenSL::setVolume(unsigned int SourceIndex, float Volume)
{
   (*slSources[SourceIndex].VolumeController)->SetVolumeLevel(slSources[SourceIndex].VolumeController, linearToMillibel(Volume));
}

void AudioSystemOpenSL::setPosition(unsigned int SourceIndex, const Vector3& Position)
{
   //(*slSources[SourceIndex].VolumeController)->SetStereoPosition(slSources[SourceIndex].VolumeController, floatToPermille(Pan));
}

void AudioSystemOpenSL::setDirection(unsigned int SourceIndex, const Vector3& Direction)
{

}

SLmillibel AudioSystemOpenSL::linearToMillibel(float fGain)
{
   if(fGain >= 1.0f)
      return 0;

   if(fGain <= 0.0f)
      return SL_MILLIBEL_MIN;

   return (SLmillibel)(2000 * log10(fGain));
}

SLpermille AudioSystemOpenSL::floatToPermille(float fPanning)
{
   return (SLpermille)(1000 * fPanning);
}

#endif


#include "Audio/GEAudioSystem.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;

const char* AudioSystem::platformAudioFileExtension()
{
   return "";
}

void AudioSystem::platformInit()
{
}

void AudioSystem::platformUpdate()
{
}

void AudioSystem::platformRelease()
{
}

void AudioSystem::platformLoadSound(BufferID pBuffer, Content::AudioData* pAudioData)
{
}

void AudioSystem::platformUnloadSound(BufferID pBuffer)
{
}

void AudioSystem::platformReleaseChannel(ChannelID pChannel)
{
}

void AudioSystem::platformPlaySound(ChannelID pChannel, BufferID pBuffer, bool pLooping)
{
}

void AudioSystem::platformStop(ChannelID pChannel)
{
}

void AudioSystem::platformPause(ChannelID pChannel)
{
}

void AudioSystem::platformResume(ChannelID pChannel)
{
}

bool AudioSystem::platformIsPlaying(ChannelID pChannel) const
{
   return false;
}

bool AudioSystem::platformIsPaused(ChannelID pChannel) const
{
   return false;
}

bool AudioSystem::platformIsInUse(ChannelID pChannel) const
{
   return false;
}

void AudioSystem::platformSetVolume(ChannelID pChannel, float pVolume)
{
}

void AudioSystem::platformSetPitch(ChannelID pChannel, float pPitch)
{
}

void AudioSystem::platformSetPosition(ChannelID pChannel, const Vector3& pPosition)
{
}

void AudioSystem::platformSetOrientation(ChannelID pChannel, const Rotation& pOrientation)
{
}

void AudioSystem::platformSetMinDistance(ChannelID pChannel, float pDistance)
{
}

void AudioSystem::platformSetMaxDistance(ChannelID pChannel, float pDistance)
{
}

void AudioSystem::platformSetListenerPosition(const Vector3& pPosition)
{
}

void AudioSystem::platformSetListenerOrientation(const Rotation& pOrientation)
{
}
