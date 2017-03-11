
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (DirectX 11)
//
//  --- GERenderTextureDX11.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "GERenderTextureDX11.h"

using namespace GE;
using namespace GE::Rendering;

RenderTextureDX11::RenderTextureDX11(ID3D11Device* Device, ID3D11DeviceContext* Context, uint Width, uint Height)
   : dxContext(Context)
{
   D3D11_TEXTURE2D_DESC textureDesc;
   HRESULT result;
   D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
   D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
   D3D11_TEXTURE2D_DESC depthBufferDesc;
   D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

   // Initialize the render target texture description.
   ZeroMemory(&textureDesc, sizeof(textureDesc));

   // Setup the render target texture description.
   textureDesc.Width = Width;
   textureDesc.Height = Height;
   textureDesc.MipLevels = 1;
   textureDesc.ArraySize = 1;
   textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
   textureDesc.SampleDesc.Count = 1;
   textureDesc.Usage = D3D11_USAGE_DEFAULT;
   textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
   textureDesc.CPUAccessFlags = 0;
   textureDesc.MiscFlags = 0;

   // Create the render target texture.
   result = Device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);
   GEAssert(!FAILED(result));

   // Setup the description of the render target view.
   renderTargetViewDesc.Format = textureDesc.Format;
   renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
   renderTargetViewDesc.Texture2D.MipSlice = 0;

   // Create the render target view.
   result = Device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);
   GEAssert(!FAILED(result));

   // Setup the description of the shader resource view.
   shaderResourceViewDesc.Format = textureDesc.Format;
   shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
   shaderResourceViewDesc.Texture2D.MipLevels = 1;

   // Create the shader resource view.
   result = Device->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
   GEAssert(!FAILED(result));

   // Initialize the description of the depth buffer.
   ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

   // Set up the description of the depth buffer.
   depthBufferDesc.Width = Width;
   depthBufferDesc.Height = Height;
   depthBufferDesc.MipLevels = 1;
   depthBufferDesc.ArraySize = 1;
   depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
   depthBufferDesc.SampleDesc.Count = 1;
   depthBufferDesc.SampleDesc.Quality = 0;
   depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
   depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
   depthBufferDesc.CPUAccessFlags = 0;
   depthBufferDesc.MiscFlags = 0;

   // Create the texture for the depth buffer using the filled out description.
   result = Device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
   GEAssert(!FAILED(result));

   // Initialize the depth stencil view description.
   ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

   // Set up the depth stencil view description.
   depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
   depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
   depthStencilViewDesc.Texture2D.MipSlice = 0;

   // Create the depth stencil view.
   result = Device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
   GEAssert(!FAILED(result));

   // Setup the viewport for rendering.
   m_viewport.Width = (float)Width;
   m_viewport.Height = (float)Height;
   m_viewport.MinDepth = 0.0f;
   m_viewport.MaxDepth = 1.0f;
   m_viewport.TopLeftX = 0.0f;
   m_viewport.TopLeftY = 0.0f;
}

RenderTextureDX11::~RenderTextureDX11()
{
   if(m_depthStencilView)
   {
      m_depthStencilView->Release();
      m_depthStencilView = 0;
   }

   if(m_depthStencilBuffer)
   {
      m_depthStencilBuffer->Release();
      m_depthStencilBuffer = 0;
   }

   if(m_shaderResourceView)
   {
      m_shaderResourceView->Release();
      m_shaderResourceView = 0;
   }

   if(m_renderTargetView)
   {
      m_renderTargetView->Release();
      m_renderTargetView = 0;
   }

   if(m_renderTargetTexture)
   {
      m_renderTargetTexture->Release();
      m_renderTargetTexture = 0;
   }
}

void RenderTextureDX11::setAsRenderTarget()
{
   dxContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
   dxContext->RSSetViewports(1, &m_viewport);
}

void RenderTextureDX11::clear(const Color& ClearColor)
{
   dxContext->ClearRenderTargetView(m_renderTargetView, &ClearColor.Red);
   dxContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* RenderTextureDX11::getShaderResourceView()
{
   return m_shaderResourceView;
}
