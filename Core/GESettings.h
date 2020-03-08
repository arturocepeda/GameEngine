
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GESettings.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GESingleton.h"
#include "GESerializable.h"

namespace GE { namespace Core
{
   class Settings : public Singleton<Settings>, public Serializable
   {
   private:
      // General
      uint32_t mTargetFPS;
      char mLanguage[32];

      // Video
      bool mFullscreen;
      bool mVSync;
      uint32_t mWindowSizeX;
      uint32_t mWindowSizeY;

      // Development
      bool mErrorPopUps;

   public:
      Settings();
      ~Settings();

      void load();
      void save();

      // General
      GEDefaultSetter(uint32_t, TargetFPS, m)
      GEDefaultGetter(uint32_t, TargetFPS, m)

      GEDefaultSetterString(const char*, Language, m)
      GEDefaultGetter(const char*, Language, m)

      // Video
      GEDefaultSetter(bool, Fullscreen, m)
      GEDefaultGetter(bool, Fullscreen, m)

      GEDefaultSetter(bool, VSync, m)
      GEDefaultGetter(bool, VSync, m)

      GEDefaultSetter(uint32_t, WindowSizeX, m)
      GEDefaultGetter(uint32_t, WindowSizeX, m)

      GEDefaultSetter(uint32_t, WindowSizeY, m)      
      GEDefaultGetter(uint32_t, WindowSizeY, m)

      // Development
      GEDefaultSetter(bool, ErrorPopUps, m)
      GEDefaultGetter(bool, ErrorPopUps, m)
   };
}}
