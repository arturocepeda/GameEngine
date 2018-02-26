
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Log static class
//
//  --- GELog.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"

namespace GE { namespace Core
{
   enum class LogType
   {
      Info,
      Warning,
      Error
   };


   class LogListener
   {
   public:
      virtual void onLog(LogType, const char*) {}
   };


   class Log
   {
   public:
      static LogListener* CurrentLogListener;

      static void log(LogType Type, const char* Message, ...);
   };
}}
