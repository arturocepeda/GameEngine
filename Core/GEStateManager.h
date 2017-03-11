
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
      GESTLVector(State*) vActiveStates;

   public:
      StateManager();
      ~StateManager();

      template<typename T>
      void registerState()
      {
         State* cState = Allocator::alloc<T>();
         GEInvokeCtor(T, cState);
         mStatesRegistry.add(cState);
   
         if(vActiveStates.empty())
            vActiveStates.push_back(cState);
      }

      void setActiveState(const ObjectName& StateName);
      State* getActiveState();

      void pushState(const ObjectName& StateName);
      void popState();

      void releaseStates();
   };
}}
