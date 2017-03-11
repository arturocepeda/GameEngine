
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GETimer.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GETimer.h"

using namespace GE::Core;

Timer::Timer()
   : bRunning(true)
   , dStartTime(0.0)
   , dEndTime(0.0)
{
#ifdef WIN32
   QueryPerformanceFrequency(&iFrequency);
   iStartCount.QuadPart = 0;
   iEndCount.QuadPart = 0;
#else
   iStartCount.tv_sec = 0;
   iStartCount.tv_usec = 0;
   iEndCount.tv_sec = 0;
   iEndCount.tv_usec = 0;
#endif
}

Timer::~Timer()
{
}

void Timer::start()
{
   bRunning = true;

#ifdef WIN32
   QueryPerformanceCounter(&iStartCount);
#else
   gettimeofday(&iStartCount, NULL);
#endif
}

void Timer::stop()
{
   bRunning = false;

#ifdef WIN32
   QueryPerformanceCounter(&iEndCount);
#else
   gettimeofday(&iEndCount, NULL);
#endif
}

double Timer::getTime()
{
#ifdef WIN32
   if(bRunning)
      QueryPerformanceCounter(&iEndCount);

   dStartTime = iStartCount.QuadPart * (1000000.0 / iFrequency.QuadPart);
   dEndTime = iEndCount.QuadPart * (1000000.0 / iFrequency.QuadPart);
#else
   if(bRunning)
      gettimeofday(&iEndCount, NULL);

   dStartTime = (iStartCount.tv_sec * 1000000.0) + iStartCount.tv_usec;
   dEndTime = (iEndCount.tv_sec * 1000000.0) + iEndCount.tv_usec;
#endif

   return (dEndTime - dStartTime);
}
