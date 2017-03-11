
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
{
}

StateManager::~StateManager()
{
}

void StateManager::setActiveState(const ObjectName& StateName)
{
   State* cState = mStatesRegistry.get(StateName);
   GEAssert(cState);
   vActiveStates.clear();
   vActiveStates.push_back(cState);
}

State* StateManager::getActiveState()
{
   return vActiveStates.empty() ? 0 : vActiveStates.back();
}

void StateManager::pushState(const ObjectName& StateName)
{
   State* cState = mStatesRegistry.get(StateName);
   GEAssert(cState);
   vActiveStates.push_back(cState);   
}

void StateManager::popState()
{
   vActiveStates.pop_back();
}

void StateManager::releaseStates()
{
   mStatesRegistry.clear();
}
