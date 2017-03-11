
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GETexture.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEObject.h"
#include "GERenderingObjects.h"

namespace GE { namespace Rendering
{
   struct TextureCoordinates
   {
      float U0;
      float U1;
      float V0;
      float V1;
   };


   struct TextureAtlasEntry
   {
      Core::ObjectName Name;
      TextureCoordinates UV;
   };


   class Texture : public Core::Object
   {
   private:
      uint iWidth;
      uint iHeight;

      void* pHandlePtr;

   public:
      GESTLVector(TextureAtlasEntry) AtlasUV;

      Texture(const Core::ObjectName& Name, uint Width, uint Height);

      uint getWidth() const;
      uint getHeight() const;

      void setHandler(void* HandlePtr);
      const void* getHandler() const;
   };
}}
