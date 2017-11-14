
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
#include "Entities/GEScene.h"
#include "Entities/GEEntity.h"
#include "Entities/GEComponentScript.h"

using namespace GE;
using namespace GE::Input;
using namespace GE::Core;
using namespace GE::Entities;

InputSystem::InputSystem()
   : bInputEnabled(true)
{
}

InputSystem::~InputSystem()
{
}

void InputSystem::setInputEnabled(bool Enabled)
{
   bInputEnabled = Enabled;
}

void InputSystem::inputKeyPress(char Key)
{
   if(!bInputEnabled)
      return;

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputKeyPress(Key);
   }
}

void InputSystem::inputKeyRelease(char Key)
{
   if(!bInputEnabled)
      return;

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputKeyRelease(Key);
   }
}

void InputSystem::inputMouse(const Vector2& Point)
{
   if(!bInputEnabled)
      return;

   Scene* cActiveScene = Scene::getActiveScene();

   if(cActiveScene)
   {
      GESTLVector(Component*) cComponents = cActiveScene->getComponents<ComponentScript>();

      for(uint i = 0; i < cComponents.size(); i++)
      {
         if(cComponents[i]->getOwner()->isActiveInHierarchy())
         {
            if(static_cast<ComponentScript*>(cComponents[i])->inputMouse(Point))
               return;
         }
      }
   }

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputMouse(Point);
   }
}

void InputSystem::inputMouseLeftButton()
{
   if(!bInputEnabled)
      return;

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputMouseLeftButton();
   }
}

void InputSystem::inputMouseRightButton()
{
   if(!bInputEnabled)
      return;

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputMouseRightButton();
   }
}

void InputSystem::inputMouseWheel(int Delta)
{
   if(!bInputEnabled)
      return;

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputMouseWheel(Delta);
   }
}

void InputSystem::inputTouchBegin(int ID, const Vector2& Point)
{
   if(!bInputEnabled)
      return;

   Scene* cActiveScene = Scene::getActiveScene();

   if(cActiveScene)
   {
      GESTLVector(Component*) cComponents = cActiveScene->getComponents<ComponentScript>();

      for(uint i = 0; i < cComponents.size(); i++)
      {
         if(cComponents[i]->getOwner()->isActiveInHierarchy())
         {
            if(static_cast<ComponentScript*>(cComponents[i])->inputTouchBegin(ID, Point))
               return;
         }
      }
   }

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputTouchBegin(ID, Point);
   }
}

void InputSystem::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
   if(!bInputEnabled)
      return;

   Scene* cActiveScene = Scene::getActiveScene();

   if(cActiveScene)
   {
      GESTLVector(Component*) cComponents = cActiveScene->getComponents<ComponentScript>();

      for(uint i = 0; i < cComponents.size(); i++)
      {
         if(cComponents[i]->getOwner()->isActiveInHierarchy())
         {
            if(static_cast<ComponentScript*>(cComponents[i])->inputTouchMove(ID, PreviousPoint, CurrentPoint))
               return;
         }
      }
   }

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputTouchMove(ID, PreviousPoint, CurrentPoint);
   }
}

void InputSystem::inputTouchEnd(int ID, const Vector2& Point)
{
   if(!bInputEnabled)
      return;

   Scene* cActiveScene = Scene::getActiveScene();

   if(cActiveScene)
   {
      GESTLVector(Component*) cComponents = cActiveScene->getComponents<ComponentScript>();

      for(uint i = 0; i < cComponents.size(); i++)
      {
         if(cComponents[i]->getOwner()->isActiveInHierarchy())
         {
            if(static_cast<ComponentScript*>(cComponents[i])->inputTouchEnd(ID, Point))
               return;
         }
      }
   }

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->inputTouchEnd(ID, Point);
   }
}

void InputSystem::updateAccelerometerStatus(const Vector3& Status)
{
   if(!bInputEnabled)
      return;

   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState)
   {
      cCurrentState->updateAccelerometerStatus(Status);
   }
}
