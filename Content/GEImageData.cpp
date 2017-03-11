
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEImageData.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEImageData.h"
#include "Types/GETypes.h"
#include "Core/GEAllocator.h"
#include "stblib/stb_image.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;

ImageData::ImageData()
   : iWidth(0)
   , iHeight(0)
   , iBytesPerPixel(0)
{
}

void ImageData::load(unsigned int Size, const char* Data)
{
   unload();
   pData = (char*)stbi_load_from_memory((const stbi_uc*)Data, Size, &iWidth, &iHeight, &iBytesPerPixel, 0);
}

void ImageData::load(uint Size, std::istream& Stream)
{
   unload();
   
   char* sTempBuffer = Allocator::alloc<char>(Size);
   Stream.read(sTempBuffer, Size);

   pData = (char*)stbi_load_from_memory((const stbi_uc*)sTempBuffer, Size, &iWidth, &iHeight, &iBytesPerPixel, 0);

   Allocator::free(sTempBuffer);
}

void ImageData::unload()
{
   if(pData)
   {
      stbi_image_free(pData);
      pData = nullptr;
   }
}

int ImageData::getWidth()
{
   return iWidth;
}

int ImageData::getHeight()
{
   return iHeight;
}

int ImageData::getBytesPerPixel()
{
   return iBytesPerPixel;
}
