
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
   class InputSystem : public Core::Singleton<InputSystem>
   {
   public:
      InputSystem();
      ~InputSystem();

      void inputKeyPress(char Key);
      void inputKeyRelease(char Key);

      void inputMouse(const Vector2& Point);
      void inputMouseLeftButton();
      void inputMouseRightButton();
      void inputMouseWheel(int Delta);

      void inputTouchBegin(int ID, const Vector2& Point);
      void inputTouchMove(int ID, const Vector2& PreviousPoint, const Vector2& CurrentPoint);
      void inputTouchEnd(int ID, const Vector2& Point);

      void updateAccelerometerStatus(const Vector3& Status);
   };
}}
