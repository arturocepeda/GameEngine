
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Input
//
//  --- GEInputSystem.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEInputSystem.h"
#include "Core/GEStateManager.h"

using namespace GE;
using namespace GE::Input;
using namespace GE::Core;

InputSystem::InputSystem()
{
}

InputSystem::~InputSystem()
{
}

void InputSystem::inputKeyPress(char Key)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputKeyPress(Key);
   }
}

void InputSystem::inputKeyRelease(char Key)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputKeyRelease(Key);
   }
}

void InputSystem::inputMouse(int X, int Y)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputMouse(X, Y);
   }
}

void InputSystem::inputMouseLeftButton()
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputMouseLeftButton();
   }
}

void InputSystem::inputMouseRightButton()
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputMouseRightButton();
   }
}

void InputSystem::inputMouseWheel(int Delta)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputMouseWheel(Delta);
   }
}

void InputSystem::inputTouchBegin(int ID, const Vector2& Point)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputTouchBegin(ID, Point);
   }
}

void InputSystem::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputTouchMove(ID, PreviousPoint, CurrentPoint);
   }
}

void InputSystem::inputTouchEnd(int ID, const Vector2& Point)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputTouchEnd(ID, Point);
   }
}

void InputSystem::updateAccelerometerStatus(const Vector3& Status)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->updateAccelerometerStatus(Status);
   }
}
