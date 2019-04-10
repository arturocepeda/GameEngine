
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEStateManager.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GESingleton.h"
#include "GEState.h"
#include "GEObjectManager.h"
#include "GEAllocator.h"

#include <vector>

namespace GE { namespace Core
{
   class StateManager : public Singleton<StateManager>
   {
   private:
      ObjectManager<State> mStatesRegistry;
      GESTLVector(State*) mActiveStates;
      uint32_t mStatePopRequests;

   public:
      StateManager();
      ~StateManager();

      template<typename T>
      void registerState()
      {
         State* state = Allocator::alloc<T>();
         GEInvokeCtor(T, state);
         mStatesRegistry.add(state);
   
         if(mActiveStates.empty())
         {
            mActiveStates.push_back(state);
         }
      }

      void setActiveState(const ObjectName& pStateName);
      State* getActiveState();

      void pushState(const ObjectName& pStateName);
      void popState();
      void popStates(uint32_t pStatesCount);

      void update();
      void releaseStates();
   };
}}
