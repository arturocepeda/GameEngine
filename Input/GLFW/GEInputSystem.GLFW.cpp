
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Input (GLFW)
//
//  --- GEInputSystem.GLFW.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Input/GEInputSystem.h"

#include "Externals/glfw/include/GLFW/glfw3.h"

using namespace GE::Input;

void InputSystem::platformInit()
{
}

void InputSystem::platformUpdate()
{ 
   GLFWgamepadstate state;

   static const int kMaxGamepadsCount = 4;

   for(int id = 0; id < kMaxGamepadsCount; id++)
   { 
      const bool connected = glfwJoystickIsGamepad(id) && glfwGetGamepadState(id, &state);
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

      static const int glfwButtons[] =
      {
         GLFW_GAMEPAD_BUTTON_A,
         GLFW_GAMEPAD_BUTTON_B,
         GLFW_GAMEPAD_BUTTON_X,
         GLFW_GAMEPAD_BUTTON_Y,
         GLFW_GAMEPAD_BUTTON_DPAD_UP,
         GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
         GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
         GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
         GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
         GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
         GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
         GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,
         GLFW_GAMEPAD_BUTTON_START,
         GLFW_GAMEPAD_BUTTON_BACK
      };
      static const size_t glfwButtonsCount = sizeof(glfwButtons) / sizeof(int);

      for(size_t i = 0u; i < glfwButtonsCount; i++)
      {
         const bool buttonPressed = state.buttons[glfwButtons[i]];

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

      state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];

      GE::Vector2 sticks[] =
      {
         GE::Vector2
         (
            state.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
            -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]
         ),
         GE::Vector2
         (
            state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
            -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]
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
         0.5f + (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] * 0.5f),
         0.5f + (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] * 0.5f)
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
