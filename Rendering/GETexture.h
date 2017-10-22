
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
#include "Content/GEResource.h"
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


   class TextureAtlasEntry : public Core::Object
   {
   public:
      TextureCoordinates UV;

      TextureAtlasEntry(const Core::ObjectName& Name, const TextureCoordinates& TexCoord)
         : Core::Object(Name)
         , UV(TexCoord)
      {
      }
   };


   class Texture : public Content::Resource
   {
   private:
      uint iWidth;
      uint iHeight;

      void* pHandlePtr;

   public:
      GESTLVector(TextureAtlasEntry) AtlasUV;
      Core::ObjectManager<TextureAtlasEntry> AtlasUVManager;

      Texture(const Core::ObjectName& Name, const Core::ObjectName& GroupName, uint Width, uint Height);

      uint getWidth() const;
      uint getHeight() const;

      uint getAtlasSize() const;
      const Core::ObjectName& getAtlasName(uint Index) const;

      void setHandler(void* HandlePtr);
      const void* getHandler() const;

      void populateAtlasUVManager();
   };
}}
