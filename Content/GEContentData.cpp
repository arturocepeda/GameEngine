
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEContentData.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEContentData.h"
#include "Core/GEAllocator.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;


//
//  ContentData
//
ContentData::ContentData()
   : iDataSize(0)
   , pData(nullptr)
{
}

ContentData::~ContentData()
{
   unload();
}

void ContentData::load(uint Size, const char* Data)
{
   unload();

   iDataSize = Size;
   pData = Allocator::alloc<char>(Size);
   memcpy(pData, Data, Size);
}

void ContentData::load(uint Size, std::istream& Stream)
{
   unload();

   iDataSize = Size;
   pData = Allocator::alloc<char>(Size);
   Stream.read(pData, iDataSize);
}

void ContentData::unload()
{
   if(pData)
   {
      Allocator::free(pData);
      pData = nullptr;
      iDataSize = 0u;
   }
}

unsigned int ContentData::getDataSize() const
{
   return iDataSize;
}

char* ContentData::getData() const
{
   return pData;
}


//
//  MemoryBuffer
//
MemoryBuffer::MemoryBuffer(char* Begin, char* End)
{
   setg(Begin, Begin, End);
}


//
//  ContentDataMemoryBuffer
//
ContentDataMemoryBuffer::ContentDataMemoryBuffer(const ContentData& cContentData)
   : MemoryBuffer(cContentData.getData(), cContentData.getData() + cContentData.getDataSize())
{
}
