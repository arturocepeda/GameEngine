
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio (FMOD)
//
//  --- GEAudioSystemFMOD.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAudioSystemFMOD.h"
#include "Core/GEDevice.h"
#include "Content/GEAudioData.h"

using namespace GE;
using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;

#define FMOD_DSP_BUFFER_SIZE 512

AudioSystemFMOD::AudioSystemFMOD()
{
    memset(fmodSounds, 0, sizeof(FMOD::Sound*) * GE_AUDIO_SOUNDS);
    memset(fmodChannels, 0, sizeof(FMOD::Channel*) * GE_AUDIO_CHANNELS);

    for(uint i = 0; i < GE_AUDIO_CHANNELS; i++)
       fVolume[i] = 1.0f;
}

AudioSystemFMOD::~AudioSystemFMOD()
{
}

void AudioSystemFMOD::internalInit()
{
   // create a system object and initialize.
   FMOD::System_Create(&fmodSystem);
   pHandler = fmodSystem;

   // set a small DSP buffer size to achieve low latency
   fmodSystem->setDSPBufferSize(FMOD_DSP_BUFFER_SIZE, 4);

   // initialize FMOD system
   fmodSystem->init(GE_AUDIO_CHANNELS, FMOD_INIT_NORMAL, NULL);
}

void AudioSystemFMOD::update()
{
    fmodSystem->update();
}

void AudioSystemFMOD::release()
{    
    fmodSystem->close();
    fmodSystem->release();
}

void AudioSystemFMOD::loadSound(uint Sound, const char* FileName, const char* FileExtension)
{
   if(Sound >= GE_AUDIO_SOUNDS)
      return;

   unloadSound(Sound);

   char sBuffer[256];
   sprintf(sBuffer, "Sounds\\%s.%s", FileName, FileExtension);

   fmodSystem->createSound(sBuffer, FMOD_SOFTWARE | FMOD_CREATESAMPLE | FMOD_3D, 0, &fmodSounds[Sound]);
}

void AudioSystemFMOD::unloadSound(uint Sound)
{
    if(Sound >= GE_AUDIO_SOUNDS || !fmodSounds[Sound])
        return;

    fmodSounds[Sound]->release();
    fmodSounds[Sound] = NULL;
}

void AudioSystemFMOD::internalPlaySound(uint Sound, uint Channel)
{
    fmodSystem->playSound(FMOD_CHANNEL_FREE, fmodSounds[Sound], true, &fmodChannels[Channel]);

    FMOD_VECTOR fmodChannelPosition = { vPosition[Channel].X, vPosition[Channel].Y, vPosition[Channel].Z };
    fmodChannels[Channel]->set3DAttributes(&fmodChannelPosition, NULL);
    fmodChannels[Channel]->setVolume(fVolume[Channel]);

    fmodChannels[Channel]->setPaused(false);
}

void AudioSystemFMOD::internalStop(uint Channel)
{
    fmodChannels[Channel]->stop();
}

bool AudioSystemFMOD::isPlaying(uint Channel)
{
    bool bPlaying;
    fmodChannels[Channel]->isPlaying(&bPlaying);

    return bPlaying;
}

void AudioSystemFMOD::setListenerPosition(const Vector3& Position)
{    
    FMOD_VECTOR fmodListenerPosition = { Position.X, Position.Y, Position.Z };
    fmodSystem->set3DListenerAttributes(0, &fmodListenerPosition, NULL, NULL, NULL);
}

void AudioSystemFMOD::setVolume(uint Channel, float Volume)
{
   fVolume[Channel] = Volume;

   if(!fmodChannels[Channel])
      return;

   bool bPlaying;
   fmodChannels[Channel]->isPlaying(&bPlaying);

   if(bPlaying)
     fmodChannels[Channel]->setVolume(Volume);
}

void AudioSystemFMOD::setPosition(uint Channel, const Vector3& Position)
{
   vPosition[Channel] = Position;

   if(!fmodChannels[Channel])
      return;

   bool bPlaying;
   fmodChannels[Channel]->isPlaying(&bPlaying);

   if(bPlaying)
   {
      FMOD_VECTOR fmodChannelPosition = { Position.X, Position.Y, Position.Z };
      fmodChannels[Channel]->set3DAttributes(&fmodChannelPosition, NULL);
   }
}
