
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio System (OpenAL)
//
//  --- GEAudioSystemOpenAL.mm ---
//
//////////////////////////////////////////////////////////////////

#include "Audio/GEAudioSystem.h"
#include "Content/GEAudioData.h"
#include "Core/GEPlatform.h"

#include "Externals/OpenAL/include/al.h"
#include "Externals/OpenAL/include/alc.h"

ALCdevice* alDevice = 0;
ALCcontext* alContext = 0;

ALuint* alBuffers = 0;
ALuint* alSources = 0;

using namespace GE;
using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;

void AudioSystem::platformInit()
{
   alDevice = alcOpenDevice(0);
   alContext = alcCreateContext(alDevice, 0);
   alcMakeContextCurrent(alContext);

   alBuffers = Allocator::alloc<ALuint>(mBuffersCount);
   alSources = Allocator::alloc<ALuint>(mChannelsCount);

   alGenBuffers(mBuffersCount, alBuffers);
   alGenSources(mChannelsCount, alSources);
}

void AudioSystem::platformUpdate()
{
}

void AudioSystem::platformRelease()
{
   alDeleteBuffers(mBuffersCount, alBuffers);
   alDeleteSources(mChannelsCount, alSources);

   Allocator::free(alBuffers);
   Allocator::free(alSources);

   alcMakeContextCurrent(0);
   alcDestroyContext(alContext);
   alcCloseDevice(alDevice);

   alBuffers = 0;
   alSources = 0;

   alContext = 0;
   alDevice = 0;
}

void AudioSystem::platformLoadSound(BufferID pBuffer, AudioData* pAudioData)
{
   GEAssert(pBuffer < mBuffersCount);
   
   const ALenum alFormat = pAudioData->getNumberOfChannels() == 1
      ? AL_FORMAT_MONO16
      : AL_FORMAT_STEREO16;
   ALchar* alData = pAudioData->getData();
   const ALsizei alSize = (ALsizei)pAudioData->getDataSize();
   ALsizei alFrequency = (ALsizei)pAudioData->getSampleRate();
      
   alBufferData(alBuffers[pBuffer], alFormat, alData, alSize, alFrequency);
}

void AudioSystem::platformUnloadSound(BufferID pBuffer)
{
   GEAssert(pBuffer < mBuffersCount);

   alBufferData(alBuffers[pBuffer], 0, 0, 0, 0);
}

void AudioSystem::platformPlaySound(ChannelID pChannel, BufferID pBuffer)
{
   alSourcei(alSources[pChannel], AL_BUFFER, alBuffers[pBuffer]);
   alSourcePlay(alSources[pChannel]);
}

void AudioSystem::platformStop(ChannelID pChannel)
{
   alSourceStop(alSources[pChannel]);
}

void AudioSystem::platformPause(ChannelID pChannel)
{
   alSourcePause(alSources[pChannel]);
}

void AudioSystem::platformResume(ChannelID pChannel)
{
   alSourcePlay(alSources[pChannel]);
}

bool AudioSystem::platformIsPlaying(ChannelID pChannel) const
{
   ALenum alState;
   alGetSourcei(pChannel, AL_SOURCE_STATE, &alState);

   return alState == AL_PLAYING;
}

bool AudioSystem::platformIsPaused(ChannelID pChannel) const
{
   ALenum alState;
   alGetSourcei(pChannel, AL_SOURCE_STATE, &alState);

   return alState == AL_PAUSED;
}

bool AudioSystem::platformIsInUse(ChannelID pChannel) const
{
   ALenum alState;
   alGetSourcei(pChannel, AL_SOURCE_STATE, &alState);

   return alState == AL_PLAYING || alState == AL_PAUSED;
}

void AudioSystem::platformSetVolume(ChannelID pChannel, float pVolume)
{
   alSourcef(alSources[pChannel], AL_GAIN, pVolume);
}

void AudioSystem::platformSetPosition(ChannelID pChannel, const Vector3& pPosition)
{
   alSource3f(alSources[pChannel], AL_POSITION, pPosition.X, pPosition.Y, pPosition.Z);
}

void AudioSystem::platformSetListenerPosition(const Vector3& pPosition)
{
   alListener3f(AL_POSITION, pPosition.X, pPosition.Y, pPosition.Z);
}

void AudioSystem::platformSetListenerOrientation(const Quaternion& pOrientation)
{
   Rotation rotation = Rotation(pOrientation);
   Vector3 eulerOrientation = rotation.getEulerAngles() * GE_RAD2DEG;

   alListener3f(AL_DIRECTION, eulerOrientation.X, eulerOrientation.Y, eulerOrientation.Z);
}
