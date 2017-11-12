
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
{
}

InputSystem::~InputSystem()
{
}

void InputSystem::inputKeyPress(char Key)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputKeyPress(Key);
   }
}

void InputSystem::inputKeyRelease(char Key)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputKeyRelease(Key);
   }
}

void InputSystem::inputMouse(const Vector2& Point)
{
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

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputMouse(Point);
   }
}

void InputSystem::inputMouseLeftButton()
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputMouseLeftButton();
   }
}

void InputSystem::inputMouseRightButton()
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputMouseRightButton();
   }
}

void InputSystem::inputMouseWheel(int Delta)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputMouseWheel(Delta);
   }
}

void InputSystem::inputTouchBegin(int ID, const Vector2& Point)
{
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

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputTouchBegin(ID, Point);
   }
}

void InputSystem::inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint)
{
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

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputTouchMove(ID, PreviousPoint, CurrentPoint);
   }
}

void InputSystem::inputTouchEnd(int ID, const Vector2& Point)
{
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

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->inputTouchEnd(ID, Point);
   }
}

void InputSystem::updateAccelerometerStatus(const Vector3& Status)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   if(cCurrentState && cCurrentState->getInputEnabled())
   {
      cCurrentState->updateAccelerometerStatus(Status);
   }
}
