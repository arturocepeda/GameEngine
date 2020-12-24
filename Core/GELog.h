
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

#include <vector>

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
      virtual void onLog(LogType pType, const char* pMessage) = 0;
   };


   class Log
   {
   private:
      static std::vector<LogListener*> smListeners;

   public:
      static void addListener(LogListener* pListener);
      static void log(LogType pType, const char* pMessage, ...);
   };
}}
