
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEContentData.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypeDefinitions.h"
#include <iostream>

namespace GE { namespace Content
{
   enum class ContentType
   {
      GenericTextData,
      GenericBinaryData,
      Texture,
      TextureAtlasInfo,
      Audio,
      Shader,
      FontTexture,
      FontData
   };


   class ContentData
   {
   protected:
      uint iDataSize;
      char* pData;
    
   public:
      ContentData();
      virtual ~ContentData();

      virtual void load(uint Size, const char* Data);
      virtual void load(uint Size, std::istream& Stream);
      virtual void unload();

      unsigned int getDataSize() const;
      char* getData() const;
   };


   struct MemoryBuffer : std::streambuf
   {
      MemoryBuffer(char* Begin, char* End);
   };


   struct ContentDataMemoryBuffer : MemoryBuffer
   {
      ContentDataMemoryBuffer(const ContentData& cContentData);
   };
}}
