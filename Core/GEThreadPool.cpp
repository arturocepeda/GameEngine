
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

   GEMutexInit(mQueueMutex);
   GEConditionVariableInit(cvJobsPending);
   GEConditionVariableInit(cvJobsCompleted);

   iWorkersActiveMask = 0;

   for(uint i = 0; i < iWorkersCount; i++)
   {
      sWorkers[i] = Worker(this, i);
      GEThreadCreate(sWorkers[i].Thread, workerFunction, &sWorkers[i]);
   }
}

ThreadPool::~ThreadPool()
{
   for(uint i = 0; i < iWorkersCount; i++)
   {
      GEThreadClose(sWorkers[i].Thread);
   }

   Allocator::free(sWorkers);

   GEConditionVariableDestroy(cvJobsPending);
   GEConditionVariableDestroy(cvJobsCompleted);
   GEMutexDestroy(mQueueMutex);
}

void ThreadPool::kickJobsProtected()
{
   GEMutexLock(mQueueMutex);

   for(uint i = 0; i < iWorkersCount; i++)
   {
      iWorkersActiveMask |= 1 << i;
   }

   GEMutexUnlock(mQueueMutex);

   GEConditionVariableSignal(cvJobsPending);
}

GEThreadFunction(ThreadPool::workerFunction)
{
   GEProfilerThreadID("Worker");

   Worker* sWorker = static_cast<Worker*>(pData);
   ThreadPool* cPool = sWorker->Pool;
   std::function<void()> task;

   while(sWorker->Alive)
   {
      GEMutexLock(cPool->mQueueMutex);
      GEConditionVariableWait(cPool->cvJobsPending, cPool->mQueueMutex, (cPool->iWorkersActiveMask & (1 << sWorker->ID)) > 0);
      GEMutexUnlock(cPool->mQueueMutex);

      do
      {
         task = nullptr;

         GEMutexLock(cPool->mQueueMutex);

         GESTLQueue(Job)& vJobsQueue = cPool->getJobsQueue();

         if(!vJobsQueue.empty())
         {
            task = vJobsQueue.front().Desc.Task;
            vJobsQueue.pop();
         }

         GEMutexUnlock(cPool->mQueueMutex);

         if(task)
         {
            task();
         }
      }
      while(task);

      GEMutexLock(cPool->mQueueMutex);
      cPool->iWorkersActiveMask &= ~(1 << sWorker->ID);
      GEMutexUnlock(cPool->mQueueMutex);

      GEConditionVariableSignal(cPool->cvJobsCompleted);
   }

   return 0;
}

void ThreadPool::queueJob(const JobDesc& Desc)
{
   GEAssert(Desc.Task);

   GEMutexLock(mQueueMutex);
   vJobsQueue.push(Job(Desc));
   GEMutexUnlock(mQueueMutex);
}


//
//  ThreadPoolSync
//
ThreadPoolSync::ThreadPoolSync(uint WorkersCount)
   : ThreadPool(WorkersCount)
{
}

ThreadPoolSync::~ThreadPoolSync()
{
}

void ThreadPoolSync::kickJobs()
{
   kickJobsProtected();
}

void ThreadPoolSync::waitForJobsCompletion()
{
   GEMutexLock(mQueueMutex);
   GEConditionVariableWait(cvJobsCompleted, mQueueMutex, iWorkersActiveMask == 0);
   GEMutexUnlock(mQueueMutex);
}



//
//  ThreadPoolAsync
//
ThreadPoolAsync::ThreadPoolAsync(uint WorkersCount)
   : ThreadPool(WorkersCount)
{
}

ThreadPoolAsync::~ThreadPoolAsync()
{
}

void ThreadPoolAsync::queueJob(const JobDesc& Desc)
{
   ThreadPool::queueJob(Desc);
   kickJobsProtected();
}
