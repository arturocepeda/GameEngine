
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

#include <android/asset_manager_jni.h>
#include "Core/GEDevice.h"
#include "Core/GEUtils.h"

using namespace GE;
using namespace GE::Core;


static const size_t kSubDirBufferSize = 64u;
static const size_t kExtensionBufferSize = 32u;


static AAssetManager* NativeAssetManager = nullptr;


extern "C"
{
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_CreateAssetManager(JNIEnv* env, jclass clazz, jobject assetManager)
   {
      NativeAssetManager = AAssetManager_fromJava(env, assetManager);
   }
};


uint Device::getFileLength(const char* pFilename)
{
    AAsset* aAsset = AAssetManager_open(NativeAssetManager, pFilename, AASSET_MODE_UNKNOWN);
    
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
    AAsset* aAsset = AAssetManager_open(NativeAssetManager, pFilename, AASSET_MODE_UNKNOWN);
    
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

   AAssetDir* aAssetDir = AAssetManager_openDir(NativeAssetManager, subDir);

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

   AAssetDir* aAssetDir = AAssetManager_openDir(NativeAssetManager, subDir);

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
