
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

using namespace GE;
using namespace GE::Core;

static AAssetManager* NativeAssetManager = 0;

extern "C"
{
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_CreateAssetManager(JNIEnv* env, jclass clazz, jobject assetManager)
   {
      NativeAssetManager = AAssetManager_fromJava(env, assetManager);
   }
};

uint Device::getFileLength(const char* Filename)
{
    AAsset* aAsset = AAssetManager_open(NativeAssetManager, Filename, AASSET_MODE_UNKNOWN);
    
    if(aAsset)
    {
        int iReadBytes = AAsset_getLength(aAsset);
        AAsset_close(aAsset);
        return iReadBytes;
    }

    return 0;
}

uint Device::readFile(const char* Filename, unsigned char* ReadBuffer, uint BufferSize)
{
    AAsset* aAsset = AAssetManager_open(NativeAssetManager, Filename, AASSET_MODE_UNKNOWN);
    
    if(aAsset)
    {
        int iReadBytes = AAsset_read(aAsset, ReadBuffer, BufferSize);
        AAsset_close(aAsset);
        return iReadBytes;
    }

    return 0;
}

uint Device::getContentFilesCount(const char* SubDir, const char* Extension)
{
   AAssetDir* aAssetDir = AAssetManager_openDir(NativeAssetManager, SubDir);

   if(!aAssetDir)
      return 0;

   uint iExtensionLength = strlen(Extension);
   uint iFilesCount = 0;
   const char* sFileName = 0;

   do
   {
      sFileName = AAssetDir_getNextFileName(aAssetDir);

      if(sFileName)
      {
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
   while(sFileName);

   AAssetDir_close(aAssetDir);

   return iFilesCount;
}

bool Device::getContentFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   AAssetDir* aAssetDir = AAssetManager_openDir(NativeAssetManager, SubDir);

   if(!aAssetDir)
   {
      return false;
   }

   uint iExtensionLength = strlen(Extension);
   uint iFileIndex = 0;
   const char* sFileName = 0;

   do
   {
      sFileName = AAssetDir_getNextFileName(aAssetDir);

      if(sFileName)
      {
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
   while(sFileName);

   AAssetDir_close(aAssetDir);

   return true;
}
