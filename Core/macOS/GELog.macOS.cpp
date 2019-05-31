
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Log static class (macOS)
//
//  --- GELog.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GELog.h"

#include <iostream>
#include <stdarg.h>

using namespace GE;
using namespace GE::Core;

LogListener* Log::CurrentLogListener = 0;

void Log::log(LogType Type, const char* Message, ...)
{
   char sBuffer[1024];

   va_list vArguments;
   va_start(vArguments, Message);
   vsprintf(sBuffer, Message, vArguments);
   va_end(vArguments);

   std::cout << sBuffer << std::endl;

   if(CurrentLogListener)
   {
      CurrentLogListener->onLog(Type, sBuffer);
   }
}
