
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
VertexShader::VertexShader(const char* Filename, uint VertexElements, ID3D11Device1* DXDevice)
   : dxInputLayout(0)
   , dxVertexShader(0)
{
   char* pShaderByteCodeData = 0;
   uint iShaderByteCodeSize = 0;

   char sBuffer[64];
   ID3DBlob* dxCodeBlob = 0;
   ContentData cVertexShader;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      sprintf(sBuffer, "Shaders\\hlsl\\%s.vsh.hlsl", Filename);

      wchar_t wsBuffer[64];
      mbstowcs(wsBuffer, sBuffer, strlen(sBuffer) + 1);

      ID3DBlob* dxErrorBlob = 0;
      HRESULT hr = D3DCompileFromFile(wsBuffer, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &dxCodeBlob, &dxErrorBlob);

      if(FAILED(hr))
      {
         if(dxErrorBlob)
         {
            OutputDebugStringA((char*)dxErrorBlob->GetBufferPointer());
            dxErrorBlob->Release();
         }

         GEAssert(false);
      }

      pShaderByteCodeData = (char*)dxCodeBlob->GetBufferPointer();
      iShaderByteCodeSize = (uint)dxCodeBlob->GetBufferSize();
   }
   else
   {
      sprintf(sBuffer, "%s.vsh", Filename);

      Device::readContentFile(ContentType::GenericBinaryData, "Shaders\\hlsl", sBuffer, "cso", &cVertexShader);
      GEAssert(cVertexShader.getDataSize() > 0);

      pShaderByteCodeData = cVertexShader.getData();
      iShaderByteCodeSize = cVertexShader.getDataSize();
   }

   GEAssert(pShaderByteCodeData);
   GEAssert(iShaderByteCodeSize > 0);

   DXDevice->CreateVertexShader(pShaderByteCodeData, iShaderByteCodeSize, 0, &dxVertexShader);

   GESTLVector(D3D11_INPUT_ELEMENT_DESC) dxVertexLayout;

   // vertex elements
   D3D11_INPUT_ELEMENT_DESC dxInputElementDesc;
   dxInputElementDesc.SemanticIndex = 0;
   dxInputElementDesc.InputSlot = 0;
   dxInputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
   dxInputElementDesc.InstanceDataStepRate = 0;

   uint iByteOffset = 0;

   if(VertexElements & VE_WVP)
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

   if(VertexElements & VE_Position)
   {
      dxInputElementDesc.SemanticName = "POSITION";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
      dxInputElementDesc.AlignedByteOffset = iByteOffset;

      dxVertexLayout.push_back(dxInputElementDesc);
      iByteOffset += 3 * sizeof(float);
   }

   if(VertexElements & VE_Color)
   {
      dxInputElementDesc.SemanticName = "COLOR";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      dxInputElementDesc.AlignedByteOffset = iByteOffset;

      dxVertexLayout.push_back(dxInputElementDesc);
      iByteOffset += 4 * sizeof(float);
   }

   if(VertexElements & VE_Normal)
   {
      dxInputElementDesc.SemanticName = "NORMAL";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
      dxInputElementDesc.AlignedByteOffset = iByteOffset;

      dxVertexLayout.push_back(dxInputElementDesc);
      iByteOffset += 3 * sizeof(float);
   }

   if(VertexElements & VE_TexCoord)
   {
      dxInputElementDesc.SemanticName = "TEXCOORD";
      dxInputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
      dxInputElementDesc.AlignedByteOffset = iByteOffset;

      dxVertexLayout.push_back(dxInputElementDesc);
      iByteOffset += 2 * sizeof(float);
   }

   DXDevice->CreateInputLayout(&dxVertexLayout[0], (UINT)dxVertexLayout.size(),
      pShaderByteCodeData, iShaderByteCodeSize, &dxInputLayout);
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
PixelShader::PixelShader(const char* Filename, ID3D11Device1* DXDevice)
   : dxPixelShader(0)
{
   char* pShaderByteCodeData = 0;
   uint iShaderByteCodeSize = 0;

   char sBuffer[64];
   ID3DBlob* dxCodeBlob = 0;
   ContentData cPixelShader;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      sprintf(sBuffer, "Shaders\\hlsl\\%s.psh.hlsl", Filename);

      wchar_t wsBuffer[64];
      mbstowcs(wsBuffer, sBuffer, strlen(sBuffer) + 1);

      ID3DBlob* dxErrorBlob = 0;
      HRESULT hr = D3DCompileFromFile(wsBuffer, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &dxCodeBlob, &dxErrorBlob);

      if(FAILED(hr) && dxErrorBlob)
      {
         OutputDebugStringA((char*)dxErrorBlob->GetBufferPointer());
         dxErrorBlob->Release();
         GEAssert(false);
      }

      pShaderByteCodeData = (char*)dxCodeBlob->GetBufferPointer();
      iShaderByteCodeSize = (uint)dxCodeBlob->GetBufferSize();
   }
   else
   {
      sprintf(sBuffer, "%s.psh", Filename);

      Device::readContentFile(ContentType::GenericBinaryData, "Shaders\\hlsl", sBuffer, "cso", &cPixelShader);
      GEAssert(cPixelShader.getDataSize() > 0);

      pShaderByteCodeData = cPixelShader.getData();
      iShaderByteCodeSize = cPixelShader.getDataSize();
   }

   GEAssert(pShaderByteCodeData);
   GEAssert(iShaderByteCodeSize > 0);

   DXDevice->CreatePixelShader(pShaderByteCodeData, iShaderByteCodeSize, 0, &dxPixelShader);
}

ID3D11PixelShader* PixelShader::getShader() const
{
   return dxPixelShader;
}
