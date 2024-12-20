
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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
#include "Core/GEDevice.h"
#include "Core/GETaskManager.h"
#include "Content/GEResourcesManager.h"
#include "Content/GEAudioData.h"

using namespace GE;
using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;

//
//  AudioBus
//
const ObjectName AudioBus::TypeName = ObjectName("AudioBus");
const char* AudioBus::SubDir = "Audio";
const char* AudioBus::Extension = "buses";

AudioBus::AudioBus(const ObjectName& pName, const ObjectName& pGroupName)
   : Resource(pName, pGroupName, TypeName)
   , mParent(nullptr)
   , mVolume(1.0f)
{
   GERegisterProperty(ObjectName, Parent);
   GERegisterProperty(Float, Volume);
   GERegisterPropertyReadonly(Float, DerivedVolume);
}

AudioBus::~AudioBus()
{
}

const ObjectName& AudioBus::getParent() const
{
   return mParent ? mParent->getName() : ObjectName::Empty;
}

void AudioBus::setParent(const ObjectName& pName)
{
   if(pName != cName)
   {
      mParent = AudioSystem::getInstance()->getAudioBus(pName);
   }
}

float AudioBus::getDerivedVolume() const
{
   return mVolume * (mParent ? mParent->getDerivedVolume() : 1.0f);
}


//
//  AudioSystem
//
const ObjectName AudioSystem::MasterBusName = ObjectName("Master");

AudioSystem::AudioSystem()
   : mHandler(nullptr)
   , mChannelsCount(0u)
   , mBuffersCount(0u)
   , mChannels(nullptr)
   , mChannelAssignmentIndex(0u)
   , mBuffers(nullptr)
   , mAudioEventInstanceAssignmentIndex(0u)
   , mAudioStreamAssignmentIndex(0u)
{
   AudioBus* masterBus = Allocator::alloc<AudioBus>();
   GEInvokeCtor(AudioBus, masterBus)(MasterBusName, ObjectName::Empty);
   mAudioBuses.add(masterBus);

   SerializableResourcesManager::getInstance()->registerSerializableResourceType<AudioBank>(&mAudioBanks);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<AudioEvent>(&mAudioEvents);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<AudioBus>(&mAudioBuses);
}

AudioSystem::~AudioSystem()
{
   release();
}

void AudioSystem::releaseChannel(ChannelID pChannel)
{
   mChannels[pChannel].Free = true;

   const size_t audioEventInstancesCount = mActiveAudioEventInstances.size();

   for(size_t i = 0u; i < audioEventInstancesCount; i++)
   {
      if(mActiveAudioEventInstances[i]->Channel == pChannel)
      {
         mActiveAudioEventInstances[i]->reset();
         mActiveAudioEventInstances[i] = mActiveAudioEventInstances[audioEventInstancesCount - 1];
         mActiveAudioEventInstances.pop_back();
         break;
      }
   }

   platformReleaseChannel(pChannel);
}

void AudioSystem::loadAudioBankFiles(AudioBank* pAudioBank, LoadingMode pLoadingMode)
{
   const GESTLVector(ObjectName)& audioFileNames = pAudioBank->getAudioFileNames();
   GESTLVector(BufferID) bufferIndicesToLoad;

   for(size_t i = 0u; i < audioFileNames.size(); i++)
   {
      const ObjectName& audioFileName = audioFileNames[i];

      // assign buffer index
      GEMutexLock(mMutex);

      const BufferID invalidBufferIndex = mBuffersCount;
      BufferID loadedFileIndex = invalidBufferIndex;
      BufferID firstFreeIndex = invalidBufferIndex;

      for(BufferID bufferID = 0u; bufferID < mBuffersCount; bufferID++)
      {
         if(mBuffers[bufferID].AssignedFileName == audioFileName)
         {
            loadedFileIndex = bufferID;
            break;
         }
         else if(mBuffers[bufferID].References == 0u)
         {
            if(firstFreeIndex == invalidBufferIndex)
            {
               firstFreeIndex = bufferID;
            }
         }
      }

      const BufferID bufferIndexToAssign = loadedFileIndex != invalidBufferIndex
         ? loadedFileIndex
         : firstFreeIndex;

      GEAssert(bufferIndexToAssign < mBuffersCount);

      mBuffers[bufferIndexToAssign].AssignedFileName = audioFileName;
      mBuffers[bufferIndexToAssign].References++;

      GEMutexUnlock(mMutex);

      if(mBuffers[bufferIndexToAssign].References == 1u)
      {
         bufferIndicesToLoad.push_back(bufferIndexToAssign);
      }
   }

   if(pLoadingMode == LoadingMode::Blocking)
   {
      for(size_t i = 0u; i < bufferIndicesToLoad.size(); i++)
      {
         const uint32_t bufferIndex = bufferIndicesToLoad[i];
         loadAudioFile(bufferIndex);
      }
   }
   else
   {
      pAudioBank->setState(AudioBankState::Loading);
      pAudioBank->resetAsyncLoadedAudioFiles();

      const size_t bufferIndicesToLoadCount = bufferIndicesToLoad.size();

      for(size_t i = 0u; i < bufferIndicesToLoadCount; i++)
      {
         const uint32_t bufferIndex = bufferIndicesToLoad[i];

         JobDesc jobDesc("LoadAudioFile");
         jobDesc.Task = [this, bufferIndex, pAudioBank, bufferIndicesToLoadCount]
         {
            loadAudioFile(bufferIndex);

            pAudioBank->incrementAsyncLoadedAudioFiles();

            if(pAudioBank->getAsyncLoadedAudioFiles() == bufferIndicesToLoadCount)
            {
               pAudioBank->setState(AudioBankState::Loaded);
            }
         };
         TaskManager::getInstance()->queueJob(jobDesc);
      }
   }
}

void AudioSystem::releaseAudioBankFiles(AudioBank* pAudioBank)
{
   GEMutexLock(mMutex);

   const GESTLVector(ObjectName)& audioFileNames = pAudioBank->getAudioFileNames();

   for(size_t i = 0u; i < audioFileNames.size(); i++)
   {
      const ObjectName& audioFileName = audioFileNames[i];

      for(BufferID bufferID = 0u; bufferID < mBuffersCount; bufferID++)
      {
         if(mBuffers[bufferID].AssignedFileName == audioFileName)
         {
            mBuffers[bufferID].References--;

            if(mBuffers[bufferID].References == 0u)
            {
               for(ChannelID channelID = 0u; channelID < mChannelsCount; channelID++)
               {
                  if(!mChannels[channelID].Free && mChannels[channelID].AssignedBuffer == bufferID)
                  {
                     platformStop(channelID);
                     releaseChannel(channelID);
                  }
               }

               platformUnloadSound(bufferID);

               GEInvokeDtor(AudioData, mBuffers[bufferID].Data);
               Allocator::free(mBuffers[bufferID].Data);

               mBuffers[bufferID].AssignedFileName = ObjectName::Empty;
               mBuffers[bufferID].Data = nullptr;
            }

            break;
         }
      }
   }

   GEMutexUnlock(mMutex);
}

void AudioSystem::loadAudioFile(BufferID pBuffer)
{
   static const char* kAudioFilesSubdir = "Audio/files";

   mBuffers[pBuffer].Data = Allocator::alloc<AudioData>(1, AllocationCategory::Audio);
   GEInvokeCtor(AudioData, mBuffers[pBuffer].Data)();

   Device::readContentFile
   (
      ContentType::Audio,
      kAudioFilesSubdir,
      mBuffers[pBuffer].AssignedFileName.getString(),
      platformAudioFileExtension(),
      mBuffers[pBuffer].Data
   );

   platformLoadSound(pBuffer, mBuffers[pBuffer].Data);
}

void AudioSystem::init(uint32_t pChannelsCount, uint32_t pBuffersCount)
{
   mChannelsCount = pChannelsCount;
   mBuffersCount = pBuffersCount;

   GEAssert(mChannelsCount > 0u);

   mChannels = Allocator::alloc<AudioChannel>(mChannelsCount);

   for(ChannelID i = 0u; i < mChannelsCount; i++)
   {
      GEInvokeCtor(AudioChannel, &mChannels[i])();
   }

   GEAssert(mBuffersCount > 0u);

   mBuffers = Allocator::alloc<AudioBuffer>(mBuffersCount);

   for(BufferID i = 0u; i < mBuffersCount; i++)
   {
      GEInvokeCtor(AudioBuffer, &mBuffers[i])();
   }

   platformInit();

   setListenerPosition(Vector3::Zero);

   SerializableResourcesManager::getInstance()->loadAll<AudioEvent>();
   SerializableResourcesManager::getInstance()->loadAll<AudioBank>();
   SerializableResourcesManager::getInstance()->loadAll<AudioBus>();

   GEMutexInit(mMutex);
}

void AudioSystem::update()
{
   const float deltaTime = Time::getDelta();

   // audio system update
   GEMutexLock(mMutex);

   for(ChannelID i = 0u; i < mChannelsCount; i++)
   {
      if(!mChannels[i].Free && !platformIsInUse(i))
      {
         releaseChannel(i);
      }
   }

   platformUpdate();

   // handle fades
   for(size_t i = 0u; i < mActiveAudioEventInstances.size(); )
   {
      AudioEventInstance* instance = mActiveAudioEventInstances[i];
      
      if(instance->State == AudioEventInstanceState::FadingIn &&
         !platformIsPaused(instance->Channel))
      {
         instance->VolumeFactorFade += (1.0f / instance->Event->getFadeInTime()) * deltaTime;

         if(instance->VolumeFactorFade >= 1.0f)
         {
            instance->VolumeFactorFade = 1.0f;
            instance->State = AudioEventInstanceState::Playing;
         }

         platformSetVolume(instance->Channel, instance->getVolume());
      }
      else if(instance->State == AudioEventInstanceState::FadingOut &&
         !platformIsPaused(instance->Channel))
      {
         instance->VolumeFactorFade -= (1.0f / instance->Event->getFadeOutTime()) * deltaTime;

         if(instance->VolumeFactorFade < GE_EPSILON)
         {
            platformStop(instance->Channel);
            releaseChannel(instance->Channel);
            continue;
         }
         else
         {
            platformSetVolume(instance->Channel, instance->getVolume());
         }
      }

      i++;
   }

   GEMutexUnlock(mMutex);
}

void AudioSystem::release()
{
   unloadAllAudioBanks();
   platformRelease();

   GEMutexDestroy(mMutex);

   if(mBuffers)
   {
      Allocator::free(mBuffers);
      mBuffers = nullptr;
   }

   if(mChannels)
   {
      Allocator::free(mChannels);
      mChannels = nullptr;
   }
}

void AudioSystem::loadAudioBank(const ObjectName& pAudioBankName, LoadingMode pLoadingMode)
{
   AudioBank* audioBank = mAudioBanks.get(pAudioBankName);

   if(audioBank && audioBank->getState() == AudioBankState::Unloaded)
   {
      audioBank->load(mAudioEvents);
      loadAudioBankFiles(audioBank, pLoadingMode);
   }
}

AudioBankState AudioSystem::getAudioBankState(const Core::ObjectName& pAudioBankName) const
{
   AudioBank* audioBank = mAudioBanks.get(pAudioBankName);
   return audioBank ? audioBank->getState() : AudioBankState::Unloaded;
}

void AudioSystem::unloadAudioBank(const ObjectName& pAudioBankName)
{
   AudioBank* audioBank = mAudioBanks.get(pAudioBankName);

   if(audioBank && audioBank->getState() == AudioBankState::Loaded)
   {
      releaseAudioBankFiles(audioBank);
      audioBank->unload();
   }
}

void AudioSystem::unloadAllAudioBanks()
{
   mAudioBanks.iterate([this](AudioBank* pAudioBank)
   {
      releaseAudioBankFiles(pAudioBank);
      pAudioBank->unload();
      return true;
   });
}

AudioBus* AudioSystem::getAudioBus(const ObjectName& pAudioBusName) const
{
   return mAudioBuses.get(pAudioBusName);
}

AudioEventInstance* AudioSystem::playAudioEvent(const ObjectName& pAudioBankName,
   const ObjectName& pAudioEventName, AudioBus* pAudioBus)
{
   AudioBank* audioBank = mAudioBanks.get(pAudioBankName);

   if(!audioBank)
   {
      Log::log(LogType::Warning, "The '%s' audio bank does not exist", pAudioBankName.getString());
      return nullptr;
   }

   if(audioBank->getState() != AudioBankState::Loaded)
   {
      Log::log(LogType::Warning, "The '%s' audio bank has not been loaded", pAudioBankName.getString());
      return nullptr;
   }

   AudioEvent* audioEvent = audioBank->getAudioEvent(pAudioEventName);

   if(!audioEvent)
   {
      Log::log(LogType::Warning, "The '%s' audio bank does not contain the '%s' audio event", pAudioBankName.getString(), pAudioEventName.getString());
      return nullptr;
   }

   if(audioEvent->getAudioFileCount() == 0)
   {
      Log::log(LogType::Warning, "The '%s' audio event does not contain any audio files", pAudioEventName.getString());
      return nullptr;
   }

   int audioFileIndex = 0;

   if(audioEvent->getAudioFileCount() > 1u)
   {
      RandInt rand(0, (int)audioEvent->getAudioFileCount() - 1);
      audioFileIndex = rand.generate();
   }

   const ObjectName& audioFileName = audioEvent->getAudioFile(audioFileIndex)->getFileName();

   const BufferID InvalidBufferIndex = mBuffersCount;
   BufferID bufferID = InvalidBufferIndex;

   for(BufferID i = 0u; i < mBuffersCount; i++)
   {
      if(mBuffers[i].AssignedFileName == audioFileName)
      {
         bufferID = i;
         break;
      }
   }

   if(bufferID == InvalidBufferIndex)
      return nullptr;

   GEMutexLock(mMutex);

   // select channel to play the sound
   ChannelID selectedChannel = 0u;
   uint32_t oldestAssignmentIndex = mChannels[0].AssignmentIndex;
   bool channelStolen = true;

   for(ChannelID currentChannel = 0u; currentChannel < mChannelsCount; currentChannel++)
   {
      if(mChannels[currentChannel].Free)
      {
         selectedChannel = currentChannel;
         channelStolen = false;
         break;
      }

      if(mChannels[currentChannel].AssignmentIndex < oldestAssignmentIndex)
      {
         oldestAssignmentIndex = mChannels[currentChannel].AssignmentIndex;
         selectedChannel = currentChannel;
      }
   }

   if(channelStolen)
   {
      platformStop(selectedChannel);
      releaseChannel(selectedChannel);
   }

   mChannels[selectedChannel].AssignmentIndex = mChannelAssignmentIndex++;
   mChannels[selectedChannel].AssignedBuffer = bufferID;
   mChannels[selectedChannel].Free = false;

   // assign event instance
   AudioEventInstance* audioEventInstance = nullptr;

   do
   {
      audioEventInstance = &mAudioEventInstances[mAudioEventInstanceAssignmentIndex++];

      if(mAudioEventInstanceAssignmentIndex == AudioEventInstancesCount)
      {
         mAudioEventInstanceAssignmentIndex = 0u;
      }
   }
   while(audioEventInstance->State != AudioEventInstanceState::Free);

   if(audioEvent->getFadeInTime() < GE_EPSILON)
   {
      audioEventInstance->State = AudioEventInstanceState::Playing;
   }
   else
   {
      audioEventInstance->State = AudioEventInstanceState::FadingIn;
      audioEventInstance->VolumeFactorFade = 0.0f;
   }
   
   mActiveAudioEventInstances.push_back(audioEventInstance);

   audioEventInstance->Event = audioEvent;
   audioEventInstance->Channel = selectedChannel;
   audioEventInstance->Bus = pAudioBus;

   // play the sound
   const bool looping = audioEvent->getPlayMode() == AudioEventPlayMode::Loop;
   platformPlaySound(selectedChannel, bufferID, looping);
   
   float pitch = audioEvent->getPitchRange().X;
   
   if(!GEFloatEquals(audioEvent->getPitchRange().X, audioEvent->getPitchRange().Y))
   {
      RandFloat rand(audioEvent->getPitchRange().X, audioEvent->getPitchRange().Y);
      pitch = rand.generate();
   }
   
   platformSetVolume(selectedChannel, audioEventInstance->getVolume());
   platformSetPitch(selectedChannel, pitch);

   GEMutexUnlock(mMutex);

   // return the event instance
   return audioEventInstance;
}

void AudioSystem::stop(AudioEventInstance* pAudioEventInstance)
{
   if(pAudioEventInstance->State == AudioEventInstanceState::Free)
      return;

   if(pAudioEventInstance->Event->getFadeOutTime() < GE_EPSILON ||
      platformIsPaused(pAudioEventInstance->Channel))
   {
      GEMutexLock(mMutex);

      platformStop(pAudioEventInstance->Channel);
      releaseChannel(pAudioEventInstance->Channel);

      GEMutexUnlock(mMutex);
   }
   else
   {
      pAudioEventInstance->State = AudioEventInstanceState::FadingOut;
   }
}

void AudioSystem::pause(AudioEventInstance* pAudioEventInstance)
{
   if(pAudioEventInstance->State == AudioEventInstanceState::Free)
      return;

   platformPause(pAudioEventInstance->Channel);
}

void AudioSystem::resume(AudioEventInstance* pAudioEventInstance)
{
   if(pAudioEventInstance->State == AudioEventInstanceState::Free)
      return;

   platformResume(pAudioEventInstance->Channel);
}

bool AudioSystem::isPlaying(AudioEventInstance* pAudioEventInstance) const
{
   return pAudioEventInstance->State != AudioEventInstanceState::Free &&
      platformIsPlaying(pAudioEventInstance->Channel);
}

bool AudioSystem::isPaused(AudioEventInstance* pAudioEventInstance) const
{
   return pAudioEventInstance->State != AudioEventInstanceState::Free &&
      platformIsPaused(pAudioEventInstance->Channel);
}

void AudioSystem::setVolume(AudioEventInstance* pAudioEventInstance, float pVolume)
{
   if(pAudioEventInstance->State == AudioEventInstanceState::Free)
      return;

   pAudioEventInstance->VolumeBase = pVolume;
   platformSetVolume(pAudioEventInstance->Channel, pAudioEventInstance->getVolume());
}

void AudioSystem::setPosition(AudioEventInstance* pAudioEventInstance, const Vector3& pPosition)
{
   if(pAudioEventInstance->State == AudioEventInstanceState::Free)
      return;

   platformSetPosition(pAudioEventInstance->Channel, pPosition);
}

void AudioSystem::setOrientation(AudioEventInstance* pAudioEventInstance, const Rotation& pOrientation)
{
   if(pAudioEventInstance->State == AudioEventInstanceState::Free)
      return;

   platformSetOrientation(pAudioEventInstance->Channel, pOrientation);
}

void AudioSystem::setMinDistance(AudioEventInstance* pAudioEventInstance, float pDistance)
{
   if(pAudioEventInstance->State == AudioEventInstanceState::Free)
      return;

   platformSetMinDistance(pAudioEventInstance->Channel, pDistance);
}

void AudioSystem::setMaxDistance(AudioEventInstance* pAudioEventInstance, float pDistance)
{
   if(pAudioEventInstance->State == AudioEventInstanceState::Free)
      return;

   platformSetMaxDistance(pAudioEventInstance->Channel, pDistance);
}

void AudioSystem::pauseAll()
{
   for(size_t i = 0u; i < mActiveAudioEventInstances.size(); i++)
   {
      platformPause(mActiveAudioEventInstances[i]->Channel);
   }
}

void AudioSystem::resumeAll()
{
   for(size_t i = 0u; i < mActiveAudioEventInstances.size(); i++)
   {
      platformResume(mActiveAudioEventInstances[i]->Channel);
   }
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
