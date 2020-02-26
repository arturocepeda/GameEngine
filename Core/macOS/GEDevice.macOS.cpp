
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
#include "Core/GEUtils.h"
#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"

#include <iostream>
#include <stdarg.h>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;

const uint32_t kMaxPath = 256u;

int Device::getTouchPadWidth()
{
   return ScreenWidth;
}

int Device::getTouchPadHeight()
{
   return ScreenHeight;
}

static GESTLString getFullPath(const char* Filename)
{
   char sBuffer[kMaxPath];
   getcwd(sBuffer, kMaxPath);
   sprintf(sBuffer, "%s/%s", sBuffer, Filename);

   return GESTLString(sBuffer);
}

static bool hasExtension(const char* sFilename, const char* sExtension)
{
   size_t iFilenameLength = strlen(sFilename);
   size_t iExtensionLength = strlen(sExtension);
   
   const char* sExtInFilename = strstr(sFilename, sExtension);
   
   return sExtInFilename == (sFilename + iFilenameLength - iExtensionLength);
}

static const char* getUserName()
{
   return getenv("USER");
}

uint Device::getContentFilesCount(const char* pSubDir, const char* pExtension)
{
   char subDir[kMaxPath];
   strcpy(subDir, pSubDir);
   
   if(ContentHashPath)
   {
      toHashPath(subDir);
   }
   
   DIR* dp = opendir(subDir);
   
   if(!dp)
   {
      return 0u;
   }
   
   char extension[32];
   strcpy(extension, pExtension);
   
   if(ContentHashPath)
   {
      toHashPath(extension);
   }
   
   dirent* dirp = nullptr;
   uint iFilesCount = 0u;
   
   while((dirp = readdir(dp)))
   {
      if(hasExtension(dirp->d_name, extension))
      {
         iFilesCount++;
      }
   }
   
   closedir(dp);
   
   return iFilesCount;
}

bool Device::getContentFileName(const char* pSubDir, const char* pExtension, uint pIndex, char* pName)
{
   char subDir[kMaxPath];
   strcpy(subDir, pSubDir);
   
   if(ContentHashPath)
   {
      toHashPath(subDir);
   }
   
   DIR* dp = opendir(subDir);
   
   if(!dp)
   {
      return false;
   }
   
   char extension[32];
   strcpy(extension, pExtension);
   
   if(ContentHashPath)
   {
      toHashPath(extension);
   }
   
   dirent* dirp = nullptr;
   uint iFileIndex = 0u;
   const size_t iExtensionLength = strlen(extension);
   
   while((dirp = readdir(dp)))
   {
      if(hasExtension(dirp->d_name, extension))
      {
         if(iFileIndex == pIndex)
         {
            strcpy(pName, dirp->d_name);
            const size_t iFullNameLength = strlen(pName);
            pName[iFullNameLength - iExtensionLength - 1u] = '\0';
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
   char sFileName[kMaxPath];
   sprintf(sFileName, "%s/%s.%s", SubDir, Name, Extension);

   if(ContentHashPath)
   {
      toHashPath(sFileName);
   }
   
   return getFileLength(sFileName) > 0;
}

void Device::readContentFile(ContentType Type, const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sSubDir[kMaxPath];
   strcpy(sSubDir, SubDir);
   
   char sFileName[kMaxPath];
   sprintf(sFileName, "%s/%s.%s", SubDir, Name, Extension);

   if(ContentHashPath)
   {
      if(isHash(Name))
      {
         toHashPath(sSubDir);
         
         char extension[32];
         strcpy(extension, Extension);
         toHashPath(extension);
         
         sprintf(sFileName, "%s/%s.%s", sSubDir, Name, extension);
      }
      else
      {
         sprintf(sFileName, "%s/%s.%s", sSubDir, Name, Extension);
         toHashPath(sFileName);
      }
   }
   else
   {
      sprintf(sFileName, "%s/%s.%s", sSubDir, Name, Extension);
   }
   
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
   char sFileName[kMaxPath];
   sprintf(sFileName, "Users/%s/%s/%s.%s", getUserName(), SubDir, Name, Extension);
   
   return getFileLength(sFileName) > 0;
}

void Device::readUserFile(const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sFileName[kMaxPath];
   sprintf(sFileName, "Users/%s/%s/%s.%s", getUserName(), SubDir, Name, Extension);
   
   uint iFileLength = getFileLength(sFileName);
   GEAssert(iFileLength > 0);
   
   byte* pFileData = Allocator::alloc<byte>(iFileLength);
   readFile(sFileName, pFileData, iFileLength);
   ContentData->load(iFileLength, (const char*)pFileData);
   Allocator::free(pFileData);
}

void Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension, const ContentData* ContentData)
{
   static const mode_t mkdirMode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
   
   DIR* dp = opendir("Users");
   
   if(!dp)
   {
      mkdir("Users", mkdirMode);
   }
   
   char sSubDir[kMaxPath];
   sprintf(sSubDir, "Users/%s", getUserName());
   
   dp = opendir(sSubDir);
   
   if(!dp)
   {
      mkdir(sSubDir, mkdirMode);
   }
   
   sprintf(sSubDir, "Users/%s/%s", getUserName(), SubDir);
   
   dp = opendir(sSubDir);
   
   if(!dp)
   {
      mkdir(sSubDir, mkdirMode);
   }
   
   char sFileName[kMaxPath];
   sprintf(sFileName, "Users/%s/%s/%s.%s", getUserName(), SubDir, Name, Extension);
   GESTLString sFullPath = getFullPath(sFileName);
   
   std::ofstream file(sFullPath.c_str(), std::ios::out | std::ios::binary);
   GEAssert(file.is_open());
   
   file.write(ContentData->getData(), ContentData->getDataSize());
   file.close();
}

uint Device::getUserFilesCount(const char* SubDir, const char* Extension)
{
   char sSubDir[kMaxPath];
   sprintf(sSubDir, "Users/%s/%s", getUserName(), SubDir);
   
   DIR* dp = opendir(sSubDir);
   
   if(!dp)
   {
      return 0u;
   }
   
   dirent* dirp = nullptr;
   uint iFilesCount = 0u;
   
   while((dirp = readdir(dp)))
   {
      if(hasExtension(dirp->d_name, Extension))
      {
         iFilesCount++;
      }
   }
   
   closedir(dp);
   
   return iFilesCount;
}

bool Device::getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   char sSubDir[kMaxPath];
   sprintf(sSubDir, "Users/%s/%s", getUserName(), SubDir);
   
   DIR* dp = opendir(sSubDir);
   
   if(!dp)
   {
      return false;
   }
   
   dirent* dirp = nullptr;
   uint iFileIndex = 0u;
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

void Device::deleteUserFile(const char* SubDir, const char* Name, const char* Extension)
{
   char sFileName[kMaxPath];
   sprintf(sFileName, "Users/%s/%s/%s.%s", getUserName(), SubDir, Name, Extension);
   GESTLString sFullPath = getFullPath(sFileName);
   remove(sFullPath.c_str());
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
   return (int)sysconf(_SC_NPROCESSORS_ONLN);
}
