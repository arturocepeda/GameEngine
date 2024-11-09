
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

   GESerializableEnum(AudioBankState)
   {
      Unloaded,
      Loading,
      Loaded,

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
   public:
      enum class State : uint8_t
      {
         Unloaded,
         Loading,
         Loaded
      };

   private:
      GESTLMap(uint32_t, AudioEvent*) mAudioEvents;
      GESTLVector(Core::ObjectName) mAudioFileNames;

      AudioBankType mType;
      std::atomic<AudioBankState> mState;
      std::atomic<size_t> mAsyncLoadedAudioFiles;

   public:
      static const Core::ObjectName TypeName;
      static const char* SubDir;
      static const char* Extension;

      AudioBank(const Core::ObjectName& pName, const Core::ObjectName& pGroupName);
      ~AudioBank();

      GEDefaultGetter(AudioBankType, Type, m)
      GEDefaultGetter(AudioBankState, State, m)
      GEDefaultGetter(const GESTLVector(Core::ObjectName)&, AudioFileNames, m)

      GEDefaultSetter(AudioBankType, Type, m)
      GEDefaultSetter(AudioBankState, State, m)

      GEPropertyArray(AudioEventEntry, AudioEventEntry)

      void load(Core::ObjectManager<AudioEvent>& pAudioEventManager);
      void unload();

      AudioEvent* getAudioEvent(const Core::ObjectName& pAudioEventName);

      void resetAsyncLoadedAudioFiles();
      void incrementAsyncLoadedAudioFiles();
      size_t getAsyncLoadedAudioFiles() const;
   };
}}
