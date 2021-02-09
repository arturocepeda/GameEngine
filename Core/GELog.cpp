
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Log static class
//
//  --- GELog.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GELog.h"

#include <stdarg.h>
#include <stdio.h>

using namespace GE;
using namespace GE::Core;

std::vector<LogListener*> Log::smListeners;

void Log::addListener(LogListener* pListener)
{
   smListeners.push_back(pListener);
}

void Log::log(LogType pType, const char* pMessage, ...)
{
   char buffer[1024];

   va_list args;
   va_start(args, pMessage);
   vsprintf(buffer, pMessage, args);
   va_end(args);

   for(size_t i = 0u; i < smListeners.size(); i++)
   {
      smListeners[i]->onLog(pType, buffer);
   }
}
