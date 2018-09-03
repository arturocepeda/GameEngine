
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Audio
//
//  --- GEAudioEvent.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAudioEvent.h"

#include "Core/GEAllocator.h"

using namespace GE;
using namespace GE::Audio;
using namespace GE::Core;


//
//  AudioFileEntry
//
const ObjectName AudioFileEntryName = ObjectName("AudioFile");

AudioFileEntry::AudioFileEntry()
   : SerializableArrayElement(AudioFileEntryName)
{
   GERegisterProperty(ObjectName, FileName);
}

AudioFileEntry::~AudioFileEntry()
{
}


//
//  AudioEvent
//
const ObjectName AudioEvent::TypeName = ObjectName("AudioEvent");

AudioEvent::AudioEvent(const ObjectName& pName, const ObjectName& pGroupName)
   : Resource(pName, pGroupName, TypeName)
   , mPlayMode(AudioEventPlayMode::OneShot)
{
   GERegisterPropertyEnum(AudioEventPlayMode, PlayMode);

   GERegisterPropertyArray(AudioFile);
}

AudioEvent::~AudioEvent()
{
   GEReleasePropertyArray(AudioFile);
}
