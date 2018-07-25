
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
#include "Content/GEResource.h"

namespace GE { namespace Core
{
   class Clock : public Content::Resource
   {
   private:
      static const float MaxDelta;

      float fDelta;
      float fTimeFactor;

   public:
      static const ObjectName TypeName;

      Clock(const ObjectName& Name, const ObjectName& GroupName);
      ~Clock();

      float getDelta() const { return fDelta < MaxDelta ? fDelta : MaxDelta; }
      float getTimeFactor() const { return fTimeFactor; }

      void setDelta(float Delta) { fDelta = Delta * fTimeFactor; }
      void setTimeFactor(float TimeFactor) { fTimeFactor = TimeFactor; }
   };


   class Time
   {
   private:
      static const ObjectName DefaultClockName;

      static float fElapsed;
      static ObjectManager<Clock>* mClocks;

   public:
      static void init();
      static void release();

      static float getElapsed();

      static Clock* getDefaultClock();
      static Clock* getClock(const ObjectName& pClockName);

      static void reset();
      static void setDelta(float DeltaTime);
   };
}}
