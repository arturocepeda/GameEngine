
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio System (AAudio)
//
//  --- GEAudioSystemAAudio.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Audio/GEAudioSystem.h"
#include "Content/GEAudioData.h"

#include <atomic>

#include <aaudio/AAudio.h>

using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;


struct AAudioChannel
{
   AAudioStreamBuilder* StreamBuilder;
   AAudioStream* Stream;
   int32_t FrameCursor;
   BufferID Buffer;
   std::atomic<float> Volume;
   std::atomic<float> Pitch;
   bool Looping;

   AAudioChannel()
      : StreamBuilder(nullptr)
      , Stream(nullptr)
      , FrameCursor(0)
      , Buffer(0u)
      , Volume(-1.0f)
      , Pitch(-1.0f)
      , Looping(false)
   {}
};

static AAudioChannel* gAAudioChannels = nullptr;


const char* AudioSystem::platformAudioFileExtension()
{
   return "ogg";
}

void AudioSystem::platformInit()
{
   gAAudioChannels = Allocator::alloc<AAudioChannel>(mChannelsCount, AllocationCategory::Audio);

   for(uint32_t i = 0u; i < mChannelsCount; i++)
   {
      GEInvokeCtor(AAudioChannel, &gAAudioChannels[i]);
   }
}

void AudioSystem::platformUpdate()
{
}

void AudioSystem::platformRelease()
{
   if(gAAudioChannels)
   {
      Allocator::free(gAAudioChannels);
      gAAudioChannels = nullptr;
   }
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
   if(!gAAudioChannels[pChannel].Stream)
   {
      return;
   }

   AAudioStream_close(gAAudioChannels[pChannel].Stream);
   AAudioStreamBuilder_delete(gAAudioChannels[pChannel].StreamBuilder);

   GEInvokeCtor(AAudioChannel, &gAAudioChannels[pChannel]);
}

void AudioSystem::platformPlaySound(ChannelID pChannel, BufferID pBuffer, bool pLooping)
{
   AAudioChannel& aaudioChannel = gAAudioChannels[pChannel];
   aaudioChannel.Buffer = pBuffer;
   aaudioChannel.Looping = pLooping;

   AAudio_createStreamBuilder(&aaudioChannel.StreamBuilder);
   GEAssert(aaudioChannel.StreamBuilder);

   const AudioBuffer& audioBuffer = mBuffers[pBuffer];
   const int32_t audioSampleRate = (int32_t)audioBuffer.Data->getSampleRate();
   const int32_t audioChannelCount = (int32_t)audioBuffer.Data->getNumberOfChannels();

   AAudioStreamBuilder_setSampleRate(aaudioChannel.StreamBuilder, audioSampleRate);
   AAudioStreamBuilder_setChannelCount(aaudioChannel.StreamBuilder, audioChannelCount);
   AAudioStreamBuilder_setFormat(aaudioChannel.StreamBuilder, AAUDIO_FORMAT_PCM_I16);
   AAudioStreamBuilder_setDirection(aaudioChannel.StreamBuilder, AAUDIO_DIRECTION_OUTPUT);

   AAudioStreamBuilder_setDataCallback
   (
      aaudioChannel.StreamBuilder,
      [](AAudioStream* pStream, void* pUserData, void* pAudioData, int32_t pNumFrames) -> aaudio_data_callback_result_t
      {
         AudioSystem* audioSystem = (AudioSystem*)pUserData;
         AAudioChannel* aaudioChannel = nullptr;

         for(uint32_t i = 0u; i < audioSystem->mChannelsCount; i++)
         {
            if(pStream == gAAudioChannels[i].Stream)
            {
               aaudioChannel = &gAAudioChannels[i];
               break;
            }
         }

         GEAssert(aaudioChannel);

         if(aaudioChannel->Volume < 0.0f || aaudioChannel->Pitch < 0.0f)
         {
            // Still uninitialized
            return AAUDIO_CALLBACK_RESULT_CONTINUE;
         }

         const AudioBuffer& audioBuffer = audioSystem->mBuffers[aaudioChannel->Buffer];
         const int32_t audioChannelCount = (int32_t)audioBuffer.Data->getNumberOfChannels();

         static const int32_t kBitDepth = 2u; // 16-bit
         const int32_t frameSize = kBitDepth * audioChannelCount;
         const int32_t framesCount = (int32_t)audioBuffer.Data->getDataSize() / frameSize;

         if(aaudioChannel->FrameCursor == framesCount)
         {
            if(aaudioChannel->Looping)
            {
               aaudioChannel->FrameCursor = 0;
            }
            else
            {
               return AAUDIO_CALLBACK_RESULT_STOP;
            }
         }

         const int32_t remainingFrames = framesCount - aaudioChannel->FrameCursor;
         const int32_t framesToCopy = pNumFrames <= remainingFrames
                 ? pNumFrames
                 : remainingFrames;

         const int32_t dataOffset = aaudioChannel->FrameCursor * frameSize;
         const int32_t framesToCopySize = framesToCopy * frameSize;

         memcpy(pAudioData, audioBuffer.Data->getData() + dataOffset, framesToCopySize);
         aaudioChannel->FrameCursor += framesToCopy;

         if(!GEFloatEquals(aaudioChannel->Volume, 1.0f))
         {
            const int32_t samplesCount = framesToCopy * audioChannelCount;
            int16_t* audioDataCursor = (int16_t*)pAudioData;

            for(int32_t i = 0; i < samplesCount; i++, audioDataCursor++)
            {
               *audioDataCursor = (int16_t)((float)(*audioDataCursor) * aaudioChannel->Volume);
            }
         }

         return AAUDIO_CALLBACK_RESULT_CONTINUE;
      },
      this
   );

   AAudioStreamBuilder_openStream(aaudioChannel.StreamBuilder, &aaudioChannel.Stream);
   GEAssert(aaudioChannel.Stream);

   AAudioStream_requestStart(aaudioChannel.Stream);
}

void AudioSystem::platformStop(ChannelID pChannel)
{
   if(!gAAudioChannels[pChannel].Stream)
   {
      return;
   }

   AAudioStream_requestStop(gAAudioChannels[pChannel].Stream);
}

void AudioSystem::platformPause(ChannelID pChannel)
{
   if(!gAAudioChannels[pChannel].Stream)
   {
      return;
   }

   AAudioStream_requestPause(gAAudioChannels[pChannel].Stream);
}

void AudioSystem::platformResume(ChannelID pChannel)
{
   if(!gAAudioChannels[pChannel].Stream)
   {
      return;
   }

   AAudioStream_requestStart(gAAudioChannels[pChannel].Stream);
}

bool AudioSystem::platformIsPlaying(ChannelID pChannel) const
{
   if(!gAAudioChannels[pChannel].Stream)
   {
      return false;
   }

   const aaudio_stream_state_t state = AAudioStream_getState(gAAudioChannels[pChannel].Stream);
   return state == AAUDIO_STREAM_STATE_STARTED || state == AAUDIO_STREAM_STATE_STARTING;
}

bool AudioSystem::platformIsPaused(ChannelID pChannel) const
{
   if(!gAAudioChannels[pChannel].Stream)
   {
      return false;
   }

   const aaudio_stream_state_t state = AAudioStream_getState(gAAudioChannels[pChannel].Stream);
   return state == AAUDIO_STREAM_STATE_PAUSED || state == AAUDIO_STREAM_STATE_PAUSING;
}

bool AudioSystem::platformIsInUse(ChannelID pChannel) const
{
   if(!gAAudioChannels[pChannel].Stream)
   {
      return false;
   }

   const aaudio_stream_state_t state = AAudioStream_getState(gAAudioChannels[pChannel].Stream);
   return state != AAUDIO_STREAM_STATE_STOPPED;
}

void AudioSystem::platformSetVolume(ChannelID pChannel, float pVolume)
{
   if(!gAAudioChannels[pChannel].Stream)
   {
      return;
   }

   gAAudioChannels[pChannel].Volume = pVolume;
}

void AudioSystem::platformSetPitch(ChannelID pChannel, float pPitch)
{
   if(!gAAudioChannels[pChannel].Stream)
   {
      return;
   }

   gAAudioChannels[pChannel].Pitch = pPitch;
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
