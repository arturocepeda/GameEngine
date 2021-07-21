
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEStateManager.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEStateManager.h"

using namespace GE;
using namespace GE::Core;

StateManager::StateManager()
   : mStatePopRequests(0u)
{
}

StateManager::~StateManager()
{
}

void StateManager::setActiveState(const ObjectName& pStateName)
{
   State* state = mStatesRegistry.get(pStateName);
   GEAssert(state);
   mActiveStates.clear();
   mActiveStates.push_back(state);
}

State* StateManager::getActiveState()
{
   return mActiveStates.empty() ? 0 : mActiveStates.back();
}

void StateManager::pushState(const ObjectName& pStateName)
{
   State* state = mStatesRegistry.get(pStateName);
   GEAssert(state);
   mActiveStates.push_back(state);   
}

void StateManager::popState()
{
   mActiveStates.pop_back();
}

void StateManager::requestPopStates(uint32_t pStatesCount)
{
   mStatePopRequests += pStatesCount;
}

void StateManager::update()
{
   if(mStatePopRequests > 0u)
   {
      popState();
      mStatePopRequests--;
   }
}

void StateManager::releaseStates()
{
   mStatesRegistry.clear();
}
