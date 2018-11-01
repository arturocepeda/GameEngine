
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEState.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Input/GEInputSystem.h"
#include "Core/GEObject.h"

#define GE_KEY_UP          38
#define GE_KEY_DOWN        40
#define GE_KEY_LEFT        37
#define GE_KEY_RIGHT       39
        
#define GE_KEY_ENTER       13
#define GE_KEY_ESC         27
#define GE_KEY_BACKSPACE   8
#define GE_KEY_SHIFT       16
#define GE_KEY_TAB         9
#define GE_KEY_CAPSLOCK    20
        
#define GE_KEY_INSERT      45
#define GE_KEY_DELETE      46
#define GE_KEY_HOME        36
#define GE_KEY_END         35
#define GE_KEY_PAGEUP      33
#define GE_KEY_PAGEDOWN    34

#define GE_KEY_PLUS        107
#define GE_KEY_MINUS       109

namespace GE { namespace Core
{
   enum class StateType
   {
      Exclusive,
      Modal
   };


   class State : public Object, public Input::InputListener
   {
   protected:
      void* pGlobalData;
      StateType eStateType;

   public:
      State(const ObjectName& Name, void* GlobalData);
      virtual ~State();

      StateType getStateType() const;

      virtual void activate() = 0;
      virtual void update() = 0;
      virtual void deactivate() = 0;

      virtual void pause();
      virtual void resume();
   };
}}
