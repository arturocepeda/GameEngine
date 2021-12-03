
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Input (XInput)
//
//  --- GEInputSystem.XInput.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Input/GEInputSystem.h"

#include <XInput.h>

#pragma comment(lib, "xinput9_1_0.lib")

using namespace GE::Input;

static WORD gGamepadVibrationLevel[(int)Gamepad::VibrationDevice::Count] = {{ 0u }};

void InputSystem::platformInit()
{
}

void InputSystem::platformUpdate()
{ 
   XINPUT_STATE state;
   ZeroMemory(&state, sizeof(XINPUT_STATE));

   for(DWORD i = 0u; i < XUSER_MAX_COUNT; i++)
   { 
      const int id = (int)i;
      const bool connected = XInputGetState(i, &state) == ERROR_SUCCESS;

      Gamepad* gamepad = getGamepad(id);

      if(connected && !gamepad)
      {
         onGamepadConnected(id);
         gamepad = getGamepad(id);
      }
      else if(!connected && gamepad)
      {
         onGamepadDisconnected(id);
      }

      if(!connected)
      {
         continue;
      }

      static const int xinputButtons[] =
      {
         XINPUT_GAMEPAD_A,
         XINPUT_GAMEPAD_B,
         XINPUT_GAMEPAD_X,
         XINPUT_GAMEPAD_Y,
         XINPUT_GAMEPAD_DPAD_UP,
         XINPUT_GAMEPAD_DPAD_DOWN,
         XINPUT_GAMEPAD_DPAD_LEFT,
         XINPUT_GAMEPAD_DPAD_RIGHT,
         XINPUT_GAMEPAD_LEFT_SHOULDER,
         XINPUT_GAMEPAD_RIGHT_SHOULDER,
         XINPUT_GAMEPAD_LEFT_THUMB,
         XINPUT_GAMEPAD_RIGHT_THUMB,
         XINPUT_GAMEPAD_START,
         XINPUT_GAMEPAD_BACK
      };
      static const size_t xinputButtonsCount = sizeof(xinputButtons) / sizeof(int);

      for(size_t i = 0u; i < xinputButtonsCount; i++)
      {
         const bool buttonPressed = (state.Gamepad.wButtons & xinputButtons[i]) > 0u;

         if(buttonPressed != gamepad->mStateButtons[i])
         {
            if(buttonPressed)
            {
               setCurrentInputDevice(InputDevice::Gamepad);
               inputGamepadButtonPress(id, (Gamepad::Button)i);
            }
            else
            {
               inputGamepadButtonRelease(id, (Gamepad::Button)i);
            }
         }
      }

      GE::Vector2 sticks[] =
      {
         GE::Vector2
         (
            fmaxf(-1.0f, (float)state.Gamepad.sThumbLX / 32767),
            fmaxf(-1.0f, (float)state.Gamepad.sThumbLY / 32767)
         ),
         GE::Vector2
         (
            fmaxf(-1.0f, (float)state.Gamepad.sThumbRX / 32767),
            fmaxf(-1.0f, (float)state.Gamepad.sThumbRY / 32767)
         )
      };

      static const float kDeadZone = 0.1f;

      for(size_t i = 0u; i < 2u; i++)
      {
         if(fabsf(sticks[i].X) < kDeadZone)
         {
            sticks[i].X = 0.0f;
         }
         if(fabsf(sticks[i].Y) < kDeadZone)
         {
            sticks[i].Y = 0.0f;
         }

         if(!sticks[i].equals(gamepad->mStateSticks[i]))
         {
            setCurrentInputDevice(InputDevice::Gamepad);
            inputGamepadStickChanged(id, (Gamepad::Stick)i, sticks[i]);
         }
      }

      const float triggers[] =
      {
         (float)state.Gamepad.bLeftTrigger / 255u,
         (float)state.Gamepad.bRightTrigger / 255u
      };

      for(size_t i = 0u; i < 2u; i++)
      {
         if(!GEFloatEquals(triggers[i], gamepad->mStateTriggers[i]))
         {
            setCurrentInputDevice(InputDevice::Gamepad);
            inputGamepadTriggerChanged(id, (Gamepad::Trigger)i, triggers[i]);
         }
      }
   }
}

void InputSystem::platformShutdown()
{
}

void InputSystem::platformSetGamepadVibration(int pID, Gamepad::VibrationDevice pDevice, float pLevel)
{
   gGamepadVibrationLevel[(int)pDevice] = (WORD)(pLevel * 65535.0f);

   XINPUT_VIBRATION vibration;
   vibration.wLeftMotorSpeed = gGamepadVibrationLevel[(int)Gamepad::VibrationDevice::Left];
   vibration.wRightMotorSpeed = gGamepadVibrationLevel[(int)Gamepad::VibrationDevice::Right];

   XInputSetState((DWORD)pID, &vibration);
}
