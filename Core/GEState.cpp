
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
#include <stdio.h>

using namespace GE;
using namespace GE::Core;

State::State(const ObjectName& Name, void* GlobalData)
   : Object(Name)
   , pGlobalData(GlobalData)
   , eStateType(StateType::Exclusive)
{
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

void State::inputKeyPress(char Key)
{
}

void State::inputKeyRelease(char Key)
{
}

void State::inputMouse(const Vector2& Point)
{
}

void State::inputMouseLeftButton()
{
}

void State::inputMouseRightButton()
{
}

void State::inputMouseWheel(int Delta)
{
}

void State::inputTouchBegin(int ID, const Vector2& Point)
{
}

void State::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
}

void State::inputTouchEnd(int ID, const Vector2& Point)
{
}
   
void State::updateAccelerometerStatus(const Vector3& Status)
{
}
