
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
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
const char* AudioEvent::SubDir = "Audio";
const char* AudioEvent::Extension = "events";

AudioEvent::AudioEvent(const ObjectName& pName, const ObjectName& pGroupName)
   : Resource(pName, pGroupName, TypeName)
   , mPlayMode(AudioEventPlayMode::OneShot)
   , mFadeInTime(0.0f)
   , mFadeOutTime(0.0f)
   , mPitchRange(1.0f, 1.0f)
{
   GERegisterPropertyEnum(AudioEventPlayMode, PlayMode);
   GERegisterProperty(Float, FadeInTime);
   GERegisterProperty(Float, FadeOutTime);
   GERegisterProperty(Vector2, PitchRange);

   GERegisterPropertyArray(AudioFile);
}

AudioEvent::~AudioEvent()
{
   GEReleasePropertyArray(AudioFile);
}
