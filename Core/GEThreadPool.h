
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEThreadPool.h ---
//
//////////////////////////////////////////////////////////////////


#pragma once

#include "GEThreads.h"
#include "Types/GETypeDefinitions.h"
#include "Types/GESTLTypes.h"
#include "Core/GEObject.h"

#include <functional>

namespace GE { namespace Core
{
   struct JobDesc
   {
      char Name[32];
      std::function<void()> Task;

      JobDesc(const char* cName)
      {
         strcpy(Name, cName);
      }
   };


   enum class JobState
   {
      Unassigned,
      Assigned,
      Done
   };


   struct Job
   {
      JobDesc Desc;
      JobState State;

      Job(const JobDesc& sDesc)
         : Desc(sDesc)
         , State(JobState::Unassigned) {}
   };


   enum class WorkerState
   {
      Free,
      Working,
      Idle
   };

   
   class ThreadPool;


   struct Worker
   {
      ThreadPool* Pool;

      uint ID;
      GEThread Thread;
      GEMutex Mutex;

      WorkerState State;
      Job* CurrentJob;
      bool Alive;

      Worker(ThreadPool* cPool, uint iID)
         : Pool(cPool)
         , ID(iID)
         , State(WorkerState::Free)
         , CurrentJob(0)
         , Alive(true) {}
   };


   class ThreadPool
   {
   protected:
      uint iThreadsCount;
      Worker* sWorkers;
      uint iWorkersCount;
      GESTLQueue(Job) vJobsQueue;
      GEMutex mThreadPoolMutex;

      ThreadPool(uint WorkersCount);
      ~ThreadPool();

   public:
      void queueJob(const JobDesc& Desc);

      void lock() { GEMutexLock(mThreadPoolMutex); }
      GESTLQueue(Job)& getJobsQueue() { return vJobsQueue; }
      void unlock() { GEMutexUnlock(mThreadPoolMutex); }
   };


   class ThreadPoolSync : public ThreadPool
   {
   private:
      static GEThreadFunction(workerFunction);

   public:
      ThreadPoolSync(uint WorkersCount);
      ~ThreadPoolSync();

      void kickJobs();
      void waitForJobsCompletion();
   };


   class ThreadPoolAsync : public ThreadPool
   {
   private:
      static GEThreadFunction(workerFunction);

   public:
      ThreadPoolAsync(uint WorkersCount);
      ~ThreadPoolAsync();
   };
}}
