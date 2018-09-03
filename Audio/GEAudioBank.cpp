
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio
//
//  --- GEAudioBank.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAudioBank.h"

#include "Content/GEAudioData.h"
#include "Core/GEDevice.h"

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

AudioBank::AudioBank(const ObjectName& pName, const ObjectName& pGroupName)
   : Resource(pName, pGroupName, TypeName)
   , mLoaded(false)
{
   GERegisterPropertyArray(AudioEventEntry);
}

AudioBank::~AudioBank()
{
   GEReleasePropertyArray(AudioEventEntry);
}

void AudioBank::load(ObjectManager<AudioEvent>& pAudioEventManager)
{
   if(mLoaded)
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
   GESTLVector(ObjectName) audioFileNames;

   for(GESTLMap(uint32_t, AudioEvent*)::iterator it = mAudioEvents.begin(); it != mAudioEvents.end(); it++)
   {
      AudioEvent* audioEvent = it->second;

      for(uint32_t i = 0; i < audioEvent->getAudioFileCount(); i++)
      {
         const ObjectName& audioFileName = audioEvent->getAudioFile(i)->getFileName();
         bool alreadyAdded = false;

         for(size_t j = 0; j < audioFileNames.size(); j++)
         {
            if(audioFileNames[j] == audioFileName)
            {
               alreadyAdded = true;
               break;
            }
         }

         if(!alreadyAdded)
         {
            audioFileNames.push_back(audioFileName);
         }
      }
   }

   // load audio files
   char subdir[256];
   sprintf(subdir, "Audio/%s", cName.getString());

   for(size_t i = 0; i < audioFileNames.size(); i++)
   {
      AudioData* audioData = Allocator::alloc<AudioData>();
      GEInvokeCtor(AudioData, audioData)();

      Device::readContentFile(ContentType::Audio, subdir, audioFileNames[i].getString(), "wav", audioData);

      std::pair<uint32_t, AudioData*> audioFilePair;
      audioFilePair.first = audioFileNames[i].getID();
      audioFilePair.second = audioData;
      mAudioFiles.insert(audioFilePair);
   }

   // the bank has been loaded
   mLoaded = true;
}

void AudioBank::unload()
{
   if(!mLoaded)
      return;

   // unload audio files
   for(GESTLMap(uint32_t, AudioData*)::iterator it = mAudioFiles.begin(); it != mAudioFiles.end(); it++)
   {
      AudioData* audioData = it->second;
      GEInvokeDtor(AudioData, audioData);
      Allocator::free(audioData);
   }

   // clear maps
   mAudioFiles.clear();
   mAudioEvents.clear();

   // the bank has been unloaded
   mLoaded = false;
}

AudioEvent* AudioBank::getAudioEvent(const ObjectName& pAudioEventName)
{
   GESTLMap(uint32_t, AudioEvent*)::iterator it = mAudioEvents.find(pAudioEventName.getID());

   if(it != mAudioEvents.end())
      return it->second;

   return 0;
}
