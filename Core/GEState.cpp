
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEState.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEState.h"

using namespace GE;
using namespace GE::Core;

State::State(const ObjectName& Name, void* GlobalData)
   : Object(Name)
   , pGlobalData(GlobalData)
   , eStateType(StateType::Exclusive)
{
   mInputPriority = 64u;
}

State::~State()
{
}

StateType State::getStateType() const
{
   return eStateType;
}

void State::pause()
{
}

void State::resume()
{
}
