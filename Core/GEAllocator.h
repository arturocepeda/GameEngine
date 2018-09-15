
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEAllocator.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Core/GEThreads.h"

#include <cstdlib>
#include <climits>

#if defined (GE_DEVELOPMENT)
# include "Core/GELog.h"
# include <map>
# include <typeinfo>
#endif

#define GEInvokeCtor(ClassName, Ptr) new (Ptr) ClassName
#define GEInvokeDtor(ClassName, Ptr) Ptr->~ClassName();

namespace GE { namespace Core
{
   GESerializableEnum(AllocationCategory)
   {
      General,
      STL,
      Audio,
      Scripting,

      Count
   };


   struct AllocationInfo
   {
      static const uint DescriptionSize = 512 - sizeof(AllocationCategory) - sizeof(uint);

      char Description[DescriptionSize];
      AllocationCategory Category;
      uint Size;

      AllocationInfo();
      AllocationInfo(const char* sDesc, AllocationCategory eCategory, uint iSize);
   };


   class Allocator
   {
   private:
#if defined (GE_DEVELOPMENT)
      static std::map<void*, AllocationInfo> mAllocationsRegistry;
      static uint iTotalBytesAllocated[(int)AllocationCategory::Count];
      static bool bLoggingEnabled[(int)AllocationCategory::Count];
      static GEMutex pMutex;
      static bool bInitialized;
#endif

   public:
      static void init();
      static void release();

      template<typename T>
      static T* alloc(uint ElementsCount = 1, AllocationCategory Category = AllocationCategory::General)
      {
         (void)Category;
         uint iSize = sizeof(T) * ElementsCount;
         T* pPtr = (T*)::realloc(0, iSize);
         GEAssert(pPtr);

#if defined (GE_DEVELOPMENT)
         if(bInitialized)
         {
            char sBuffer[AllocationInfo::DescriptionSize];

            if(ElementsCount > 1)
               sprintf(sBuffer, "%s (%u)", typeid(T).name(), ElementsCount);
            else
               sprintf(sBuffer, "%s", typeid(T).name());

            GEMutexLock(pMutex);

            mAllocationsRegistry[pPtr] = AllocationInfo(sBuffer, Category, iSize);;
            iTotalBytesAllocated[(int)Category] += iSize;

            if(bLoggingEnabled[(int)Category])
            {
               Log::log(LogType::Info, "Heap Allocation [%s]: %s --- %u bytes (total: %u bytes)",
                  strAllocationCategory[(int)Category],
                  sBuffer,
                  iSize,
                  iTotalBytesAllocated[(int)Category]);
            }

            GEMutexUnlock(pMutex);
         }
#endif
         return pPtr;
      }

      template<typename T>
      static T* realloc(void* Ptr, uint ElementsCount = 1, AllocationCategory Category = AllocationCategory::General)
      {
         uint iSize = sizeof(T) * ElementsCount;
         T* pPtr = (T*)::realloc(Ptr, iSize);
         GEAssert(pPtr);

#if defined (GE_DEVELOPMENT)
         if(bInitialized)
         {
            char sBuffer[AllocationInfo::DescriptionSize];

            if(ElementsCount > 1)
               sprintf(sBuffer, "%s (%u)", typeid(T).name(), ElementsCount);
            else
               sprintf(sBuffer, "%s", typeid(T).name());

            GEMutexLock(pMutex);

            if(Ptr)
            {
               std::map<void*, AllocationInfo>::const_iterator it = mAllocationsRegistry.find(Ptr);
               GEAssert(it != mAllocationsRegistry.end());
               iTotalBytesAllocated[(int)Category] -= it->second.Size;
               mAllocationsRegistry.erase(it);
            }

            mAllocationsRegistry[pPtr] = AllocationInfo(sBuffer, Category, iSize);;
            iTotalBytesAllocated[(int)Category] += iSize;

            GEMutexUnlock(pMutex);
         }
#endif
         return pPtr;
      }

      static void free(void* Ptr);

#if defined (GE_DEVELOPMENT)
      static uint getTotalBytesAllocated(AllocationCategory eCategory)
      {
         return iTotalBytesAllocated[(int)eCategory];
      }
#endif
   };


   template<typename T>
   class STLAllocator
   {
   public:
      typedef T value_type;
      typedef T* pointer;
      typedef const T* const_pointer;
      typedef T& reference;
      typedef const T& const_reference;
      typedef std::size_t size_type;
      typedef std::ptrdiff_t difference_type;

      template<typename U>
      struct rebind { typedef STLAllocator<U> other; };

      pointer address(reference value) const { return &value; }
      const_pointer address(const_reference value) const { return &value; }

      STLAllocator() {}
      STLAllocator(const STLAllocator&) {}
      template<typename U>
      STLAllocator(const STLAllocator<U>&) {}
      ~STLAllocator() {}

      size_type max_size() const { return SIZE_MAX / sizeof(T); }

      pointer allocate(size_type num, const void* = 0)
      {
         return Allocator::alloc<T>((uint)num, AllocationCategory::STL);
      }
      void construct(pointer p, const T& value)
      {
         GEInvokeCtor(T, p)(value);
      }
      void destroy(pointer p)
      {
         (void)p;
         GEInvokeDtor(T, p);
      }
      void deallocate(pointer p, size_type)
      {
         Allocator::free(p);
      }
   };

   template<typename T1, typename T2>
   bool operator==(const STLAllocator<T1>&, const STLAllocator<T2>&) { return true; }
   template<typename T1, typename T2>
   bool operator!=(const STLAllocator<T1>&, const STLAllocator<T2>&) { return false; }
}}
