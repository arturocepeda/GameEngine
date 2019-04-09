
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Input
//
//  --- GEInputSystem.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Core/GESingleton.h"

namespace GE { namespace Input
{
   class InputListener
   {
   protected:
      uint8_t mInputPriority;

      InputListener();

   public:
      uint8_t getInputPriority() const { return mInputPriority; }

      virtual bool inputKeyPress(char pKey);
      virtual bool inputKeyRelease(char pKey);

      virtual bool inputMouse(const Vector2& pPoint);
      virtual bool inputMouseLeftButton();
      virtual bool inputMouseRightButton();
      virtual bool inputMouseWheel(int pDelta);

      virtual bool inputTouchBegin(int pID, const Vector2& pPoint);
      virtual bool inputTouchMove(int pID, const Vector2& pPreviousPoint, const Vector2& pCurrentPoint);
      virtual bool inputTouchEnd(int pID, const Vector2& pPoint);
   };


   class InputSystem : public Core::Singleton<InputSystem>
   {
   private:
      enum class InputEventType : uint16_t
      {
         Invalid,

         KeyPressed,
         KeyReleased,

         MouseMoved,
         MouseLeftButton,
         MouseRightButton,
         MouseWheel,

         TouchBegin,
         TouchMoved,
         TouchEnd
      };

      struct InputEvent
      {
         InputEventType mType;
         int16_t mID;
         Vector2 mPointA;
         Vector2 mPointB;

         InputEvent()
            : mType(InputEventType::Invalid)
            , mID(0)
         {
         }
      };

      GEMutex mEventsMutex;
      GESTLVector(InputListener*) mListeners;
      GESTLVector(InputEvent) mEvents;

      bool mInputEnabled;
      Vector2 mMousePosition;
      Vector3 mAccelerometerStatus;

   public:
      InputSystem();
      ~InputSystem();

      void addListener(InputListener* pListener);
      void removeListener(InputListener* pListener);

      void setInputEnabled(bool pEnabled);
      void processEvents();

      const Vector2& getMousePosition() const { return mMousePosition; }
      const Vector3& getAccelerometerStatus() const { return mAccelerometerStatus; } 

      void inputKeyPress(char pKey);
      void inputKeyRelease(char pKey);

      void inputMouse(const Vector2& pPoint);
      void inputMouseLeftButton();
      void inputMouseRightButton();
      void inputMouseWheel(int pDelta);

      void inputTouchBegin(int pID, const Vector2& pPoint);
      void inputTouchMove(int pID, const Vector2& pPreviousPoint, const Vector2& pCurrentPoint);
      void inputTouchEnd(int pID, const Vector2& pPoint);

      void updateAccelerometerStatus(const Vector3& pStatus);
   };
}}
