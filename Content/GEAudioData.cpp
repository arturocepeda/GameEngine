
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

#include "Core/GEPlatform.h"
#include "Core/GEAllocator.h"

using namespace GE::Content;
using namespace GE::Core;


struct AudioDataStream
{
   const char* Data;
   size_t Size;
   size_t Cursor;

   AudioDataStream(const char* pData, size_t pSize)
      : Data(pData)
      , Size(pSize)
      , Cursor(0)
   {
   }
};


#if defined (GE_PLATFORM_DESKTOP) || defined (GE_PLATFORM_ANDROID)
# define GE_VORBIS_SUPPORT
#endif


#if defined (GE_VORBIS_SUPPORT)

# if defined (GE_PLATFORM_DESKTOP)
#  include "Externals/libogg/include/os_types.h"
#  include "Externals/libvorbis/include/vorbisfile.h"
# elif defined (GE_PLATFORM_ANDROID)
#  include "Externals/Tremor/ivorbisfile.h"
# endif

namespace GE { namespace internal
{
   //
   //  Ogg Vorbis functions
   //
   size_t ovRead(void* pDestination, size_t pSize, size_t pNumMembers, void* pDataSource)
   {
      AudioDataStream* stream = static_cast<AudioDataStream*>(pDataSource);

      // calculate number of bytes to read
      const size_t iBytesToRead = GEMin(pSize * pNumMembers, stream->Size - stream->Cursor);

      // read the data
      memcpy(pDestination, stream->Data + stream->Cursor, iBytesToRead);
      stream->Cursor += iBytesToRead;

      return iBytesToRead;
   }

   int ovSeek(void* pDataSource, ogg_int64_t pOffset, int pWhence)
   {
      AudioDataStream* stream = static_cast<AudioDataStream*>(pDataSource);

      switch(pWhence)
      {
      case SEEK_SET:
         stream->Cursor = GEMin((unsigned int)pOffset, stream->Size);
         break;

      case SEEK_CUR:
         stream->Cursor = GEMin(stream->Cursor + (unsigned int)pOffset, stream->Size);
         break;

      case SEEK_END:
         stream->Cursor = stream->Size;
         break;

      default:
         return -1;
      }

      return 0;
   }

   long ovTell(void* pDataSource)
   {
      AudioDataStream* stream = static_cast<AudioDataStream*>(pDataSource);
      return (long)stream->Cursor;
   }

   int ovClose(void* pDataSource)
   {
      (void)pDataSource;
      return 0;
   }
}}
#endif


//
//  AudioData
//
AudioData::AudioData()
   : iSampleRate(0)
   , iBitDepth(0)
   , iNumberOfChannels(0)
{
}

void AudioData::loadWAVData(uint32_t Size, const char* Data)
{
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

   pData = Allocator::alloc<char>(iDataSize, AllocationCategory::Audio);
   memcpy(pData, pDataPointer, iDataSize);
}

void AudioData::loadOggData(uint32_t Size, const char* Data)
{
#if defined (GE_VORBIS_SUPPORT)
   AudioDataStream stream = AudioDataStream(Data, Size);
   OggVorbis_File ovFile;
   ov_callbacks ovCallbacks;

   // set our functions to handle Vorbis OGG data
   ovCallbacks.read_func = GE::internal::ovRead;
   ovCallbacks.seek_func = GE::internal::ovSeek;
   ovCallbacks.tell_func = GE::internal::ovTell;
   ovCallbacks.close_func = GE::internal::ovClose;

   // attach audio file data with the ovFile struct
   ov_open_callbacks(&stream, &ovFile, 0, 0, ovCallbacks);

   // check format and frequency
   vorbis_info* pVorbisInfo = ov_info(&ovFile, -1);
   iNumberOfChannels = pVorbisInfo->channels;
   iSampleRate = pVorbisInfo->rate;
   iBitDepth = 16;

   // read file data
   const size_t BufferSize = 4096;
   char sBuffer[BufferSize];

   const uint iDecodedSamplesCount = (uint)ov_pcm_total(&ovFile, -1);
   const uint iBytesPerDecodedSample = (uint)iBitDepth / 8;

   iDataSize = iDecodedSamplesCount * iBytesPerDecodedSample * (uint)iNumberOfChannels;
   pData = Allocator::alloc<char>(iDataSize, AllocationCategory::Audio);

   int iBitStream = 0;
   long iReadedBytes = 0;
   long iTotalReadedBytes = 0;

   do
   {
#if defined (GE_PLATFORM_DESKTOP)
      iReadedBytes = ov_read(&ovFile, sBuffer, BufferSize, 0, 2, 1, &iBitStream);
#else
      iReadedBytes = ov_read(&ovFile, sBuffer, BufferSize, &iBitStream);
#endif
      memcpy(pData + iTotalReadedBytes, sBuffer, iReadedBytes);
      iTotalReadedBytes += iReadedBytes;
   }
   while(iReadedBytes > 0);

   ov_clear(&ovFile);
#endif
}

void AudioData::loadCustom(uint32_t Size, const char* Data)
{
   iSampleRate = 0;
   iBitDepth = 0;
   iNumberOfChannels = 0;

   ContentData::load(Size, Data);
}

void AudioData::load(GE::uint Size, const char* Data)
{
   unload();

   // WAV
   if(strncmp(Data, "RIFF", 4) == 0)
   {
      loadWAVData(Size, Data);
   }
   // Ogg Vorbis
   else if(strncmp(Data, "OggS", 4) == 0)
   {
      loadOggData(Size, Data);
   }
   // Custom format
   else
   {
      loadCustom(Size, Data);
   }
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
