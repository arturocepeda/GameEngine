
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
