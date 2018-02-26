
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEThreads.h ---
//
//////////////////////////////////////////////////////////////////


#pragma once

#include "GEPlatform.h"

//
//  Windows
//
#if defined(GE_PLATFORM_WINDOWS)


# include <windows.h>


# define GEThread  HANDLE
# define GEThreadFunction(FunctionName)  DWORD WINAPI FunctionName(LPVOID pData)

# define GEThreadCreate(Thread, FunctionName, DataPointer)  Thread = CreateThread(NULL, 0, FunctionName, DataPointer, 0, NULL)
# define GEThreadWait(Thread)  WaitForSingleObject(Thread, INFINITE)
# define GEThreadClose(Thread)  CloseHandle(Thread)

# define GEThreadID  (GE::uint)GetCurrentThreadId()
# define GEThreadAffinity(Thread, CoreIndex)  SetThreadAffinityMask(Thread, 1 << CoreIndex)


# define GEMutex  CRITICAL_SECTION

# define GEMutexInit(Mutex)  InitializeCriticalSection(&Mutex)
# define GEMutexLock(Mutex)  EnterCriticalSection(&Mutex)
# define GEMutexUnlock(Mutex)  LeaveCriticalSection(&Mutex)
# define GEMutexDestroy(Mutex)  DeleteCriticalSection(&Mutex)


# define GEConditionVariable  CONDITION_VARIABLE

# define GEConditionVariableInit(CV)  InitializeConditionVariable(&CV)
# define GEConditionVariableWait(CV, Mutex, Condition)  while(!(Condition)) SleepConditionVariableCS(&CV, &Mutex, INFINITE)
# define GEConditionVariableSignal(CV)  WakeAllConditionVariable(&CV)
# define GEConditionVariableDestroy(CV)


# define GESleep(Milliseconds)  Sleep(Milliseconds)


//
//  Windows Phone 8
//
#elif defined(GE_PLATFORM_WP8)


# include <ppltasks.h>


# define GEThread  HANDLE
# define GEThreadFunction(FunctionName)  DWORD WINAPI FunctionName(LPVOID pData)

# define GEThreadCreate(Thread, FunctionName, DataPointer)  Concurrency::create_task([]{ FunctionName(DataPointer); })
# define GEThreadWait(Thread)
# define GEThreadClose(Thread)

# define GEThreadAffinity(Thread, CoreIndex)


# define GEMutex  CRITICAL_SECTION

# define GEMutexInit(Mutex)  InitializeCriticalSectionEx(&Mutex, 0, 0)
# define GEMutexLock(Mutex)  EnterCriticalSection(&Mutex)
# define GEMutexUnlock(Mutex)  LeaveCriticalSection(&Mutex)
# define GEMutexDestroy(Mutex)  DeleteCriticalSection(&Mutex)


# define GESleep(Milliseconds)



//
//  Unix
//
#else


# include <pthread.h>
# include <unistd.h>
# include <sys/syscall.h>


# define GEThread  pthread_t
# define GEThreadFunction(FunctionName)  void* FunctionName(void* pData)

# define GEThreadCreate(Thread, FunctionName, DataPointer)  pthread_create(&Thread, NULL, FunctionName, DataPointer)
# define GEThreadWait(Thread)  pthread_join(Thread, NULL)
# define GEThreadClose(Thread)

# define GEThreadID  (GE::uint)pthread_getthreadid_np()
# define GEThreadAffinity(Thread, CoreIndex) \
   cpu_set_t cpuset; \
   CPU_ZERO(&cpuset); \
   CPU_SET(CoreIndex, &cpuset); \
   syscall(__NR_sched_setaffinity, gettid(), sizeof(cpu_set_t), &cpuset)


# define GEMutex  pthread_mutex_t

# define GEMutexInit(Mutex)  pthread_mutex_init(&Mutex, NULL)
# define GEMutexLock(Mutex)  pthread_mutex_lock(&Mutex)
# define GEMutexUnlock(Mutex)  pthread_mutex_unlock(&Mutex)
# define GEMutexDestroy(Mutex)  pthread_mutex_destroy(&Mutex)


# define GEConditionVariable  pthread_cond_t

# define GEConditionVariableInit(CV)  pthread_cond_init(&CV, NULL);
# define GEConditionVariableWait(CV, Mutex, Condition)  while(!(Condition)) pthread_cond_wait(&CV, &Mutex);
# define GEConditionVariableSignal(CV)  pthread_cond_broadcast(&CV);
# define GEConditionVariableDestroy(CV)  pthread_cond_destroy(&CV);


# define GESleep(Milliseconds)  usleep(Milliseconds * 1000)


#endif
