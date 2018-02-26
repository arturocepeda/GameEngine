
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Log static class (Windows)
//
//  --- GELog.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GELog.h"

#include <windows.h>
#include <Shlobj.h>
#include <iostream>
#include <stdarg.h>

using namespace GE;
using namespace GE::Core;

LogListener* Log::CurrentLogListener = 0;

void Log::log(LogType Type, const char* Message, ...)
{
#if defined(GE_EDITOR_SUPPORT)
  char sBuffer[1024];

  va_list vArguments;
  va_start(vArguments, Message);
  vsprintf(sBuffer, Message, vArguments);
  va_end(vArguments);

  OutputDebugString(sBuffer);
  OutputDebugString("\n");

  if(CurrentLogListener)
  {
     CurrentLogListener->onLog(Type, sBuffer);
  }
#endif
}
