
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
      GESTLVector(Core::ObjectName) mAudioFileNames;

      bool mLoaded;

   public:
      static const Core::ObjectName TypeName;

      AudioBank(const Core::ObjectName& pName, const Core::ObjectName& pGroupName);
      ~AudioBank();

      GEPropertyArray(AudioEventEntry, AudioEventEntry)

      void load(Core::ObjectManager<AudioEvent>& pAudioEventManager);
      void unload();

      GEDefaultGetter(bool, Loaded)
      GEDefaultGetter(const GESTLVector(Core::ObjectName)&, AudioFileNames)

      AudioEvent* getAudioEvent(const Core::ObjectName& pAudioEventName);
   };
}}
