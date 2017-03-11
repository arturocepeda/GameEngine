
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (OpenGL ES)
//
//  --- GERenderSystemES20.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Rendering/GERenderSystem.h"
#include "Rendering/GERenderingObjects.h"
#include "GERenderingShadersES20.h"
#include "Core/GEUtils.h"
#include <vector>

namespace GE { namespace Rendering
{
   class RenderSystemES20 : public RenderSystem
   {
   protected:
      void createBuffers();
      void releaseBuffers();
    
      void linkProgram(ShaderProgramES20* cProgram);
      bool checkProgram(ShaderProgramES20* cProgram);
      void getUniformsLocation(ShaderProgramES20* cProgram);

   public:
      RenderSystemES20();
      ~RenderSystemES20();

      void setVertexDeclaration(const RenderOperation& cRenderOperation);
      void attachShaders(ShaderProgramES20* cProgram);
   };
}}
