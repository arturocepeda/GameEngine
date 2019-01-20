
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
   GESerializableEnum(AudioBankType)
   {
      Buffered,
      Streamed,

      Count
   };


   class AudioEventEntry : public Core::SerializableArrayElement
   {
   private:
      Core::ObjectName mEventName;

   public:
      AudioEventEntry();
      ~AudioEventEntry();

      GEDefaultGetter(const Core::ObjectName&, EventName, m)
      GEDefaultSetter(const Core::ObjectName&, EventName, m)
   };


   class AudioBank : public Content::Resource
   {
   private:
      GESTLMap(uint32_t, AudioEvent*) mAudioEvents;
      GESTLVector(Core::ObjectName) mAudioFileNames;

      AudioBankType mType;
      bool mLoaded;

   public:
      static const Core::ObjectName TypeName;
      static const char* SubDir;
      static const char* Extension;

      AudioBank(const Core::ObjectName& pName, const Core::ObjectName& pGroupName);
      ~AudioBank();

      GEDefaultGetter(AudioBankType, Type, m)

      GEDefaultSetter(AudioBankType, Type, m)

      GEPropertyArray(AudioEventEntry, AudioEventEntry)

      void load(Core::ObjectManager<AudioEvent>& pAudioEventManager);
      void unload();

      GEDefaultGetter(bool, Loaded, m)
      GEDefaultGetter(const GESTLVector(Core::ObjectName)&, AudioFileNames, m)

      AudioEvent* getAudioEvent(const Core::ObjectName& pAudioEventName);
   };
}}
