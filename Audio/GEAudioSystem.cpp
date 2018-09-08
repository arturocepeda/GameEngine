
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
#include "Core/GELog.h"
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
   , mChannelAssignmentIndex(0)
   , mAudioEventInstanceAssignmentIndex(0)
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

void AudioSystem::releaseChannel(ChannelID pChannel)
{
   mChannels[pChannel].Free = true;

   const size_t audioEventInstancesCount = mActiveAudioEventInstances.size();

   for(size_t i = 0; i < audioEventInstancesCount; i++)
   {
      if(mActiveAudioEventInstances[i]->Channel == pChannel)
      {
         mActiveAudioEventInstances[i]->Active = false;
         mActiveAudioEventInstances[i] = mActiveAudioEventInstances[audioEventInstancesCount - 1];
         mActiveAudioEventInstances.pop_back();
         break;
      }
   }
}

void AudioSystem::init(uint32_t pChannelsCount, uint32_t pBuffersCount)
{
   mChannelsCount = pChannelsCount;
   mBuffersCount = pBuffersCount;

   GEAssert(mChannelsCount > 0);

   mChannels = Allocator::alloc<AudioChannel>(mChannelsCount);

   for(uint32_t i = 0; i < mChannelsCount; i++)
   {
      GEInvokeCtor(AudioChannel, &mChannels[i])();
   }

   platformInit();

   setListenerPosition(Vector3::Zero);

   loadAudioEventEntries();
   loadAudioBankEntries();

   GEMutexInit(mMutex);
}

void AudioSystem::update()
{
   const float deltaTime = Time::getDefaultClock()->getDelta();

   mTimeSinceLastUpdate += deltaTime;

   if(mTimeSinceLastUpdate >= UpdatePeriod)
   {
      mTimeSinceLastUpdate -= UpdatePeriod;

      GEMutexLock(mMutex);

      for(uint32_t i = 0; i < mChannelsCount; i++)
      {
         if(!mChannels[i].Free && !platformIsInUse(i))
         {
            releaseChannel(i);
         }
      }

      GEMutexUnlock(mMutex);

      platformUpdate();
   }
}

void AudioSystem::release()
{
   GEMutexDestroy(mMutex);

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

AudioEventInstance* AudioSystem::playAudioEvent(const ObjectName& pAudioBankName, const ObjectName& pAudioEventName)
{
   AudioBank* audioBank = mAudioBanks.get(pAudioBankName);

   if(!audioBank)
   {
      Log::log(LogType::Warning, "The '%s' audio bank does not exist", pAudioBankName.getString());
      return 0;
   }

   if(!audioBank->getLoaded())
   {
      Log::log(LogType::Warning, "The '%s' audio bank has not been loaded", pAudioBankName.getString());
      return 0;
   }

   AudioEvent* audioEvent = audioBank->getAudioEvent(pAudioEventName);

   if(!audioEvent)
   {
      Log::log(LogType::Warning, "The '%s' audio bank does not contain the '%s' audio event", pAudioBankName.getString(), pAudioEventName.getString());
      return 0;
   }

   if(audioEvent->getAudioFileCount() == 0)
   {
      Log::log(LogType::Warning, "The '%s' audio event does not contain any audio files", pAudioEventName.getString());
      return 0;
   }

   int audioFileIndex = 0;

   if(audioEvent->getAudioFileCount() > 1)
   {
      RandInt rand = RandInt(0, audioEvent->getAudioFileCount() - 1);
      audioFileIndex = rand.generate();
   }

   const ObjectName& audioFileName = audioEvent->getAudioFile(audioFileIndex)->getFileName();

   GESTLMap(uint32_t, BufferID)::iterator it = mAudioBuffers.find(audioFileName.getID());

   if(it == mAudioBuffers.end())
      return 0;

   const BufferID bufferID = it->second;

   GEMutexLock(mMutex);

   // select channel to play the sound
   ChannelID selectedChannel = 0;
   uint32_t oldestAssignmentIndex = mChannels[0].AssignmentIndex;

   for(ChannelID currentChannel = 0; currentChannel < mChannelsCount; currentChannel++)
   {
      if(mChannels[currentChannel].Free)
      {
         selectedChannel = currentChannel;
         break;
      }

      if(mChannels[currentChannel].AssignmentIndex < oldestAssignmentIndex)
      {
         oldestAssignmentIndex = mChannels[currentChannel].AssignmentIndex;
         selectedChannel = currentChannel;
      }
   }

   mChannels[selectedChannel].AssignmentIndex = mChannelAssignmentIndex++;
   mChannels[selectedChannel].AssignedBuffer = bufferID;
   mChannels[selectedChannel].Free = false;

   // assign event instance
   AudioEventInstance* audioEventInstance = &mAudioEventInstances[mAudioEventInstanceAssignmentIndex++];

   if(mAudioEventInstanceAssignmentIndex == AudioEventInstancesCount)
   {
      mAudioEventInstanceAssignmentIndex = 0;
   }

   mActiveAudioEventInstances.push_back(audioEventInstance);

   GEMutexUnlock(mMutex);

   audioEventInstance->Event = audioEvent;
   audioEventInstance->Channel = selectedChannel;
   audioEventInstance->Active = true;

   // play the sound
   platformPlaySound(selectedChannel, bufferID);

   // return the event instance
   return audioEventInstance;
}

void AudioSystem::stop(AudioEventInstance* pAudioEventInstance)
{
   if(!pAudioEventInstance->Active)
      return;

   GEMutexLock(mMutex);
   releaseChannel(pAudioEventInstance->Channel);
   GEMutexUnlock(mMutex);

   platformStop(pAudioEventInstance->Channel);
}

void AudioSystem::pause(AudioEventInstance* pAudioEventInstance)
{
   if(!pAudioEventInstance->Active)
      return;

   platformPause(pAudioEventInstance->Channel);
}

void AudioSystem::resume(AudioEventInstance* pAudioEventInstance)
{
   if(!pAudioEventInstance->Active)
      return;

   platformResume(pAudioEventInstance->Channel);
}

bool AudioSystem::isPlaying(AudioEventInstance* pAudioEventInstance) const
{
   return pAudioEventInstance->Active && platformIsPlaying(pAudioEventInstance->Channel);
}

bool AudioSystem::isPaused(AudioEventInstance* pAudioEventInstance) const
{
   return pAudioEventInstance->Active && platformIsPaused(pAudioEventInstance->Channel);
}

void AudioSystem::setVolume(AudioEventInstance* pAudioEventInstance, float pVolume)
{
   if(!pAudioEventInstance->Active)
      return;

   platformSetVolume(pAudioEventInstance->Channel, pVolume);
}

void AudioSystem::setPosition(AudioEventInstance* pAudioEventInstance, const Vector3& pPosition)
{
   if(!pAudioEventInstance->Active)
      return;

   platformSetPosition(pAudioEventInstance->Channel, pPosition);
}

void AudioSystem::setOrientation(AudioEventInstance* pAudioEventInstance, const Rotation& pOrientation)
{
   if(!pAudioEventInstance->Active)
      return;

   platformSetOrientation(pAudioEventInstance->Channel, pOrientation);
}

void AudioSystem::setMinDistance(AudioEventInstance* pAudioEventInstance, float pDistance)
{
   if(!pAudioEventInstance->Active)
      return;

   platformSetMinDistance(pAudioEventInstance->Channel, pDistance);
}

void AudioSystem::setMaxDistance(AudioEventInstance* pAudioEventInstance, float pDistance)
{
   if(!pAudioEventInstance->Active)
      return;

   platformSetMaxDistance(pAudioEventInstance->Channel, pDistance);
}

void AudioSystem::setListenerPosition(const Vector3& pPosition)
{
   if(!mListenerPosition.equals(pPosition))
   {
      mListenerPosition = pPosition;

      platformSetListenerPosition(pPosition);
   }
}

void AudioSystem::setListenerOrientation(const Rotation& pOrientation)
{
   if(!mListenerOrientation.getQuaternion().equals(pOrientation.getQuaternion()))
   {
      mListenerOrientation = pOrientation;

      platformSetListenerOrientation(pOrientation);
   }
}
