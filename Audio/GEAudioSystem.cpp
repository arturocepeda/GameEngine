
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio
//
//  --- GEAudioSystem.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAudioSystem.h"
#include "Core/GEAllocator.h"

using namespace GE;
using namespace GE::Audio;
using namespace GE::Core;

AudioSystem::AudioSystem()
   : pHandler(0)
   , iChannels(0)
   , iSounds(0)
   , sChannels(0)
   , iAssignmentTime(0)
{
}

AudioSystem::~AudioSystem()
{
   if(sChannels)
      Allocator::free(sChannels);
}

void AudioSystem::init(uint Channels, uint Sounds)
{
   iChannels = Channels;
   iSounds = Sounds;
   
   if(Channels > 0)
      sChannels = Allocator::alloc<AudioChannel>(Channels);
   
   internalInit();
}

void AudioSystem::update()
{
}

void* AudioSystem::getHandler()
{
   return pHandler;
}

uint AudioSystem::playSound(uint Sound)
{
   // select channel to play the sound
   uint iCurrentChannel = 0;
   uint iSelectedChannel = 0;
   uint iOldestAssignmentTime = sChannels[0].AssignmentTime;

   for(; iCurrentChannel < iChannels; iCurrentChannel++)
   {
      if(sChannels[iCurrentChannel].Free)
      {
         iSelectedChannel = iCurrentChannel;
         break;
      }

      if(sChannels[iCurrentChannel].AssignmentTime < iOldestAssignmentTime)
      {
         iOldestAssignmentTime = sChannels[iCurrentChannel].AssignmentTime;
         iSelectedChannel = iCurrentChannel;
      }
   }

   // play the sound through the selected channel
   playSound(Sound, iSelectedChannel);

   // return the selected channel
   return iSelectedChannel;
}

void AudioSystem::playSound(uint Sound, uint Channel)
{
   sChannels[Channel].AssignmentTime = iAssignmentTime++;
   sChannels[Channel].AssignedSound = Sound;
   sChannels[Channel].Free = false;

   internalPlaySound(Sound, Channel);
}

void AudioSystem::stop(uint Channel)
{
   sChannels[Channel].Free = true;

   internalStop(Channel);
}

void AudioSystem::unloadAllSounds()
{
   for(uint i = 0; i < iSounds; i++)
      unloadSound(i);
}
