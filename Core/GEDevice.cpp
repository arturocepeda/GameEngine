
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class
//
//  --- GEDevice.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDevice.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;

GEMutex Device::mIOMutex;
Device::IOBuffersMap* Device::mIOBuffers = 0;

int Device::ScreenWidth = 0;
int Device::ScreenHeight = 0;

SystemLanguage Device::Language = SystemLanguage::English;
DeviceOrientation Device::Orientation = DeviceOrientation::Portrait;

int Device::AudioSystemSampleRate = 0;
int Device::AudioSystemFramesPerBuffer = 0;

void Device::init()
{
   GEMutexInit(mIOMutex);
   mIOBuffers = Allocator::alloc<IOBuffersMap>();
   GEInvokeCtor(IOBuffersMap, mIOBuffers);
}

void Device::release()
{
   GEInvokeDtor(IOBuffersMap, mIOBuffers);
   Allocator::free(mIOBuffers);
   mIOBuffers = 0;
   GEMutexDestroy(mIOMutex);
}

Device::IOBuffer* Device::requestIOBuffer(uint iSize)
{
   GEMutexLock(mIOMutex);

   IOBuffer* pBuffer = 0;
   uint iThreadID = GEThreadID;
   IOBuffersMap::iterator it = mIOBuffers->find(iThreadID);

   if(it != mIOBuffers->end())
   {
      pBuffer = &it->second;
   }
   else
   {
      (*mIOBuffers)[iThreadID] = IOBuffer();
      pBuffer = &mIOBuffers->find(iThreadID)->second;
   }

   GEMutexUnlock(mIOMutex);

   if(pBuffer->capacity() < iSize)
   {
      pBuffer->resize(iSize);
   }

   return pBuffer;
}

int Device::getScreenWidth()
{
   return ScreenWidth;
}

int Device::getScreenHeight()
{
   return ScreenHeight;
}

float Device::getAspectRatio()
{
   return (float)ScreenHeight / ScreenWidth;
}
