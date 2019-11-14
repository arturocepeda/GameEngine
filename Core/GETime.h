
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

      float mDelta;
      float mTimeFactor;

   public:
      static const ObjectName TypeName;
      static const char* SubDir;
      static const char* Extension;

      Clock(const ObjectName& Name, const ObjectName& GroupName);
      ~Clock();

      float getDelta() const { return mDelta < MaxDelta ? mDelta : MaxDelta; }
      float getTimeFactor() const { return mTimeFactor; }

      void setDelta(float pDelta) { mDelta = pDelta * mTimeFactor; }
      void setTimeFactor(float pTimeFactor) { mTimeFactor = pTimeFactor; }
   };


   class Time
   {
   private:
      static const ObjectName DefaultClockName;

      static float mDelta;
      static float mElapsed;
      static ObjectManager<Clock>* mClocks;

   public:
      static void init();
      static void release();

      static float getDelta();
      static float getElapsed();

      static Clock* getDefaultClock();
      static Clock* getClock(const ObjectName& pClockName);

      static void reset();
      static void setDelta(float pDeltaTime);
   };
}}
