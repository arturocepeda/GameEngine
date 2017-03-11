
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEUtils.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEUtils.h"
#include "GEAllocator.h"
#include "Types/GETypes.h"

using namespace GE;
using namespace GE::Core;

//
//  Scaler
//
Scaler::Scaler(float x0, float x1, float y0, float y1)
{
   float dx = x1 - x0;
   float dy = y1 - y0;

   m = dy / dx;
   b = y0 - (m * x0);
}

Scaler::~Scaler()
{
}

float Scaler::y(float x)
{
   return (m * x) + b;
}

float Scaler::x(float y)
{
   return (y - b) / m;
}


//
//  XmlStringWriter
//
XmlStringWriter::XmlStringWriter()
   : sBuffer(0)
{
}

XmlStringWriter::~XmlStringWriter()
{
   releaseBuffer();
}

void XmlStringWriter::releaseBuffer()
{
   if(sBuffer)
      Allocator::free(sBuffer);
}

void XmlStringWriter::write(const void* data, size_t size)
{
   releaseBuffer();
   sBuffer = Allocator::alloc<char>((uint)size);
   memcpy(sBuffer, data, size);

   // set the end of the string
   char* pCursor = sBuffer + size - 1;

   while(*pCursor != '>')
      pCursor--;

   *(pCursor + 1) = '\0';
}

const char* XmlStringWriter::getData()
{
   return sBuffer;
}


//
//  FNV-1a Hash
//
unsigned int GE::Core::hash(const GESTLString& Str)
{
   if(Str.empty())
      return 0;

   const uint OffsetBasis = 2166136261;
   const uint FNVPrime = 16777619;

   uint iHash = OffsetBasis;

   for(uint i = 0; i < Str.size(); i++)
   {
      iHash ^= Str[i];
      iHash *= FNVPrime;
   }

   return iHash;
}
