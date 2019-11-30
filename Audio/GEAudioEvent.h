
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio
//
//  --- GEAudioEvent.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Content/GEResource.h"

namespace GE { namespace Audio
{
   class AudioFileEntry : public Core::SerializableArrayElement
   {
   private:
      Core::ObjectName mFileName;

   public:
      AudioFileEntry();
      ~AudioFileEntry();

      GEDefaultGetter(const Core::ObjectName&, FileName, m)
      GEDefaultSetter(const Core::ObjectName&, FileName, m)
   };


   GESerializableEnum(AudioEventPlayMode)
   {
      OneShot,
      Loop,

      Count
   };


   class AudioEvent : public Content::Resource
   {
   private:
      AudioEventPlayMode mPlayMode;
      float mFadeInTime;
      float mFadeOutTime;
      Vector2 mPitchRange;

   public:
      static const Core::ObjectName TypeName;
      static const char* SubDir;
      static const char* Extension;

      AudioEvent(const Core::ObjectName& pName, const Core::ObjectName& pGroupName);
      ~AudioEvent();

      GEDefaultGetter(AudioEventPlayMode, PlayMode, m)
      GEDefaultGetter(float, FadeInTime, m)
      GEDefaultGetter(float, FadeOutTime, m)
      GEDefaultGetter(const Vector2&, PitchRange, m)

      GEDefaultSetter(AudioEventPlayMode, PlayMode, m)
      GEDefaultSetter(float, FadeInTime, m)
      GEDefaultSetter(float, FadeOutTime, m)
      GEDefaultSetter(const Vector2&, PitchRange, m)

      GEPropertyArray(AudioFileEntry, AudioFile)
   };
}}
