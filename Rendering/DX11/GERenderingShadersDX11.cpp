
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (DirectX 11)
//
//  --- GERenderingShadersDX11.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "GERenderingShadersDX11.h"
#include "Core/GEDevice.h"
#include "Core/GEApplication.h"
#include "Core/GEPlatform.h"

#include <D3DCompiler.h>

#pragma comment(lib, "D3DCompiler.lib")

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Rendering;


//
//  VertexShader
//
VertexShader::VertexShader(const char* Filename, uint8_t VertexElements, const ShaderProgram::PreprocessorMacroList& Macros, ID3D11Device1* DXDevice)
   : dxInputLayout(0)
   , dxVertexShader(0)
{
   char sBuffer[64];
   sprintf(sBuffer, "Shaders\\hlsl\\%s.vsh.hlsl", Filename);

   wchar_t wsBuffer[64];
   mbstowcs(wsBuffer, sBuffer, strlen(sBuffer) + 1);

   D3D_SHADER_MACRO* dxDefines = 0;

   if(!Macros.empty())
   {
      dxDefines = new D3D_SHADER_MACRO[Macros.size() + 1];

      for(uint i = 0; i < Macros.size(); i++)
      {
         dxDefines[i].Name = Macros[i].Name;
         dxDefines[i].Definition = Macros[i].Value;
      }

      dxDefines[Macros.size()].Name = 0;
      dxDefines[Macros.size()].Definition = 0;
   }

   ID3DBlob* dxCodeBlob = 0;
   ID3DBlob* dxErrorBlob = 0;
   HRESULT hr = D3DCompileFromFile(wsBuffer, dxDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &dxCodeBlob, &dxErrorBlob);

   if(!Macros.empty())
   {
      delete[] dxDefines;
      dxDefines = 0;
   }

   if(FAILED(hr))
   {
      if(dxErrorBlob)
      {
         Device::log("ERROR: Vertex Shader could not be loaded\n%s", (char*)dxErrorBlob->GetBufferPointer());
         dxErrorBlob->Release();
      }

      return;
   }

   char* pShaderByteCodeData = (char*)dxCodeBlob->GetBufferPointer();
   uint iShaderByteCodeSize = (uint)dxCodeBlob->GetBufferSize();

   GEAssert(pShaderByteCodeData);
   GEAssert(iShaderByteCodeSize > 0);

   DXDevice->CreateVertexShader(pShaderByteCodeData, iShaderByteCodeSize, 0, &dxVertexShader);

   createInputLayout(pShaderByteCodeData, iShaderByteCodeSize, VertexElements, DXDevice);
}

VertexShader::VertexShader(const char* ByteCode, uint ByteCodeSize, uint VertexElements, ID3D11Device1* DXDevice)
   : dxInputLayout(0)
   , dxVertexShader(0)
{
   DXDevice->CreateVertexShader(ByteCode, ByteCodeSize, 0, &dxVertexShader);

   createInputLayout(ByteCode, ByteCodeSize, VertexElements, DXDevice);
}

void VertexShader::createInputLayout(const char* pByteCode, uint iByteCodeSize, uint8_t iVertexElements, ID3D11Device1* dxDevice)
{
   GESTLVector(D3D11_INPUT_ELEMENT_DESC) dxVertexLayout;

   // vertex elements
   D3D11_INPUT_ELEMENT_DESC dxInputElementDesc;
   dxInputElementDesc.SemanticIndex = 0;
   dxInputElementDesc.InputSlot = 0;
   dxInputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
   dxInputElementDesc.InstanceDataStepRate = 0;

   uint iByteOffset = 0;

   if(GEHasFlag(iVertexElements, VertexElementsBitMask::WVP))
   {
      dxInputElementDesc.SemanticName = "WORLDVIEWPROJECTION";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

      for(uint i = 0; i < 4; i++)
      {
         dxInputElementDesc.AlignedByteOffset = iByteOffset;
         dxInputElementDesc.SemanticIndex = i;
         dxVertexLayout.push_back(dxInputElementDesc);
         iByteOffset += 4 * sizeof(float);
      }

      dxInputElementDesc.SemanticIndex = 0;
   }

   if(GEHasFlag(iVertexElements, VertexElementsBitMask::Position))
   {
      dxInputElementDesc.SemanticName = "POSITION";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
      dxInputElementDesc.AlignedByteOffset = iByteOffset;

      dxVertexLayout.push_back(dxInputElementDesc);
      iByteOffset += 3 * sizeof(float);
   }

   if(GEHasFlag(iVertexElements, VertexElementsBitMask::Color))
   {
      dxInputElementDesc.SemanticName = "COLOR";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      dxInputElementDesc.AlignedByteOffset = iByteOffset;

      dxVertexLayout.push_back(dxInputElementDesc);
      iByteOffset += 4 * sizeof(float);
   }

   if(GEHasFlag(iVertexElements, VertexElementsBitMask::Normal))
   {
      dxInputElementDesc.SemanticName = "NORMAL";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
      dxInputElementDesc.AlignedByteOffset = iByteOffset;

      dxVertexLayout.push_back(dxInputElementDesc);
      iByteOffset += 3 * sizeof(float);
   }

   if(GEHasFlag(iVertexElements, VertexElementsBitMask::TexCoord))
   {
      dxInputElementDesc.SemanticName = "TEXCOORD";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
      dxInputElementDesc.AlignedByteOffset = iByteOffset;

      dxVertexLayout.push_back(dxInputElementDesc);
      iByteOffset += 2 * sizeof(float);
   }

   dxDevice->CreateInputLayout(&dxVertexLayout[0], (UINT)dxVertexLayout.size(), pByteCode, iByteCodeSize, &dxInputLayout);
}

ID3D11InputLayout* VertexShader::getInputLayout() const
{
   return dxInputLayout;
}

ID3D11VertexShader* VertexShader::getShader() const
{
   return dxVertexShader;
}


//
//  PixelShader
//
PixelShader::PixelShader(const char* Filename, const ShaderProgram::PreprocessorMacroList& Macros, ID3D11Device1* DXDevice)
   : dxPixelShader(0)
{
   char sBuffer[64];
   sprintf(sBuffer, "Shaders\\hlsl\\%s.psh.hlsl", Filename);

   wchar_t wsBuffer[64];
   mbstowcs(wsBuffer, sBuffer, strlen(sBuffer) + 1);

   D3D_SHADER_MACRO* dxDefines = 0;

   if(!Macros.empty())
   {
      dxDefines = new D3D_SHADER_MACRO[Macros.size() + 1];

      for(uint i = 0; i < Macros.size(); i++)
      {
         dxDefines[i].Name = Macros[i].Name;
         dxDefines[i].Definition = Macros[i].Value;
      }

      dxDefines[Macros.size()].Name = 0;
      dxDefines[Macros.size()].Definition = 0;
   }

   ID3DBlob* dxCodeBlob = 0;
   ID3DBlob* dxErrorBlob = 0;
   HRESULT hr = D3DCompileFromFile(wsBuffer, dxDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &dxCodeBlob, &dxErrorBlob);

   if(!Macros.empty())
   {
      delete[] dxDefines;
      dxDefines = 0;
   }

   if(FAILED(hr))
   {
      if(dxErrorBlob)
      {
         Device::log("ERROR: Pixel Shader could not be loaded\n%s", (char*)dxErrorBlob->GetBufferPointer());
         dxErrorBlob->Release();
      }

      return;
   }

   char* pShaderByteCodeData = (char*)dxCodeBlob->GetBufferPointer();
   uint iShaderByteCodeSize = (uint)dxCodeBlob->GetBufferSize();

   GEAssert(pShaderByteCodeData);
   GEAssert(iShaderByteCodeSize > 0);

   DXDevice->CreatePixelShader(pShaderByteCodeData, iShaderByteCodeSize, 0, &dxPixelShader);
}

PixelShader::PixelShader(const char* ByteCode, uint ByteCodeSize, ID3D11Device1* DXDevice)
{
   DXDevice->CreatePixelShader(ByteCode, ByteCodeSize, 0, &dxPixelShader);
}

ID3D11PixelShader* PixelShader::getShader() const
{
   return dxPixelShader;
}
