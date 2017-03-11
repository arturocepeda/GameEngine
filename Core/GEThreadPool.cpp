
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEThreadPool.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "GEThreadPool.h"
#include "GEAllocator.h"
#include "GEProfiler.h"

using namespace GE;
using namespace GE::Core;


//
//  ThreadPool
//
ThreadPool::ThreadPool(uint WorkersCount)
{
   iWorkersCount = WorkersCount;
   sWorkers = Allocator::alloc<Worker>(iWorkersCount);

   GEMutexInit(mThreadPoolMutex);

   for(uint i = 0; i < iWorkersCount; i++)
   {
      sWorkers[i] = Worker(this, i);

      GEMutexInit(sWorkers[i].Mutex);
      GEMutexLock(sWorkers[i].Mutex);
   }
}

ThreadPool::~ThreadPool()
{
   for(uint i = 0; i < iWorkersCount; i++)
   {
      GEThreadClose(sWorkers[i].Thread);
      GEMutexDestroy(sWorkers[i].Mutex);
   }

   Allocator::free(sWorkers);

   GEMutexDestroy(mThreadPoolMutex);
}

void ThreadPool::queueJob(const JobDesc& Desc)
{
   GEAssert(Desc.Task);

   GEMutexLock(mThreadPoolMutex);
   vJobsQueue.push(Job(Desc));
   GEMutexUnlock(mThreadPoolMutex);
}


//
//  ThreadPoolSync
//
ThreadPoolSync::ThreadPoolSync(uint WorkersCount)
   : ThreadPool(WorkersCount)
{
   for(uint i = 0; i < iWorkersCount; i++)
      GEThreadCreate(sWorkers[i].Thread, workerFunction, &sWorkers[i]);
}

ThreadPoolSync::~ThreadPoolSync()
{
}

void ThreadPoolSync::kickJobs()
{
   for(uint i = 0; i < iWorkersCount; i++)
      GEMutexUnlock(sWorkers[i].Mutex);
}

void ThreadPoolSync::waitForJobsCompletion()
{
   for(uint i = 0; i < iWorkersCount; i++)
      GEMutexLock(sWorkers[i].Mutex);
}

GEThreadFunction(ThreadPoolSync::workerFunction)
{
   GEProfilerThreadID("Worker");

   Worker* sWorker = static_cast<Worker*>(pData);
   ThreadPool* cPool = sWorker->Pool;
   std::function<void()> task;

   while(sWorker->Alive)
   {
      GEMutexLock(sWorker->Mutex);

      do
      {
         task = nullptr;

         cPool->lock();

         GESTLQueue(Job)& vJobsQueue = cPool->getJobsQueue();

         if(!vJobsQueue.empty())
         {
            task = vJobsQueue.front().Desc.Task;
            vJobsQueue.pop();
         }

         cPool->unlock();

         if(task)
         {
            task();
         }
      }
      while(task);

      GEMutexUnlock(sWorker->Mutex);
   }

   return 0;
}


//
//  ThreadPoolAsync
//
ThreadPoolAsync::ThreadPoolAsync(uint WorkersCount)
   : ThreadPool(WorkersCount)
{
   for(uint i = 0; i < iWorkersCount; i++)
      GEThreadCreate(sWorkers[i].Thread, workerFunction, &sWorkers[i]);
}

ThreadPoolAsync::~ThreadPoolAsync()
{
   for(uint i = 0; i < iWorkersCount; i++)
   {
      sWorkers[i].Alive = false;
      GEThreadWait(sWorkers[i].Thread);
   }
}

GEThreadFunction(ThreadPoolAsync::workerFunction)
{
   GEProfilerThreadID("Worker");

   Worker* sWorker = static_cast<Worker*>(pData);
   ThreadPool* cPool = sWorker->Pool;
   std::function<void()> task;

   while(sWorker->Alive)
   {
      do
      {
         task = nullptr;

         cPool->lock();

         GESTLQueue(Job)& vJobsQueue = cPool->getJobsQueue();

         if(!vJobsQueue.empty())
         {
            task = vJobsQueue.front().Desc.Task;
            vJobsQueue.pop();
         }

         cPool->unlock();

         if(task)
         {
            task();
         }
      }
      while(task);

      GESleep(1);
   }

   return 0;
}
