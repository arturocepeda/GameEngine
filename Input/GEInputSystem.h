
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

#include <atomic>

namespace GE { namespace Input
{
   enum class InputDevice : uint8_t
   {
      None,
      Keyboard,
      Mouse,
      Gamepad,
      Touchpad
   };


   struct Gamepad
   {
      enum class Button
      {
         A,
         B,
         X,
         Y,
         DPadUp,
         DPadDown,
         DPadLeft,
         DPadRight,
         ShoulderLeft,
         ShoulderRight,
         StickLeft,
         StickRight,
         Start,
         Back,

         Count
      };
      enum class Stick
      {
         Left,
         Right,

         Count
      };
      enum class Trigger
      {
         Left,
         Right,

         Count
      };
      enum class VibrationDevice
      {
         Left,
         Right,

         Count
      };

      int mID;
      bool mStateButtons[(int)Button::Count];
      Vector2 mStateSticks[(int)Stick::Count];
      float mStateTriggers[(int)Trigger::Count];

      Gamepad()
         : mID(0)
      {
         memset(mStateButtons, 0, sizeof(mStateButtons));
         memset(mStateSticks, 0, sizeof(mStateSticks));
         memset(mStateTriggers, 0, sizeof(mStateTriggers));
      }
   };


   class InputListener
   {
   protected:
      uint8_t mInputPriority;

      InputListener();

   public:
      uint8_t getInputPriority() const { return mInputPriority; }

      virtual bool inputKeyPress(char pKey);
      virtual bool inputKeyRelease(char pKey);
      virtual bool inputKeyText(uint16_t pUnicode);

      virtual bool inputMouse(const Vector2& pPoint);
      virtual bool inputMouseLeftButton();
      virtual bool inputMouseRightButton();
      virtual bool inputMouseWheel(int pDelta);

      virtual bool inputGamepadButtonPress(int pID, Gamepad::Button pButton);
      virtual bool inputGamepadButtonRelease(int pID, Gamepad::Button pButton);
      virtual bool inputGamepadStickChanged(int pID, Gamepad::Stick pStick, const Vector2& pState);
      virtual bool inputGamepadTriggerChanged(int pID, Gamepad::Trigger pTrigger, float pState);

      virtual bool inputTouchBegin(int pID, const Vector2& pPoint);
      virtual bool inputTouchMove(int pID, const Vector2& pPreviousPoint, const Vector2& pCurrentPoint);
      virtual bool inputTouchEnd(int pID, const Vector2& pPoint);
   };


   class InputSystem : public Core::Singleton<InputSystem>
   {
   private:
      enum class InputEventType : uint8_t
      {
         Invalid,

         KeyPressed,
         KeyReleased,
         KeyText,

         MouseMoved,
         MouseLeftButton,
         MouseRightButton,
         MouseWheel,

         GamepadButtonPressed,
         GamepadButtonReleased,
         GamepadStickChanged,
         GamepadTriggerChanged,

         TouchBegin,
         TouchMoved,
         TouchEnd
      };

      struct InputEvent
      {
         InputEventType mType;
         uint8_t mIndex;
         int16_t mID;
         Vector2 mPointA;
         Vector2 mPointB;

         InputEvent()
            : mType(InputEventType::Invalid)
            , mIndex(0u)
            , mID(0)
         {
         }
      };

      GEMutex mEventsMutex;
      GESTLVector(InputListener*) mListeners;
      GESTLVector(InputEvent) mEvents;
      GESTLVector(Gamepad) mGamepads;

      std::atomic<InputDevice> mCurrentInputDevice;
      bool mInputEnabled;
      Vector2 mMousePosition;
      Vector3 mAccelerometerStatus;

      void platformInit();
      void platformUpdate();
      void platformShutdown();

      void platformSetGamepadVibration(int pID, Gamepad::VibrationDevice pDevice, float pLevel);

   public:
      InputSystem();
      ~InputSystem();

      void init();
      void update();
      void shutdown();

      void addListener(InputListener* pListener);
      void removeListener(InputListener* pListener);

      InputDevice getCurrentInputDevice();
      void setCurrentInputDevice(InputDevice pDevice);

      void onGamepadConnected(int pID);
      void onGamepadDisconnected(int pID);

      void setInputEnabled(bool pEnabled);
      void processEvents();

      int getConnectedGamepadID(size_t pIndex) const;
      Gamepad* getGamepad(int pID);

      const Vector2& getMousePosition() const { return mMousePosition; }
      const Vector3& getAccelerometerStatus() const { return mAccelerometerStatus; } 

      void setGamepadVibration(int pID, Gamepad::VibrationDevice pDevice, float pLevel);

      void inputKeyPress(char pKey);
      void inputKeyRelease(char pKey);
      void inputKeyText(uint16_t pUnicode);

      void inputMouse(const Vector2& pPoint);
      void inputMouseLeftButton();
      void inputMouseRightButton();
      void inputMouseWheel(int pDelta);

      void inputGamepadButtonPress(int pID, Gamepad::Button pButton);
      void inputGamepadButtonRelease(int pID, Gamepad::Button pButton);
      void inputGamepadStickChanged(int pID, Gamepad::Stick pStick, const Vector2& pState);
      void inputGamepadTriggerChanged(int pID, Gamepad::Trigger pTrigger, float pState);

      void inputTouchBegin(int pID, const Vector2& pPoint);
      void inputTouchMove(int pID, const Vector2& pPreviousPoint, const Vector2& pCurrentPoint);
      void inputTouchEnd(int pID, const Vector2& pPoint);

      void updateAccelerometerStatus(const Vector3& pStatus);
   };
}}
