
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEApplication.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GESTLTypes.h"

namespace GE { namespace Core
{
   enum class ApplicationContentType
   {
      Xml,
      Bin
   };


   class Application
   {
   private:
      static void registerComponentFactories();

   public:
      static const char* Name;
      static const char* ID;

      static const char* VersionString;
      static uint VersionNumber;

      static const char* ExecutablePath;
      static GESTLVector(const char*) Arguments;

      static ApplicationContentType ContentType;

      static void startUp();
      static void shutDown();
   };
}}
