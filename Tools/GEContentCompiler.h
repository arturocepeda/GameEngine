
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Tools
//
//  --- GEContentCompiler.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEApplication.h"

namespace GE { namespace Tools
{
   class ContentCompiler
   {
   private:
      static void packShaders(Core::ApplicationRenderingAPI pRenderingAPI);
      static void packTextures();
      static void packTextureFile(const char* pXmlFileName);
      static void packMaterials();
      static void packFonts();
      static void packFontFile(const char* pXmlFileName);
      static void packStrings();
      static void packMeshes();
      static void packSkeletons();
      static void packAnimations();
      static void packSounds();
      static void packPrefabs();
      static void packScenes();

      static void compileScripts();

   public:
      static void compileContent();
   };
}}
