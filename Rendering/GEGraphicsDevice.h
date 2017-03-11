
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEGraphicsDevice.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEUtils.h"

namespace GE { namespace Rendering
{
   class GraphicsDevice : private Core::NonCopyable
   {
   public:
      GraphicsDevice();
      virtual ~GraphicsDevice();

      virtual void* getNativePointer() = 0;
   };
}}
