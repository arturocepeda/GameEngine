
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
   private:
      int iWidth;
      int iHeight;
      int iBytesPerPixel;
    
   public:
      ImageData();

      virtual void load(uint Size, const char* Data) override;
      virtual void load(uint Size, std::istream& Stream) override;
      virtual void unload() override;
    
      int getWidth();
      int getHeight();
      int getBytesPerPixel();
   };
}}
