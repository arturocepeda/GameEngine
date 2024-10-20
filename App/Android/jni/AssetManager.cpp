
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Android
//
//  --- AssetManager.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDevice.h"
#include "Core/GEApplication.h"
#include "Core/GEUtils.h"
#include "Content/GEContentData.h"

#include <fstream>
#include <dirent.h>
#include <sys/stat.h>

#include <android/asset_manager_jni.h>

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;


static const size_t kSubDirBufferSize = 64u;
static const size_t kExtensionBufferSize = 32u;


static AAssetManager* gNativeAssetManager = nullptr;
static std::string gInternalStoragePath;


extern "C"
{
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_CreateAssetManager(JNIEnv* pEnv, jclass pClass, jobject pAssetManager)
   {
      (void)pClass;
      gNativeAssetManager = AAssetManager_fromJava(pEnv, pAssetManager);
   }

   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_SendInternalStoragePath(JNIEnv* pEnv, jclass pClass, jstring pInternalStoragePath)
   {
      gInternalStoragePath = pEnv->GetStringUTFChars(pInternalStoragePath, nullptr);
   }
};


uint Device::getFileLength(const char* pFilename)
{
    AAsset* aAsset = AAssetManager_open(gNativeAssetManager, pFilename, AASSET_MODE_UNKNOWN);
    
    if(aAsset)
    {
        int iReadBytes = AAsset_getLength(aAsset);
        AAsset_close(aAsset);
        return iReadBytes;
    }

    return 0;
}

uint Device::readFile(const char* pFilename, unsigned char* pReadBuffer, uint pBufferSize)
{
    AAsset* aAsset = AAssetManager_open(gNativeAssetManager, pFilename, AASSET_MODE_UNKNOWN);
    
    if(aAsset)
    {
        int iReadBytes = AAsset_read(aAsset, pReadBuffer, pBufferSize);
        AAsset_close(aAsset);
        return iReadBytes;
    }

    return 0;
}

uint Device::getContentFilesCount(const char* pSubDir, const char* pExtension)
{
   char subDir[kSubDirBufferSize];
   strcpy(subDir, pSubDir);

   if(Device::ContentHashPath)
   {
      toHashPath(subDir);
   }

   AAssetDir* aAssetDir = AAssetManager_openDir(gNativeAssetManager, subDir);

   if(!aAssetDir)
   {
      return 0;
   }

   char extension[kExtensionBufferSize];
   strcpy(extension, pExtension);

   if(Device::ContentHashPath)
   {
      toHashPath(extension);
   }

   uint iExtensionLength = strlen(extension);
   uint iFilesCount = 0u;
   const char* sFileName = nullptr;

   do
   {
      sFileName = AAssetDir_getNextFileName(aAssetDir);

      if(sFileName)
      {
         uint iFileNameLength = strlen(sFileName);

         if(iFileNameLength < iExtensionLength)
         {
            continue;
         }

         bool bHasExtension = true;
         uint iCharIndex = iFileNameLength - iExtensionLength;

         for(uint i = 0u; i < iExtensionLength; i++, iCharIndex++)
         {
            if(sFileName[iCharIndex] != extension[i])
            {
               bHasExtension = false;
               break;
            }
         }

         if(bHasExtension)
         {
            iFilesCount++;
         }
      }
   }
   while(sFileName);

   AAssetDir_close(aAssetDir);

   return iFilesCount;
}

bool Device::getContentFileName(const char* pSubDir, const char* pExtension, uint pIndex, char* pName)
{
   char subDir[kSubDirBufferSize];
   strcpy(subDir, pSubDir);

   if(Device::ContentHashPath)
   {
      toHashPath(subDir);
   }

   AAssetDir* aAssetDir = AAssetManager_openDir(gNativeAssetManager, subDir);

   if(!aAssetDir)
   {
      return false;
   }

   char extension[kExtensionBufferSize];
   strcpy(extension, pExtension);

   if(Device::ContentHashPath)
   {
      toHashPath(extension);
   }

   uint iExtensionLength = strlen(extension);
   uint iFileIndex = 0u;
   const char* sFileName = nullptr;

   do
   {
      sFileName = AAssetDir_getNextFileName(aAssetDir);

      if(sFileName)
      {
         uint iFileNameLength = strlen(sFileName);

         if(iFileNameLength < iExtensionLength)
         {
            continue;
         }

         bool bHasExtension = true;
         uint iCharIndex = iFileNameLength - iExtensionLength;

         for(uint i = 0u; i < iExtensionLength; i++, iCharIndex++)
         {
            if(sFileName[iCharIndex] != extension[i])
            {
               bHasExtension = false;
               break;
            }
         }

         if(bHasExtension)
         {
            if(iFileIndex == pIndex)
            {
               uint iReturnStringLength = iFileNameLength - iExtensionLength - 1;
               strncpy(pName, sFileName, iReturnStringLength);
               pName[iReturnStringLength] = '\0';
               break;
            }

            iFileIndex++;
         }
      }
   }
   while(sFileName);

   AAssetDir_close(aAssetDir);

   return true;
}

bool Device::userFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sFullPath[256];
   sprintf(sFullPath, "%s/%s/%s.%s", gInternalStoragePath.c_str(), SubDir, Name, Extension);

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
   sprintf(sFullPath, "%s/%s/%s.%s", gInternalStoragePath.c_str(), SubDir, Name, Extension);

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
   mkdir(gInternalStoragePath.c_str(), 0770);

   char sAppDataDirectory[256];
   sprintf(sAppDataDirectory, "%s/%s", gInternalStoragePath.c_str(), SubDir);
   mkdir(sAppDataDirectory, 0770);

   char sFullPath[256];
   sprintf(sFullPath, "%s/%s.%s", sAppDataDirectory, Name, Extension);

   FILE* file = fopen(sFullPath, "w+");
   GEAssert(file);

   fwrite(ContentData->getData(), 1, ContentData->getDataSize(), file);
   fflush(file);
   fclose(file);
}

std::ofstream Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension)
{
   return std::ofstream();
}

void Device::deleteUserFile(const char* SubDir, const char* Name, const char* Extension)
{
   char sFullPath[256];
   sprintf(sFullPath, "%s/%s/%s.%s", gInternalStoragePath.c_str(), SubDir, Name, Extension);

   remove(sFullPath);
}

uint Device::getUserFilesCount(const char* SubDir, const char* Extension)
{
   char sAppDataDirectory[256];
   sprintf(sAppDataDirectory, "%s/%s", gInternalStoragePath.c_str(), SubDir);

   DIR* sDir = opendir(sAppDataDirectory);

   if(!sDir)
   {
      return 0;
   }

   uint iExtensionLength = strlen(Extension);
   uint iFilesCount = 0;
   struct dirent* sDirEnt = nullptr;

   do
   {
      sDirEnt = readdir(sDir);

      if(sDirEnt)
      {
         const char* sFileName = sDirEnt->d_name;
         uint iFileNameLength = strlen(sFileName);

         if(iFileNameLength < iExtensionLength)
         {
            continue;
         }

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
            iFilesCount++;
         }
      }
   }
   while(sDirEnt);

   closedir(sDir);

   return iFilesCount;
}

bool Device::getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   char sAppDataDirectory[256];
   sprintf(sAppDataDirectory, "%s/%s", gInternalStoragePath.c_str(), SubDir);

   DIR* sDir = opendir(sAppDataDirectory);

   if(!sDir)
   {
      return false;
   }

   uint iExtensionLength = strlen(Extension);
   uint iFileIndex = 0;
   struct dirent* sDirEnt = nullptr;

   do
   {
      sDirEnt = readdir(sDir);

      if(sDirEnt)
      {
         const char* sFileName = sDirEnt->d_name;
         uint iFileNameLength = strlen(sFileName);

         if(iFileNameLength < iExtensionLength)
         {
            continue;
         }

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
