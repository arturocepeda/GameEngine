
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Audio
//
//  --- GEAudioBank.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAudioBank.h"

using namespace GE;
using namespace GE::Audio;
using namespace GE::Core;
using namespace GE::Content;


//
//  AudioEventEntry
//
const ObjectName AudioEventEntryName = ObjectName("AudioEventEntry");

AudioEventEntry::AudioEventEntry()
   : SerializableArrayElement(AudioEventEntryName)
{
   GERegisterProperty(ObjectName, EventName);
}

AudioEventEntry::~AudioEventEntry()
{
}


//
//  AudioBank
//
const ObjectName AudioBank::TypeName = ObjectName("AudioBank");
const char* AudioBank::SubDir = "Audio";
const char* AudioBank::Extension = "banks";

AudioBank::AudioBank(const ObjectName& pName, const ObjectName& pGroupName)
   : Resource(pName, pGroupName, TypeName)
   , mType(AudioBankType::Buffered)
   , mState(AudioBankState::Unloaded)
   , mAsyncLoadedAudioFiles(0u)
{
   GERegisterPropertyEnum(AudioBankType, Type);
   GERegisterPropertyEnumReadonly(AudioBankState, State);

   GERegisterPropertyArray(AudioEventEntry);
}

AudioBank::~AudioBank()
{
   GEReleasePropertyArray(AudioEventEntry);
}

void AudioBank::load(ObjectManager<AudioEvent>& pAudioEventManager)
{
   if(mState != AudioBankState::Unloaded)
      return;

   // cache audio events
   for(uint32_t i = 0; i < getAudioEventEntryCount(); i++)
   {
      AudioEventEntry* audioEventEntry = getAudioEventEntry(i);
      AudioEvent* audioEvent = pAudioEventManager.get(audioEventEntry->getEventName());

      if(audioEvent)
      {
         std::pair<uint32_t, AudioEvent*> audioEventPair;
         audioEventPair.first = audioEventEntry->getEventName().getID();
         audioEventPair.second = audioEvent;
         mAudioEvents.insert(audioEventPair);
      }
   }

   // collect required audio files
   for(GESTLMap(uint32_t, AudioEvent*)::iterator it = mAudioEvents.begin(); it != mAudioEvents.end(); it++)
   {
      AudioEvent* audioEvent = it->second;

      for(uint32_t i = 0; i < audioEvent->getAudioFileCount(); i++)
      {
         const ObjectName& audioFileName = audioEvent->getAudioFile(i)->getFileName();
         bool alreadyAdded = false;

         for(size_t j = 0; j < mAudioFileNames.size(); j++)
         {
            if(mAudioFileNames[j] == audioFileName)
            {
               alreadyAdded = true;
               break;
            }
         }

         if(!alreadyAdded)
         {
            mAudioFileNames.push_back(audioFileName);
         }
      }
   }

   // the bank has been loaded
   mState = AudioBankState::Loaded;
}

void AudioBank::unload()
{
   if(mState != AudioBankState::Loaded)
      return;

   // clear data
   mAudioFileNames.clear();
   mAudioEvents.clear();

   // the bank has been unloaded
   mState = AudioBankState::Unloaded;
}

AudioEvent* AudioBank::getAudioEvent(const ObjectName& pAudioEventName)
{
   GESTLMap(uint32_t, AudioEvent*)::iterator it = mAudioEvents.find(pAudioEventName.getID());

   if(it != mAudioEvents.end())
      return it->second;

   return nullptr;
}

void AudioBank::resetAsyncLoadedAudioFiles()
{
   mAsyncLoadedAudioFiles = 0u;
}

void AudioBank::incrementAsyncLoadedAudioFiles()
{
   mAsyncLoadedAudioFiles++;
}

size_t AudioBank::getAsyncLoadedAudioFiles() const
{
   return mAsyncLoadedAudioFiles.load();
}
