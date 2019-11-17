
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Log static class (iOS)
//
//  --- GELog.iOS.mm ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GELog.h"

#include <stdarg.h>

using namespace GE;
using namespace GE::Core;

LogListener* Log::CurrentLogListener = nullptr;

void Log::log(LogType Type, const char* Message, ...)
{
   char sBuffer[256];

   va_list vArguments;
   va_start(vArguments, Message);
   vsprintf(sBuffer, Message, vArguments);
   va_end(vArguments);

   NSLog(@"%s", sBuffer);
}
