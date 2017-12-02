
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


   struct Job
   {
      JobDesc Desc;

      Job(const JobDesc& sDesc)
         : Desc(sDesc) {}
   };

   
   class ThreadPool;


   struct Worker
   {
      ThreadPool* Pool;

      uint ID;
      GEThread Thread;

      Job* CurrentJob;
      bool Alive;

      Worker(ThreadPool* cPool, uint iID)
         : Pool(cPool)
         , ID(iID)
         , CurrentJob(0)
         , Alive(true) {}
   };


   class ThreadPool
   {
   protected:
      Worker* sWorkers;
      uint iWorkersCount;
      GESTLQueue(Job) vJobsQueue;

      ThreadPool(uint WorkersCount);
      ~ThreadPool();

      void kickJobsProtected();

      static GEThreadFunction(workerFunction);

   public:
      GEMutex mQueueMutex;
      GEConditionVariable cvJobsPending;
      GEConditionVariable cvJobsCompleted;
      uint iWorkersActiveMask;

      virtual void queueJob(const JobDesc& Desc);

      GESTLQueue(Job)& getJobsQueue() { return vJobsQueue; }
   };


   class ThreadPoolSync : public ThreadPool
   {
   public:
      ThreadPoolSync(uint WorkersCount);
      ~ThreadPoolSync();

      void kickJobs();
      void waitForJobsCompletion();
   };


   class ThreadPoolAsync : public ThreadPool
   {
   public:
      ThreadPoolAsync(uint WorkersCount);
      ~ThreadPoolAsync();

      virtual void queueJob(const JobDesc& Desc) override;
   };
}}
