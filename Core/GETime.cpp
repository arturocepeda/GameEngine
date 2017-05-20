
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GETime.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GETime.h"

using namespace GE;
using namespace GE::Core;


const float MaxDelta = 0.1f;


//
//  Clock
//
Clock::Clock()
   : fDelta(0.0f)
   , fTimeFactor(1.0f)
{
}

Clock::~Clock()
{
}

float Clock::getDelta() const
{
   return fDelta < MaxDelta ? fDelta : MaxDelta;
}

float Clock::getTimeFactor() const
{
   return fTimeFactor;
}

void Clock::setDelta(float Delta)
{
   fDelta = Delta * fTimeFactor;
}

void Clock::setTimeFactor(float TimeFactor)
{
   fTimeFactor = TimeFactor;
}


//
//  Time
//
float Time::fElapsed = 0.0f;
Clock Time::cClocks[Time::ClocksCount];

float Time::getElapsed()
{
   return fElapsed;
}

const Clock& Time::getClock(uint ClockIndex)
{
   GEAssert(ClockIndex < ClocksCount);
   return cClocks[ClockIndex];
}

void Time::reset()
{
   fElapsed = 0.0f;
}

void Time::setDelta(float DeltaTime)
{
   fElapsed += DeltaTime;

   for(uint i = 0; i < ClocksCount; i++)
      cClocks[i].setDelta(DeltaTime);
}
