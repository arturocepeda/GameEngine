
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class (macOS)
//
//  --- GEDevice.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Core/GEApplication.h"
#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"

#include <iostream>
#include <stdarg.h>
#include <fstream>
#include <dirent.h>

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;

int Device::getTouchPadWidth()
{
   return ScreenWidth;
}

int Device::getTouchPadHeight()
{
   return ScreenHeight;
}

GESTLString getFullPath(const char* Filename)
{
   const int BufferSize = 256;
   char sBuffer[BufferSize];

   getcwd(sBuffer, BufferSize);
   sprintf(sBuffer, "%s/%s", sBuffer, Filename);

   return GESTLString(sBuffer);
}

bool hasExtension(const char* sFilename, const char* sExtension)
{
   size_t iFilenameLength = strlen(sFilename);
   size_t iExtensionLength = strlen(sExtension);
   
   const char* sExtInFilename = strstr(sFilename, sExtension);
   
   return sExtInFilename == (sFilename + iFilenameLength - iExtensionLength);
}

uint Device::getContentFilesCount(const char* SubDir, const char* Extension)
{
   DIR* dp = opendir(SubDir);
   
   if(!dp)
   {
      return 0;
   }
   
   dirent* dirp = 0;
   uint iFilesCount = 0;
   
   while((dirp = readdir(dp)))
   {
      if(hasExtension(dirp->d_name, Extension))
         iFilesCount++;
   }
   
   closedir(dp);
   
   return iFilesCount;
}

bool Device::getContentFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   DIR* dp = opendir(SubDir);
   
   if(!dp)
   {
      return false;
   }
   
   dirent* dirp = 0;
   uint iFileIndex = 0;
   const size_t iExtensionLength = strlen(Extension);
   
   while((dirp = readdir(dp)))
   {
      if(hasExtension(dirp->d_name, Extension))
      {
         if(iFileIndex == Index)
         {
            strcpy(Name, dirp->d_name);
            const size_t iFullNameLength = strlen(Name);
            Name[iFullNameLength - iExtensionLength - 1u] = '\0';
            break;
         }
         
         iFileIndex++;
      }
   }
   
   closedir(dp);
   
   return true;
}

bool Device::contentFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sFileName[256];
   sprintf(sFileName, "%s/%s.%s", SubDir, Name, Extension);

   return getFileLength(sFileName) > 0;
}

void Device::readContentFile(ContentType Type, const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sFileName[256];
   sprintf(sFileName, "%s/%s.%s", SubDir, Name, Extension);

   uint iFileLength = getFileLength(sFileName);

   GEAssert(iFileLength > 0);

   byte* pFileData =
      Allocator::alloc<byte>(Type == ContentType::GenericTextData ? iFileLength + 1 : iFileLength);
   readFile(sFileName, pFileData, iFileLength);

   if(Type == ContentType::GenericTextData)
      pFileData[iFileLength] = '\0';

   switch(Type)
   {
   case ContentType::Texture:
   case ContentType::FontTexture:
      static_cast<ImageData*>(ContentData)->load(iFileLength, (const char*)pFileData);
      break;
   case ContentType::Audio:
      static_cast<AudioData*>(ContentData)->load(iFileLength, (const char*)pFileData);
      break;
   default:
      ContentData->load(Type == ContentType::GenericTextData ? iFileLength + 1 : iFileLength, (const char*)pFileData);
   }

   Allocator::free(pFileData);
}

bool Device::userFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   return false;
}

void Device::readUserFile(const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
}

void Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension, const ContentData* ContentData)
{
}

uint Device::getUserFilesCount(const char* SubDir, const char* Extension)
{
   return 0;
}

bool Device::getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   return false;
}

void Device::deleteUserFile(const char* SubDir, const char* Name, const char* Extension)
{
   
}

uint Device::getFileLength(const char* Filename)
{
   GESTLString sFullPath = getFullPath(Filename);
   std::ifstream file(sFullPath.c_str(), std::ios::in | std::ios::binary);
      
   if(file.is_open())
   {
      file.seekg(0, std::ios::end);
      std::streamsize size = file.tellg();
      file.close();
      return (uint)size;
   }

   return 0;
}

uint Device::readFile(const char* Filename, GE::byte* ReadBuffer, uint BufferSize)
{
   GESTLString sFullPath = getFullPath(Filename);
   std::ifstream file(sFullPath.c_str(), std::ios::in | std::ios::binary);
      
   if(file.is_open())
   {
      file.seekg(0, std::ios::end);
      std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);

      file.read((char*)ReadBuffer, size);
      file.close();
      return (uint)size;
   }

   return 0;
}

int Device::getNumberOfCPUCores()
{
   return 4;
}