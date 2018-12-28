
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


namespace GE { namespace Scripting
{
   class Environment;
}}


namespace GE { namespace Core
{
   enum class ApplicationContentType
   {
      Xml,
      Bin
   };


   enum class ApplicationRenderingAPI
   {
      DirectX,
      OpenGL
   };


   class Application
   {
   public:
      static const uint32_t ScriptingEnvironmentsCount = 4u;

   private:
      static Scripting::Environment* smScriptingEnvironments[ScriptingEnvironmentsCount];

      static void registerComponentFactories();

   public:
      static const char* Name;
      static const char* ID;

      static const char* VersionString;
      static uint VersionNumber;

      static const char* ExecutablePath;
      static GESTLVector(const char*) Arguments;

      static const ApplicationRenderingAPI RenderingAPI;

      static ApplicationContentType ContentType;

      static void startUp(void (*pInitAppModuleFunction)());
      static void shutDown();

      static Scripting::Environment* getScriptingEnvironment(uint32_t pIndex);
   };
}}
