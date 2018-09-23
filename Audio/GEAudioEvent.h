
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

      GEDefaultGetter(const Core::ObjectName&, FileName)
      GEDefaultSetter(const Core::ObjectName&, FileName)
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

   public:
      static const Core::ObjectName TypeName;

      AudioEvent(const Core::ObjectName& pName, const Core::ObjectName& pGroupName);
      ~AudioEvent();

      GEDefaultGetter(AudioEventPlayMode, PlayMode)
      GEDefaultGetter(float, FadeInTime)
      GEDefaultGetter(float, FadeOutTime)

      GEDefaultSetter(AudioEventPlayMode, PlayMode)
      GEDefaultSetter(float, FadeInTime)
      GEDefaultSetter(float, FadeOutTime)

      GEPropertyArray(AudioFileEntry, AudioFile)
   };
}}
