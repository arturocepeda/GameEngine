
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class (Android)
//
//  --- GEDevice.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Core/GEApplication.h"
#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"
#include "Types/GESTLTypes.h"

#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <android/log.h>

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

   unsigned int iFileLength = getFileLength(sFileName);

   GEAssert(iFileLength > 0);

   unsigned char* pFileData = new unsigned char[Type == ContentType::GenericTextData ? iFileLength + 1 : iFileLength];
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

   delete[] pFileData;
}

bool Device::userFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sFullPath[256];
   sprintf(sFullPath, "/sdcard/Android/data/%s/%s/%s.%s", Application::ID, SubDir, Name, Extension);

   FILE* file = fopen(sFullPath, "rb");
   
   if(file)
   {
      fclose(file);
      return true;
   }

   return false;
}

void Device::readUserFile(const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sFullPath[256];
   sprintf(sFullPath, "/sdcard/Android/data/%s/%s/%s.%s", Application::ID, SubDir, Name, Extension);

   FILE* file = fopen(sFullPath, "rb");
   GEAssert(file);

   fseek(file, 0L, SEEK_END);
   uint iFileSize = ftell(file);
   fseek(file, 0L, SEEK_SET);

   char* sBuffer = Allocator::alloc<char>(iFileSize);
   fread(sBuffer, 1, iFileSize, file);
   ContentData->load(iFileSize, sBuffer);

   Allocator::free(sBuffer);
   fclose(file);
}

void Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension, const ContentData* ContentData)
{
   char sAppDirectory[256];
   sprintf(sAppDirectory, "/sdcard/Android/data/%s/", Application::ID);
   mkdir(sAppDirectory, 0770);

   char sAppDataDirectory[256];
   sprintf(sAppDataDirectory, "%s%s/", sAppDirectory, SubDir);
   mkdir(sAppDataDirectory, 0770);

   char sFullPath[256];
   sprintf(sFullPath, "%s%s.%s", sAppDataDirectory, Name, Extension);

   FILE* file = fopen(sFullPath, "w+");
   GEAssert(file);

   fwrite(ContentData->getData(), 1, ContentData->getDataSize(), file);
   fflush(file);
   fclose(file);
}

void Device::deleteUserFile(const char* SubDir, const char* Name, const char* Extension)
{
   char sFullPath[256];
   sprintf(sFullPath, "/sdcard/Android/data/%s/%s/%s.%s", Application::ID, SubDir, Name, Extension);

   remove(sFullPath);
}

uint Device::getUserFilesCount(const char* SubDir, const char* Extension)
{
   char sAppDataDirectory[256];
   sprintf(sAppDataDirectory, "/sdcard/Android/data/%s/%s", Application::ID, SubDir);

   DIR* sDir = opendir(sAppDataDirectory);

   if(!sDir)
      return 0;   

   uint iExtensionLength = strlen(Extension);
   uint iFilesCount = 0;
   struct dirent* sDirEnt = 0;

   do
   {
      sDirEnt = readdir(sDir);

      if(sDirEnt)
      {
         const char* sFileName = sDirEnt->d_name;
         uint iFileNameLength = strlen(sFileName);

         if(iFileNameLength < iExtensionLength)
            continue;

         bool bHasExtension = true;
         uint iCharIndex = iFileNameLength - iExtensionLength;

         for(uint i = 0; i < iExtensionLength; i++, iCharIndex++)
         {
            if(sFileName[iCharIndex] != Extension[i])
            {
               bHasExtension = false;
               break;
            }
         }

         if(bHasExtension)
            iFilesCount++;
      }
   }
   while(sDirEnt);

   closedir(sDir);

   return iFilesCount;
}

bool Device::getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   char sAppDataDirectory[256];
   sprintf(sAppDataDirectory, "/sdcard/Android/data/%s/%s", Application::ID, SubDir);

   DIR* sDir = opendir(sAppDataDirectory);

   if(!sDir)
   {
      return false;
   }

   uint iExtensionLength = strlen(Extension);
   uint iFileIndex = 0;
   struct dirent* sDirEnt = 0;

   do
   {
      sDirEnt = readdir(sDir);

      if(sDirEnt)
      {
         const char* sFileName = sDirEnt->d_name;
         uint iFileNameLength = strlen(sFileName);

         if(iFileNameLength < iExtensionLength)
            continue;

         bool bHasExtension = true;
         uint iCharIndex = iFileNameLength - iExtensionLength;

         for(uint i = 0; i < iExtensionLength; i++, iCharIndex++)
         {
            if(sFileName[iCharIndex] != Extension[i])
            {
               bHasExtension = false;
               break;
            }
         }

         if(bHasExtension)
         {
            if(iFileIndex == Index)
            {
               uint iReturnStringLength = iFileNameLength - iExtensionLength - 1;
               strncpy(Name, sFileName, iReturnStringLength);
               Name[iReturnStringLength] = '\0';
               break;
            }

            iFileIndex++;
         }
      }
   }
   while(sDirEnt);

   closedir(sDir);

   return true;
}
