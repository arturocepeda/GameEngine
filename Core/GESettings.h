
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
      uint32_t mTargetFPS;
      bool mFullscreen;
      uint32_t mWindowSizeX;
      uint32_t mWindowSizeY;

   public:
      Settings();
      ~Settings();

      void load();

      GEDefaultSetter(uint32_t, TargetFPS, m)
      GEDefaultSetter(bool, Fullscreen, m)
      GEDefaultSetter(uint32_t, WindowSizeX, m)
      GEDefaultSetter(uint32_t, WindowSizeY, m)

      GEDefaultGetter(uint32_t, TargetFPS, m)
      GEDefaultGetter(bool, Fullscreen, m)
      GEDefaultGetter(uint32_t, WindowSizeX, m)
      GEDefaultGetter(uint32_t, WindowSizeY, m)
   };
}}
