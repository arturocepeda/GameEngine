
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GETimer.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#if defined (WIN32)
# if !defined (_WINSOCKAPI_)
#  define _WINSOCKAPI_
# endif
# include <windows.h>
#else
# include <sys/time.h>
#endif

namespace GE { namespace Core
{
   class Timer
   {
   private:
      double dStartTime;
      double dEndTime;
      bool bRunning;

   #ifdef WIN32
      LARGE_INTEGER iFrequency;
      LARGE_INTEGER iStartCount;
      LARGE_INTEGER iEndCount;
   #else
      timeval iStartCount;
      timeval iEndCount;
   #endif

   public:
      Timer();
      ~Timer();

      void start();
      void stop();
      double getTime();    // microseconds
   };
}}
