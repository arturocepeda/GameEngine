
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
   , bInputEnabled(true)
   , iMouseX(0)
   , iMouseY(0)
   , iMouseLastX(0)
   , iMouseLastY(0)
{
}

State::~State()
{
}

StateType State::getStateType() const
{
   return eStateType;
}

bool State::getInputEnabled() const
{
   return bInputEnabled;
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

void State::inputMouse(int X, int Y)
{
   iMouseLastX = iMouseX;
   iMouseLastY = iMouseY;
   iMouseX = X;
   iMouseY = Y;
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
