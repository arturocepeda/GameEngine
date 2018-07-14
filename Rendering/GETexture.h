
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


   GESerializableEnum(TextureSettingsBitMask)
   {
      AtlasUV  = 1 << 0,

      Count = 1
   };


   GESerializableEnum(TextureWrapMode)
   {
      Clamp,
      Repeat,

      Count
   };


   class Texture : public Content::SerializableResource
   {
   private:
      uint iWidth;
      uint iHeight;

      char sFormat[8];

      uint8_t eSettings;
      TextureWrapMode eWrapMode;

      void* pHandlePtr;

   public:
      GESTLVector(TextureAtlasEntry) AtlasUV;
      Core::ObjectManager<TextureAtlasEntry> AtlasUVManager;

      Texture(const Core::ObjectName& Name, const Core::ObjectName& GroupName);

      void setWidth(uint Value) { iWidth = Value; }
      void setHeight(uint Value) { iHeight = Value; }
      void setFormat(const char* Value) { strcpy(sFormat, Value); }
      void setSettings(uint8_t Value) { eSettings = Value; }
      void setWrapMode(TextureWrapMode Value) { eWrapMode = Value; }

      uint getWidth() const { return iWidth; }
      uint getHeight() const { return iHeight; }
      const char* getFormat() const { return sFormat; }
      uint8_t getSettings() const { return eSettings; }
      TextureWrapMode getWrapMode() const { return eWrapMode; }

      uint getAtlasSize() const;
      const Core::ObjectName& getAtlasName(uint Index) const;

      void setHandler(void* HandlePtr) { pHandlePtr = HandlePtr; }
      const void* getHandler() const { return pHandlePtr; }

      void populateAtlasUVManager();
   };
}}
