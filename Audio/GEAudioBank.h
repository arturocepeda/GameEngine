
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio
//
//  --- GEAudioBank.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEAudioEvent.h"

#include "Types/GETypes.h"
#include "Content/GEResource.h"

namespace GE { namespace Content
{
   class AudioData;
}}

namespace GE { namespace Audio
{
   class AudioEventEntry : public Core::SerializableArrayElement
   {
   private:
      Core::ObjectName mEventName;

   public:
      AudioEventEntry();
      ~AudioEventEntry();

      GEDefaultGetter(const Core::ObjectName&, EventName)
      GEDefaultSetter(const Core::ObjectName&, EventName)
   };


   class AudioBank : public Content::Resource
   {
   private:
      GESTLMap(uint32_t, AudioEvent*) mAudioEvents;
      GESTLMap(uint32_t, Content::AudioData*) mAudioFiles;

      bool mLoaded;

   public:
      static const Core::ObjectName TypeName;

      AudioBank(const Core::ObjectName& pName, const Core::ObjectName& pGroupName);
      ~AudioBank();

      GEPropertyArray(AudioEventEntry, AudioEventEntry)

      void load(Core::ObjectManager<AudioEvent>& pAudioEventManager);
      void unload();

      GEDefaultGetter(bool, Loaded)
      GEDefaultGetter(const GESTLMap(uint32_t, Content::AudioData*)&, AudioFiles)

      AudioEvent* getAudioEvent(const Core::ObjectName& pAudioEventName);
   };
}}
