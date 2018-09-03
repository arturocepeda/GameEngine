
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEAudioData.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAudioData.h"
#include "Core/GEAllocator.h"
#include <iostream>

using namespace GE::Content;
using namespace GE::Core;

AudioData::AudioData()
   : iSampleRate(0)
   , iBitDepth(0)
   , iNumberOfChannels(0)
{
}

void AudioData::load(unsigned int Size, const char* Data)
{
   unload();

   const int BufferSize = 8;
   char sBuffer[BufferSize];
   char* pDataPointer = (char*)Data;

   // "RIFF"
   strncpy(sBuffer, pDataPointer, 4);
   sBuffer[4] = '\0';
   pDataPointer += 4;

   if(strcmp(sBuffer, "RIFF") != 0)
      return;

   // "RIFF" chunk size
   pDataPointer += 4;

   // "WAVE"
   strncpy(sBuffer, pDataPointer, 4);
   sBuffer[4] = '\0';
   pDataPointer += 4;

   if(strcmp(sBuffer, "WAVE") != 0)
      return;

   // "fmt "
   strncpy(sBuffer, pDataPointer, 4);
   sBuffer[4] = '\0';
   pDataPointer += 4;

   if(strcmp(sBuffer, "fmt ") != 0)
      return;

   // "fmt " chunk size
   pDataPointer += 4;

   // audio format (2 bytes)
   pDataPointer += 2;

   // channels (2 bytes)
   iNumberOfChannels = *(short*)pDataPointer;
   pDataPointer += 2;

   // sample rate (4 bytes)
   iSampleRate = *(int*)pDataPointer;
   pDataPointer += 4;

   // byte rate (4 bytes)
   pDataPointer += 4;

   // block align (2 bytes)
   pDataPointer += 2;

   // bits per sample (2 bytes)
   iBitDepth = *(short*)pDataPointer;
   pDataPointer += 2;

   // "data" 
   // (there may be some bytes as extension for non PCM formats before)
   do
   {
      strncpy(sBuffer, pDataPointer, 4);
      sBuffer[4] = '\0';
      pDataPointer += 2;
   }
   while(strcmp(sBuffer, "data") != 0);

   pDataPointer += 2;

   // filePointer chunk size
   iDataSize = *(int*)pDataPointer;
   pDataPointer += 4;

   pData = Allocator::alloc<char>(iDataSize);
   memcpy(pData, pDataPointer, iDataSize);
}

int AudioData::getSampleRate()
{
   return iSampleRate;
}

short AudioData::getBitDepth()
{
   return iBitDepth;
}

short AudioData::getNumberOfChannels()
{
   return iNumberOfChannels;
}
