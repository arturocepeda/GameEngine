
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

#include <algorithm>

using namespace GE;
using namespace GE::Input;
using namespace GE::Core;


//
//  InputListener
//
InputListener::InputListener()
   : mInputPriority(128u)
{
}

bool InputListener::inputKeyPress(char)
{
   return false;
}
bool InputListener::inputKeyRelease(char)
{
   return false;
}
bool InputListener::inputKeyText(uint16_t)
{
   return false;
}

bool InputListener::inputMouse(const Vector2&)
{
   return false;
}
bool InputListener::inputMouseLeftButton()
{
   return false;
}
bool InputListener::inputMouseRightButton()
{
   return false;
}
bool InputListener::inputMouseWheel(int)
{
   return false;
}

bool InputListener::inputGamepadButtonPress(int, Gamepad::Button)
{
   return false;
}
bool InputListener::inputGamepadButtonRelease(int, Gamepad::Button)
{
   return false;
}
bool InputListener::inputGamepadStickChanged(int, Gamepad::Stick, const Vector2&)
{
   return false;
}
bool InputListener::inputGamepadTriggerChanged(int, Gamepad::Trigger, float)
{
   return false;
}

bool InputListener::inputTouchBegin(int, const Vector2&)
{
   return false;
}
bool InputListener::inputTouchMove(int, const Vector2&, const Vector2&)
{
   return false;
}
bool InputListener::inputTouchEnd(int, const Vector2&)
{
   return false;
}


//
//  InputSystem
//
InputSystem::InputSystem()
   : mInputEnabled(true)
{
   GEMutexInit(mEventsMutex);
}

InputSystem::~InputSystem()
{
   GEMutexDestroy(mEventsMutex);
}

bool inputListenerComparer(InputListener* pListenerA, InputListener* pListenerB)
{
   return (pListenerA->getInputPriority() < pListenerB->getInputPriority());
}

void InputSystem::addListener(InputListener* pListener)
{
   mListeners.push_back(pListener);
   std::sort(mListeners.begin(), mListeners.end(), inputListenerComparer);
}

void InputSystem::removeListener(InputListener* pListener)
{   
   for(size_t i = 0; i < mListeners.size(); i++)
   {
      if(mListeners[i] == pListener)
      {
         mListeners.erase(mListeners.begin() + i);
         break;
      }
   }
}

void InputSystem::processEvents()
{
   GEMutexLock(mEventsMutex);

   if(mListeners.empty() || mEvents.empty())
   {
      GEMutexUnlock(mEventsMutex);
      return;
   }

   for(size_t i = 0; i < mEvents.size(); i++)
   {
      const InputEvent& event = mEvents[i];

      switch(event.mType)
      {
      case InputEventType::KeyPressed:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputKeyPress((char)event.mID))
               break;
         }
         break;
      case InputEventType::KeyReleased:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputKeyRelease((char)event.mID))
               break;
         }
         break;
      case InputEventType::KeyText:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputKeyText((uint16_t)event.mID))
               break;
         }
         break;
      case InputEventType::MouseMoved:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputMouse(event.mPointA))
               break;
         }
         break;
      case InputEventType::MouseLeftButton:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputMouseLeftButton())
               break;
         }
         break;
      case InputEventType::MouseRightButton:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputMouseRightButton())
               break;
         }
         break;
      case InputEventType::MouseWheel:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputMouseWheel(event.mID))
               break;
         }
         break;
      case InputEventType::GamepadButtonPressed:
         getGamepad(event.mID)->mStateButtons[event.mIndex] = true;
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputGamepadButtonPress(event.mID, (Gamepad::Button)event.mIndex))
               break;
         }
         break;
      case InputEventType::GamepadButtonReleased:
         getGamepad(event.mID)->mStateButtons[event.mIndex] = false;
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputGamepadButtonRelease(event.mID, (Gamepad::Button)event.mIndex))
               break;
         }
         break;
      case InputEventType::GamepadStickChanged:
         getGamepad(event.mID)->mStateSticks[event.mIndex] = event.mPointA;
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputGamepadStickChanged(event.mID, (Gamepad::Stick)event.mIndex, event.mPointA))
               break;
         }
         break;
      case InputEventType::GamepadTriggerChanged:
         getGamepad(event.mID)->mStateTriggers[event.mIndex] = event.mPointA.X;
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputGamepadTriggerChanged(event.mID, (Gamepad::Trigger)event.mIndex, event.mPointA.X))
               break;
         }
         break;
      case InputEventType::TouchBegin:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputTouchBegin(event.mID, event.mPointA))
               break;
         }
         break;
      case InputEventType::TouchMoved:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputTouchMove(event.mID, event.mPointA, event.mPointB))
               break;
         }
         break;
      case InputEventType::TouchEnd:
         for(size_t j = 0; j < mListeners.size(); j++)
         {
            if(mListeners[j]->inputTouchEnd(event.mID, event.mPointA))
               break;
         }
         break;
      }
   }

   mEvents.clear();

   GEMutexUnlock(mEventsMutex);
}

void InputSystem::onGamepadConnected(int pID)
{
   if(!getGamepad(pID))
   {
      mGamepads.emplace_back();
      mGamepads.back().mID = pID;
   }
}

void InputSystem::onGamepadDisconnected(int pID)
{
   for(size_t i = 0u; i < mGamepads.size(); i++)
   {
      if(mGamepads[i].mID == pID)
      {
         mGamepads.erase(mGamepads.begin() + i);
         break;
      }
   }
}

void InputSystem::setInputEnabled(bool pEnabled)
{
   mInputEnabled = pEnabled;

   if(mInputEnabled)
   {
      inputMouse(mMousePosition);
   }
}

Gamepad* InputSystem::getGamepad(int pID)
{
   for(size_t i = 0u; i < mGamepads.size(); i++)
   {
      if(mGamepads[i].mID == pID)
      {
         return &mGamepads[i];
      }
   }

   return nullptr;
}

void InputSystem::inputKeyPress(char pKey)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::KeyPressed;
   event.mID = (int16_t)pKey;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputKeyRelease(char pKey)
{
   InputEvent event;
   event.mType = InputEventType::KeyReleased;
   event.mID = (int16_t)pKey;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputKeyText(uint16_t pUnicode)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::KeyText;
   event.mID = (int16_t)pUnicode;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputMouse(const Vector2& pPoint)
{
   mMousePosition = pPoint;

   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::MouseMoved;
   event.mPointA = pPoint;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputMouseLeftButton()
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::MouseLeftButton;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputMouseRightButton()
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::MouseRightButton;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputMouseWheel(int pDelta)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::MouseWheel;
   event.mID = (int16_t)pDelta;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputGamepadButtonPress(int pID, Gamepad::Button pButton)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::GamepadButtonPressed;
   event.mIndex = (uint8_t)pButton;
   event.mID = (int16_t)pID;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputGamepadButtonRelease(int pID, Gamepad::Button pButton)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::GamepadButtonReleased;
   event.mIndex = (uint8_t)pButton;
   event.mID = (int16_t)pID;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputGamepadStickChanged(int pID, Gamepad::Stick pStick, const Vector2& pState)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::GamepadStickChanged;
   event.mIndex = (uint8_t)pStick;
   event.mID = (int16_t)pID;
   event.mPointA = pState;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputGamepadTriggerChanged(int pID, Gamepad::Trigger pTrigger, float pState)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::GamepadTriggerChanged;
   event.mIndex = (uint8_t)pTrigger;
   event.mID = (int16_t)pID;
   event.mPointA.X = pState;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputTouchBegin(int pID, const Vector2& pPoint)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::TouchBegin;
   event.mID = (int16_t)pID;
   event.mPointA = pPoint;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputTouchMove(int pID, const Vector2& pPreviousPoint, const Vector2& pCurrentPoint)
{
   if(!mInputEnabled)
      return;

   InputEvent event;
   event.mType = InputEventType::TouchMoved;
   event.mID = (int16_t)pID;
   event.mPointA = pPreviousPoint;
   event.mPointB = pCurrentPoint;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::inputTouchEnd(int pID, const Vector2& pPoint)
{
   InputEvent event;
   event.mType = InputEventType::TouchEnd;
   event.mID = (int16_t)pID;
   event.mPointA = pPoint;

   GEMutexLock(mEventsMutex);
   mEvents.push_back(event);
   GEMutexUnlock(mEventsMutex);
}

void InputSystem::updateAccelerometerStatus(const Vector3& pStatus)
{
   mAccelerometerStatus = pStatus;
}
