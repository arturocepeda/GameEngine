
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEImageData.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEContentData.h"

namespace GE { namespace Content
{
   class ImageData : public ContentData
   {
   public:
      enum class Format : int
      {
         None,
         Raw,
         DDS_Uncompressed,
         DDS_DXT1,
         DDS_DXT3,
         DDS_DXT5,
         PVR,
         ASTC
      };

   private:
      int mWidth;
      int mHeight;
      int mBytesPerPixel;
      Format mFormat;

      void loadDDS(uint Size, const char* Data);
      void loadPVR(uint Size, const char* Data);
      void loadKTX(uint Size, const char* Data);
      void loadRaw(uint Size, const char* Data);
    
   public:
      ImageData();

      virtual void load(uint Size, const char* Data) override;
      virtual void load(uint Size, std::istream& Stream) override;
      virtual void unload() override;
    
      int getWidth() const { return mWidth; }
      int getHeight() const { return mHeight; }
      int getBytesPerPixel() const { return mBytesPerPixel; }
      Format getFormat() const { return mFormat; }
   };
}}
