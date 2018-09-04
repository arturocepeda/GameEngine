
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
#include "Core/GETime.h"
#include "Core/GERand.h"
#include "Content/GEResourcesManager.h"

using namespace GE;
using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;

const float UpdatePeriod = 0.5f;

AudioSystem::AudioSystem()
   : mHandler(0)
   , mChannelsCount(0)
   , mBuffersCount(0)
   , mNextBufferToAssign(0)
   , mChannels(0)
   , mAssignmentTime(0)
   , mTimeSinceLastUpdate(0.0f)
{
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<AudioBank>(&mAudioBanks);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<AudioEvent>(&mAudioEvents);
}

AudioSystem::~AudioSystem()
{
}

void AudioSystem::loadAudioEventEntries()
{
   const char* subdir = "Audio";
   const char* extension = "events.xml";

   const uint32_t groupsCount = Device::getContentFilesCount(subdir, extension);

   for(uint32_t i = 0; i < groupsCount; i++)
   {
      char fileName[256];
      Device::getContentFileName(subdir, extension, i, fileName);

      const ObjectName groupName = ObjectName(fileName);
      SerializableResourcesManager::getInstance()->loadFromXml<AudioEvent>(AudioEvent::TypeName, groupName, subdir, fileName, extension);
   }
}

void AudioSystem::loadAudioBankEntries()
{
   const char* subdir = "Audio";
   const char* extension = "banks.xml";

   const uint32_t groupsCount = Device::getContentFilesCount(subdir, extension);

   for(uint32_t i = 0; i < groupsCount; i++)
   {
      char fileName[256];
      Device::getContentFileName(subdir, extension, i, fileName);

      const ObjectName groupName = ObjectName(fileName);
      SerializableResourcesManager::getInstance()->loadFromXml<AudioBank>(AudioBank::TypeName, groupName, subdir, fileName, extension);
   }
}

void AudioSystem::init(uint32_t pChannelsCount, uint32_t pBuffersCount)
{
   mChannelsCount = pChannelsCount;
   mBuffersCount = pBuffersCount;

   GEAssert(mChannelsCount > 0);

   mChannels = Allocator::alloc<AudioChannel>(mChannelsCount);

   platformInit();

   setListenerPosition(Vector3::Zero);

   loadAudioEventEntries();
   loadAudioBankEntries();
}

void AudioSystem::update()
{
   const float deltaTime = Time::getDefaultClock()->getDelta();

   mTimeSinceLastUpdate += deltaTime;

   if(mTimeSinceLastUpdate >= UpdatePeriod)
   {
      mTimeSinceLastUpdate -= UpdatePeriod;

      for(uint32_t i = 0; i < mChannelsCount; i++)
      {
         if(!mChannels[i].Free && !platformIsInUse(i))
         {
            mChannels[i].Free = true;
         }
      }

      platformUpdate();
   }
}

void AudioSystem::release()
{
   unloadAllAudioBanks();
   platformRelease();

   if(mChannels)
   {
      Allocator::free(mChannels);
   }
}

void AudioSystem::loadAudioBank(const ObjectName& pAudioBankName)
{
   AudioBank* audioBank = mAudioBanks.get(pAudioBankName);

   if(audioBank)
   {
      audioBank->load(mAudioEvents);

      const GESTLMap(uint32_t, AudioData*)& audioFiles = audioBank->getAudioFiles();

      for(GESTLMap(uint32_t, AudioData*)::const_iterator it = audioFiles.begin(); it != audioFiles.end(); it++)
      {
         std::pair<uint32_t, BufferID> audioBufferPair;
         audioBufferPair.first = it->first;
         audioBufferPair.second = mNextBufferToAssign;
         mAudioBuffers.insert(audioBufferPair);

         platformLoadSound(mNextBufferToAssign++, it->second);
      }
   }
}

void AudioSystem::unloadAudioBank(const ObjectName& pAudioBankName)
{
   AudioBank* audioBank = mAudioBanks.get(pAudioBankName);

   if(audioBank)
   {
      audioBank->unload();
   }
}

void AudioSystem::unloadAllAudioBanks()
{
   mAudioBanks.iterate([this](AudioBank* pAudioBank)
   {
      pAudioBank->unload();
      return true;
   });
}

ChannelID AudioSystem::playAudioEvent(const ObjectName& pAudioBankName, const ObjectName& pAudioEventName)
{
   // select channel to play the sound
   ChannelID selectedChannel = 0;
   uint32_t oldestAssignmentTime = mChannels[0].AssignmentTime;

   for(ChannelID currentChannel = 0; currentChannel < mChannelsCount; currentChannel++)
   {
      if(mChannels[currentChannel].Free)
      {
         selectedChannel = currentChannel;
         break;
      }

      if(mChannels[currentChannel].AssignmentTime < oldestAssignmentTime)
      {
         oldestAssignmentTime = mChannels[currentChannel].AssignmentTime;
         selectedChannel = currentChannel;
      }
   }

   // play the event through the selected channel
   playAudioEvent(pAudioBankName, pAudioEventName, selectedChannel);

   // return the selected channel
   return selectedChannel;
}

void AudioSystem::playAudioEvent(const ObjectName& pAudioBankName, const ObjectName& pAudioEventName, ChannelID pChannel)
{
   AudioBank* audioBank = mAudioBanks.get(pAudioBankName);

   if(!audioBank)
      return;

   AudioEvent* audioEvent = audioBank->getAudioEvent(pAudioEventName);

   if(!audioEvent || audioEvent->getAudioFileCount() == 0)
      return;

   int audioFileIndex = 0;

   if(audioEvent->getAudioFileCount() > 1)
   {
      RandInt rand = RandInt(0, audioEvent->getAudioFileCount() - 1);
      audioFileIndex = rand.generate();
   }

   const ObjectName& audioFileName = audioEvent->getAudioFile(audioFileIndex)->getFileName();
   GESTLMap(uint32_t, BufferID)::iterator it = mAudioBuffers.find(audioFileName.getID());

   if(it == mAudioBuffers.end())
      return;

   const BufferID bufferID = it->second;

   mChannels[pChannel].AssignmentTime = mAssignmentTime++;
   mChannels[pChannel].AssignedBuffer = bufferID;
   mChannels[pChannel].Free = false;

   platformPlaySound(pChannel, bufferID);
}

void AudioSystem::stop(ChannelID pChannel)
{
   mChannels[pChannel].Free = true;

   platformStop(pChannel);
}

void AudioSystem::pause(ChannelID pChannel)
{
   platformPause(pChannel);
}

void AudioSystem::resume(ChannelID pChannel)
{
   platformResume(pChannel);
}

bool AudioSystem::isPlaying(ChannelID pChannel) const
{
   return platformIsPlaying(pChannel);
}

bool AudioSystem::isPaused(ChannelID pChannel) const
{
   return platformIsPaused(pChannel);
}

void AudioSystem::setVolume(ChannelID pChannel, float pVolume)
{
   platformSetVolume(pChannel, pVolume);
}

void AudioSystem::setPosition(ChannelID pChannel, const Vector3& pPosition)
{
   platformSetPosition(pChannel, pPosition);
}

void AudioSystem::setListenerPosition(const Vector3& pPosition)
{
   mListenerPosition = pPosition;

   platformSetListenerPosition(pPosition);
}

void AudioSystem::setListenerOrientation(const Quaternion& pOrientation)
{
   mListenerOrientation = pOrientation;

   platformSetListenerOrientation(pOrientation);
}
