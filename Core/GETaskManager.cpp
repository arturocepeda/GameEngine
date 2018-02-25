
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GETaskManager.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GETaskManager.h"
#include "GEState.h"
#include "GEProfiler.h"
#include "GEDevice.h"
#include "Entities/GEScene.h"

#include <cassert>

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;

TaskManager::TaskManager()
   : iFrameCounter(0)
   , cCurrentState(0)
   , bExitPending(false)
{
   cRender = Rendering::RenderSystem::getInstance();
   cAudio = Audio::AudioSystem::getInstance();
   cStateManager = StateManager::getInstance();

   cFrameThreadPool = Allocator::alloc<ThreadPoolSync>();
   GEInvokeCtor(ThreadPoolSync, cFrameThreadPool)(Device::getNumberOfCPUCores() - 2);

   cGeneralThreadPool = Allocator::alloc<ThreadPoolAsync>();
   GEInvokeCtor(ThreadPoolAsync, cGeneralThreadPool)(1);
}

TaskManager::~TaskManager()
{
   GEInvokeDtor(ThreadPoolAsync, cGeneralThreadPool);
   GEInvokeDtor(ThreadPoolSync, cFrameThreadPool);
}

void TaskManager::queueJob(const JobDesc& sJobDesc, JobType eType)
{
   if(eType == JobType::General)
      cGeneralThreadPool->queueJob(sJobDesc);
   else
      cFrameThreadPool->queueJob(sJobDesc);
}

uint TaskManager::getFrameCounter() const
{
   return iFrameCounter;
}

void TaskManager::update()
{
   GEProfilerMarker("TaskManager::update()");

   // handle state changes
   State* cActiveState = cStateManager->getActiveState();

   if(cCurrentState != cActiveState)
   {
      if(cCurrentState && (!cActiveState || cActiveState->getStateType() != StateType::Modal))
         cCurrentState->deactivate();

      if(cActiveState && (!cCurrentState || cCurrentState->getStateType() != StateType::Modal))
         cActiveState->activate();

      cCurrentState = cActiveState;
   }

   // update current state
   if(cCurrentState)
   {
      cCurrentState->update();
   }

   // queue scene update jobs
   Scene* cDebuggingScene = Scene::getDebuggingScene();
   Scene* cActiveScene = Scene::getActiveScene();

   if(cDebuggingScene)
   {
      cDebuggingScene->queueUpdateJobs();
   }

   if(cActiveScene)
   {
      cActiveScene->queueUpdateJobs();
   }

   // kick scene update jobs
   cFrameThreadPool->kickJobs();

   // update scene
   if(cDebuggingScene)
   {
      cDebuggingScene->update();
   }

   if(cActiveScene)
   {
      cActiveScene->update();
   }

   // wait for scene update jobs completion
   cFrameThreadPool->waitForJobsCompletion();

   // queue scene objects for rendering
   if(cDebuggingScene)
   {
      cDebuggingScene->queueForRendering();
   }

   if(cActiveScene)
   {
      cActiveScene->queueForRendering();
   }

   // kick queue for rendering jobs
   cFrameThreadPool->kickJobs();

   // update audio system
   if(cAudio)
   {
      cAudio->update();
   }

   // wait for queue for rendering jobs completion
   cFrameThreadPool->waitForJobsCompletion();

   // increment the frame counter
   iFrameCounter++;
}

void TaskManager::render()
{
   GEProfilerMarker("TaskManager::render()");

   cRender->renderBegin();
   cRender->renderFrame();
   cRender->renderEnd();
   cRender->clearRenderingQueues();
}

bool TaskManager::getExitPending() const
{
   return bExitPending;
}

void TaskManager::exit()
{
   bExitPending = true;
}
