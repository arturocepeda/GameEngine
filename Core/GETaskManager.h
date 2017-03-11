
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GETaskManager.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GESingleton.h"
#include "Rendering/GERenderSystem.h"
#include "Audio/GEAudioSystem.h"
#include "GEStateManager.h"
#include "GEThreadPool.h"

#include <queue>
#include <functional>

namespace GE { namespace Core
{
   enum class JobType
   {
      General,
      Frame
   };


   class State;


   class TaskManager : public Singleton<TaskManager>
   {
   private:
      Rendering::RenderSystem* cRender;
      Audio::AudioSystem* cAudio;
      StateManager* cStateManager;
      uint iFrameCounter;
      State* cCurrentState;
      bool bExitPending;

      ThreadPoolSync* cFrameThreadPool;
      ThreadPoolAsync* cGeneralThreadPool;

   public:
      TaskManager();
      ~TaskManager();

      void update();
      void render();

      void queueJob(const JobDesc& sJobDesc, JobType eType = JobType::General);

      bool getExitPending() const;
      void exit();
   };
}}
