
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Log static class (Android)
//
//  --- GELog.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GELog.h"

#include <iostream>
#include <stdarg.h>

#include <android/log.h>

using namespace GE;
using namespace GE::Core;

LogListener* Log::CurrentLogListener = 0;

void Log::log(LogType Type, const char* Message, ...)
{
   char sBuffer[256];

   va_list vArguments;
   va_start(vArguments, Message);
   vsprintf(sBuffer, Message, vArguments);
   va_end(vArguments);

   android_LogPriority logPriority = ANDROID_LOG_DEFAULT;

   switch(Type)
   {
   case LogType::Info:
      logPriority = ANDROID_LOG_INFO;
      break;
   case LogType::Warning:
      logPriority = ANDROID_LOG_WARN;
      break;
   case LogType::Error:
      logPriority = ANDROID_LOG_ERROR;
      break;
   }

   __android_log_print(logPriority, "GameEngine", "%s", sBuffer);
}
