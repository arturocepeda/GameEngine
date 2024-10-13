
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

#include "Audio/GEAudioSystem.h"
#include "Content/GEAudioData.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;


static const SLuint32 slAudioPlayerIdCount = 3u;
static const SLInterfaceID slAudioPlayerIds[] = { SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
static const SLboolean slAudioPlayerReq[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };


static SLObjectItf slEngineObject;
static SLEngineItf slEngine;
static SLObjectItf slOutputMix;

static SLDataLocator_OutputMix slDataLocatorOut;
static SLDataSink slDataSink;

struct OpenSLChannel
{
   SLObjectItf AudioPlayer;
   SLBufferQueueItf BufferQueue;
   SLPlayItf PlaybackState;
   SLVolumeItf VolumeController;

   BufferID Buffer;
   bool Looping;

   OpenSLChannel()
      : AudioPlayer(nullptr)
      , BufferQueue(nullptr)
      , PlaybackState(nullptr)
      , VolumeController(nullptr)
      , Buffer(0u)
      , Looping(false)
   {}
};

static OpenSLChannel* slChannels = nullptr;


static SLmillibel linearToMillibel(float pGain)
{
   if(pGain >= 1.0f)
   {
      return 0;
   }

   if(pGain <= 0.0f)
   {
      return SL_MILLIBEL_MIN;
   }

   return (SLmillibel)(2000.0f * log10(pGain));
}

static SLpermille floatToPermille(float pPanning)
{
   return (SLpermille)(1000.0f * pPanning);
}


const char* AudioSystem::platformAudioFileExtension()
{
   return "ogg";
}

void AudioSystem::platformInit()
{
   slChannels = Allocator::alloc<OpenSLChannel>(mChannelsCount, AllocationCategory::Audio);

   for(uint32_t i = 0u; i < mChannelsCount; i++)
   {
      GEInvokeCtor(OpenSLChannel, &slChannels[i]);
   }

   // create engine and output mix
   slCreateEngine(&slEngineObject, 0, nullptr, 0, nullptr, nullptr);
   (*slEngineObject)->Realize(slEngineObject, SL_BOOLEAN_FALSE);
   (*slEngineObject)->GetInterface(slEngineObject, SL_IID_ENGINE, (void*)&slEngine);

   const SLuint32 slEngineIdCount = 1;
   const SLInterfaceID slEngineIds[1] = { SL_IID_ENVIRONMENTALREVERB };
   const SLboolean slEngineReq[1] = { SL_BOOLEAN_FALSE };
   (*slEngine)->CreateOutputMix(slEngine, &slOutputMix, slEngineIdCount, slEngineIds, slEngineReq);
   (*slOutputMix)->Realize(slOutputMix, SL_BOOLEAN_FALSE);

   slDataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
   slDataLocatorOut.outputMix = slOutputMix;

   slDataSink.pLocator = &slDataLocatorOut;
   slDataSink.pFormat = nullptr;
}

void AudioSystem::platformUpdate()
{
}

void AudioSystem::platformRelease()
{
   if(slChannels)
   {
      Allocator::free(slChannels);
      slChannels = nullptr;
   }

   (*slOutputMix)->Destroy(slOutputMix);
   (*slEngineObject)->Destroy(slEngineObject);
}

void AudioSystem::platformLoadSound(BufferID pBuffer, Content::AudioData* pAudioData)
{
   (void)pBuffer;
   (void)pAudioData;
}

void AudioSystem::platformUnloadSound(BufferID pBuffer)
{
   (void)pBuffer;
}

void AudioSystem::platformReleaseChannel(ChannelID pChannel)
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return;
   }

   (*slChannels[pChannel].AudioPlayer)->Destroy(slChannels[pChannel].AudioPlayer);
   GEInvokeCtor(OpenSLChannel, &slChannels[pChannel]);
}

void AudioSystem::platformPlaySound(ChannelID pChannel, BufferID pBuffer, bool pLooping)
{
   // Define format
   SLDataLocator_AndroidSimpleBufferQueue slDataLocatorIn;
   slDataLocatorIn.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
   slDataLocatorIn.numBuffers = 1;

   SLDataFormat_PCM slDataFormat;
   slDataFormat.formatType = SL_DATAFORMAT_PCM;
   slDataFormat.numChannels = (SLuint32)mBuffers[pBuffer].Data->getNumberOfChannels();
   slDataFormat.samplesPerSec = (SLuint32)(mBuffers[pBuffer].Data->getSampleRate() * 1000u);
   slDataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
   slDataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
   slDataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;
   slDataFormat.channelMask = slDataFormat.numChannels == 1u
      ? SL_SPEAKER_FRONT_CENTER
      : SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;

   SLDataSource slDataSource;
   slDataSource.pLocator = &slDataLocatorIn;
   slDataSource.pFormat = &slDataFormat;

   // Create audio player, and play the sound
   (*slEngine)->CreateAudioPlayer(slEngine, &slChannels[pChannel].AudioPlayer, &slDataSource, &slDataSink, slAudioPlayerIdCount, slAudioPlayerIds, slAudioPlayerReq);
   (*slChannels[pChannel].AudioPlayer)->Realize(slChannels[pChannel].AudioPlayer, SL_BOOLEAN_FALSE);

   static const auto bufferCallback = [](SLBufferQueueItf pSLBufferQueue, void* pContext)
   {
      AudioSystem* audioSystem = (AudioSystem*)pContext;

      for(uint32_t i = 0u; i < audioSystem->mChannelsCount; i++)
      {
         if(pSLBufferQueue == slChannels[i].BufferQueue)
         {
            if(slChannels[i].Looping)
            {
               const BufferID buffer = slChannels[i].Buffer;
               const char* bufferData = audioSystem->mBuffers[buffer].Data->getData();
               const uint32_t bufferSize = audioSystem->mBuffers[buffer].Data->getDataSize();

               (*slChannels[i].BufferQueue)->Enqueue(slChannels[i].BufferQueue, bufferData, bufferSize);
            }
            else
            {
               audioSystem->platformStop((ChannelID)i);
            }

            break;
         }
      }
   };

   (*slChannels[pChannel].AudioPlayer)->GetInterface(slChannels[pChannel].AudioPlayer, SL_IID_BUFFERQUEUE, &slChannels[pChannel].BufferQueue);
   (*slChannels[pChannel].BufferQueue)->RegisterCallback(slChannels[pChannel].BufferQueue, bufferCallback, this);
   (*slChannels[pChannel].BufferQueue)->Clear(slChannels[pChannel].BufferQueue);
   (*slChannels[pChannel].BufferQueue)->Enqueue(slChannels[pChannel].BufferQueue, mBuffers[pBuffer].Data->getData(),mBuffers[pBuffer].Data->getDataSize());

   (*slChannels[pChannel].AudioPlayer)->GetInterface(slChannels[pChannel].AudioPlayer, SL_IID_VOLUME, &slChannels[pChannel].VolumeController);
   (*slChannels[pChannel].VolumeController)->EnableStereoPosition(slChannels[pChannel].VolumeController, slDataFormat.numChannels == 1u);

   (*slChannels[pChannel].AudioPlayer)->GetInterface(slChannels[pChannel].AudioPlayer, SL_IID_PLAY, &slChannels[pChannel].PlaybackState);
   (*slChannels[pChannel].PlaybackState)->SetPlayState(slChannels[pChannel].PlaybackState, SL_PLAYSTATE_PLAYING);

   slChannels[pChannel].Buffer = pBuffer;
   slChannels[pChannel].Looping = pLooping;
}

void AudioSystem::platformStop(ChannelID pChannel)
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return;
   }

   (*slChannels[pChannel].PlaybackState)->SetPlayState(slChannels[pChannel].PlaybackState, SL_PLAYSTATE_STOPPED);
}

void AudioSystem::platformPause(ChannelID pChannel)
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return;
   }

   (*slChannels[pChannel].PlaybackState)->SetPlayState(slChannels[pChannel].PlaybackState, SL_PLAYSTATE_PAUSED);
}

void AudioSystem::platformResume(ChannelID pChannel)
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return;
   }

   (*slChannels[pChannel].PlaybackState)->SetPlayState(slChannels[pChannel].PlaybackState, SL_PLAYSTATE_PLAYING);
}

bool AudioSystem::platformIsPlaying(ChannelID pChannel) const
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return false;
   }

   SLuint32 playbackState = 0u;
   (*slChannels[pChannel].PlaybackState)->GetPlayState(slChannels[pChannel].PlaybackState, &playbackState);
   return playbackState == SL_PLAYSTATE_PLAYING;
}

bool AudioSystem::platformIsPaused(ChannelID pChannel) const
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return false;
   }

   SLuint32 playbackState = 0u;
   (*slChannels[pChannel].PlaybackState)->GetPlayState(slChannels[pChannel].PlaybackState, &playbackState);
   return playbackState == SL_PLAYSTATE_PAUSED;
}

bool AudioSystem::platformIsInUse(ChannelID pChannel) const
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return false;
   }

   SLuint32 playbackState = 0u;
   (*slChannels[pChannel].PlaybackState)->GetPlayState(slChannels[pChannel].PlaybackState, &playbackState);
   return playbackState != SL_PLAYSTATE_STOPPED;
}

void AudioSystem::platformSetVolume(ChannelID pChannel, float pVolume)
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return;
   }

   (*slChannels[pChannel].VolumeController)->SetVolumeLevel(slChannels[pChannel].VolumeController, linearToMillibel(pVolume));
}

void AudioSystem::platformSetPitch(ChannelID pChannel, float pPitch)
{
}

void AudioSystem::platformSetPosition(ChannelID pChannel, const Vector3& pPosition)
{
   if(!slChannels[pChannel].AudioPlayer)
   {
      return;
   }

   //(*slChannels[pChannel].VolumeController)->SetStereoPosition(slChannels[pChannel].VolumeController, floatToPermille(pan));
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
