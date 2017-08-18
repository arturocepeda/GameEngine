
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (DirectX 11)
//
//  --- GERenderTextureDX11.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GERenderingShadersDX11.h"

namespace GE { namespace Rendering
{
   class RenderTextureDX11
   {
   private:
      ID3D11DeviceContext* dxContext;

      ID3D11Texture2D* m_renderTargetTexture;
      ID3D11RenderTargetView* m_renderTargetView;
      ID3D11ShaderResourceView* m_shaderResourceView;
      ID3D11Texture2D* m_depthStencilBuffer;
      ID3D11DepthStencilView* m_depthStencilView;
      D3D11_VIEWPORT m_viewport;

   public:
      RenderTextureDX11(ID3D11Device* Device, ID3D11DeviceContext* Context, uint Width, uint Height);
      ~RenderTextureDX11();

      void setAsRenderTarget(UINT Slot);
      void clear(const Color& ClearColor);
      ID3D11ShaderResourceView* getShaderResourceView();
   };
}}
