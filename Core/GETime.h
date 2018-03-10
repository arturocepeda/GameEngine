
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GETime.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Core/GESerializable.h"

namespace GE { namespace Core
{
   class Clock : public Serializable
   {
   private:
      static uint iClocksCounter;

      float fDelta;
      float fTimeFactor;

   public:
      Clock();
      ~Clock();

      float getDelta() const;
      float getTimeFactor() const;

      void setDelta(float Delta);
      void setTimeFactor(float TimeFactor);
   };


   class Time
   {
   public:
      static const uint ClocksCount = 2;

   private:
      static float fElapsed;
      static Clock cClocks[ClocksCount];

   public:
      static float getElapsed();
      static const Clock& getClock(uint ClockIndex);

      static void reset();
      static void setDelta(float DeltaTime);
   };
}}
