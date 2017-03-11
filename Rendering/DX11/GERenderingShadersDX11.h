
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (DirectX 11)
//
//  --- GERenderingShadersDX11.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Rendering/GERenderingObjects.h"
#include "Rendering/GEShaderProgram.h"
#include "Core/GEObject.h"
#include "Core/GEAllocator.h"
#include <d3d11_1.h>

namespace GE { namespace Rendering
{
   class VertexShader
   {
   protected:
      ID3D11InputLayout* dxInputLayout;
      ID3D11VertexShader* dxVertexShader;

   public:
      VertexShader(const char* Filename, uint VertexElements, ID3D11Device1* DXDevice);

      ID3D11InputLayout* getInputLayout() const;
      ID3D11VertexShader* getShader() const;
   };


   class PixelShader
   {
   protected:
      ID3D11PixelShader* dxPixelShader;

   public:
      PixelShader(const char* Filename, ID3D11Device1* DXDevice);

      ID3D11PixelShader* getShader() const;
   };


   class ShaderProgramDX11 : public ShaderProgram
   {
   public:
      VertexShader* VS;
      PixelShader* PS;

      ShaderProgramDX11(const Core::ObjectName& Name)
         : ShaderProgram(Name)
         , VS(0)
         , PS(0)
      {
      }

      ~ShaderProgramDX11()
      {
         if(VS)
         {
            GEInvokeDtor(VertexShader, VS);
            Core::Allocator::free(VS);
         }

         if(PS)
         {
            GEInvokeDtor(PixelShader, PS);
            Core::Allocator::free(PS);
         }
      }
   };
}}
