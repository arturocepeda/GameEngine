
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEAllocator.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAllocator.h"

using namespace GE;
using namespace GE::Core;

//
//  AllocationInfo
//
AllocationInfo::AllocationInfo()
   : Category(AllocationCategory::General)
   , Size(0)
{
}

AllocationInfo::AllocationInfo(const char* sDesc, AllocationCategory eCategory, uint iSize)
   : Category(eCategory)
   , Size(iSize)
{
   strcpy(Description, sDesc);
}


//
//  Allocator
//
#if defined (GE_DEVELOPMENT)
std::map<void*, AllocationInfo> Allocator::mAllocationsRegistry;
uint Allocator::iTotalBytesAllocated[(int)AllocationCategory::Count];
bool Allocator::bLoggingEnabled[(int)AllocationCategory::Count];
GEMutex Allocator::pMutex;
bool Allocator::bInitialized = false;
#endif

void Allocator::init()
{
#if defined (GE_DEVELOPMENT)
   memset(&iTotalBytesAllocated[0], 0, sizeof(uint) * (int)AllocationCategory::Count);
   memset(&bLoggingEnabled[0], 0, sizeof(bool) * (int)AllocationCategory::Count);

   GEMutexInit(pMutex);
   bInitialized = true;
#endif
}

void Allocator::release()
{
#if defined (GE_DEVELOPMENT)
   bInitialized = false;
   GEMutexDestroy(pMutex);
#endif
}

void Allocator::free(void* Ptr)
{
   GEAssert(Ptr);

#if defined (GE_DEVELOPMENT)
   if(bInitialized)
   {
      GEMutexLock(pMutex);

      std::map<void*, AllocationInfo>::const_iterator it = mAllocationsRegistry.find(Ptr);

      if(it != mAllocationsRegistry.end())
      {
         const AllocationInfo& sAllocationInfo = it->second;
         iTotalBytesAllocated[(int)sAllocationInfo.Category] -= sAllocationInfo.Size;

         if(bLoggingEnabled[(int)sAllocationInfo.Category])
         {
            Device::log("Heap Release [%s]: %s --- %u bytes (total: %u bytes)",
               strAllocationCategory[(int)sAllocationInfo.Category],
               sAllocationInfo.Description,
               sAllocationInfo.Size,
               iTotalBytesAllocated[(int)sAllocationInfo.Category]);
         }

         mAllocationsRegistry.erase(it);
      }

      GEMutexUnlock(pMutex);
   }
#endif

   ::free(Ptr);
}
