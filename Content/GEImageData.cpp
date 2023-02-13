
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
#include "PowerVR/PVRTTexture.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;

ImageData::ImageData()
   : mWidth(0)
   , mHeight(0)
   , mBytesPerPixel(0)
   , mFormat(Format::None)
{
}

void ImageData::loadDDS(uint Size, const char* Data)
{
   const uint32_t kFourCC_DXT1 = 0x31545844;
   const uint32_t kFourCC_DXT3 = 0x33545844;
   const uint32_t kFourCC_DXT5 = 0x35545844;

   const uint32_t kMagicSize = 4u;

   struct DDSHeader
   {
      uint32_t mSize;
      uint32_t mFlags;
      uint32_t mHeight;
      uint32_t mWidth;
      uint32_t mPitchOrLinearSize;
      uint32_t mDepth;
      uint32_t mMipMapCount;
      uint32_t mReserved1[11];
      uint32_t mPixelFormatSize;
      uint32_t mPixelFormatFlags;
      uint32_t mPixelFormatFourCC;
      uint32_t mPixelFormatRGBBitCount;
      uint32_t mPixelFormatRBitMask;
      uint32_t mPixelFormatGBitMask;
      uint32_t mPixelFormatBBitMask;
      uint32_t mPixelFormatABitMask;
      uint32_t mCaps[4];
      uint32_t mReserved2;
   };

   const DDSHeader* ddsHeader = reinterpret_cast<const DDSHeader*>(Data + kMagicSize);

   mWidth = (int)ddsHeader->mWidth;
   mHeight = (int)ddsHeader->mHeight;
   mBytesPerPixel = (int)ddsHeader->mPixelFormatSize / 8;

   if(ddsHeader->mPixelFormatFourCC == kFourCC_DXT1)
   {
      mFormat = Format::DDS_DXT1;
   }
   else if(ddsHeader->mPixelFormatFourCC == kFourCC_DXT3)
   {
      mFormat = Format::DDS_DXT3;
   }
   else if(ddsHeader->mPixelFormatFourCC == kFourCC_DXT5)
   {
      mFormat = Format::DDS_DXT5;
   }
   else
   {
      mFormat = Format::DDS_Uncompressed;
   }

   iDataSize = Size - kMagicSize - ddsHeader->mSize;
   pData = Allocator::alloc<char>(iDataSize);
   memcpy(pData, Data + kMagicSize + ddsHeader->mSize, iDataSize);
}

void ImageData::loadPVR(uint Size, const char* Data)
{
   const PVR_Texture_Header* pvrHeader = reinterpret_cast<const PVR_Texture_Header*>(Data);
   
   const uint32_t pixelType = pvrHeader->dwpfFlags & PVRTEX_PIXELTYPE;
   GEAssert(pixelType == OGL_PVRTC4);
   
   mWidth = (int)pvrHeader->dwWidth;
   mHeight = (int)pvrHeader->dwHeight;
   mBytesPerPixel = pvrHeader->dwAlphaBitMask != 0u ? 4 : 3;
   mFormat = Format::PVR;

   iDataSize = pvrHeader->dwTextureDataSize;
   pData = Allocator::alloc<char>(iDataSize);
   memcpy(pData, Data + sizeof(PVR_Texture_Header), iDataSize);
}

void ImageData::loadRaw(uint Size, const char* Data)
{
   mFormat = Format::Raw;
   pData = (char*)stbi_load_from_memory((const stbi_uc*)Data, Size, &mWidth, &mHeight, &mBytesPerPixel, 0);
}

void ImageData::load(unsigned int Size, const char* Data)
{
   unload();
   
   if(strncmp(Data, "DDS ", 4u) == 0)
   {
      loadDDS(Size, Data);
   }
   else if(Data[0] == '4')
   {
      loadPVR(Size, Data);
   }
   else
   {
      loadRaw(Size, Data);
   }
}

void ImageData::load(uint Size, std::istream& Stream)
{
   unload();
   
   char* sTempBuffer = Allocator::alloc<char>(Size);
   Stream.read(sTempBuffer, Size);

   load(Size, sTempBuffer);

   Allocator::free(sTempBuffer);
}

void ImageData::unload()
{
   if(pData)
   {
      if(mFormat != Format::Raw)
      {
         Allocator::free(pData);
      }
      else
      {
         stbi_image_free(pData);
      }

      pData = nullptr;
   }
}
