
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

const uint AudioSystemUpdateFrames = 15;

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
      cCurrentState->update();

   // update the active scene
   Scene* cActiveScene = Scene::getActiveScene();

   if(cActiveScene)
   {
      cActiveScene->update();
   }

   // kick scene update jobs
   cFrameThreadPool->kickJobs();

   // update audio system
   if(cAudio)
   {
      iFrameCounter++;

      if(iFrameCounter >= AudioSystemUpdateFrames)
      {
         cAudio->update();
         iFrameCounter = 0;
      }
   }

   // wait for scene update jobs completion
   cFrameThreadPool->waitForJobsCompletion();

   // queue scene objects for rendering
   if(cActiveScene)
   {
      cActiveScene->queueForRendering();
   }

   // kick queue for rendering jobs
   cFrameThreadPool->kickJobs();

   // wait for queue for rendering jobs completion
   cFrameThreadPool->waitForJobsCompletion();
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
