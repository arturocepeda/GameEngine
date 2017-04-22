
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (DirectX 11)
//
//  --- GERenderSystemDX11.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GERenderSystemDX11.h"
#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Core/GEProfiler.h"
#include "Core/GEApplication.h"
#include "Content/GEImageData.h"
#include "Entities/GEEntity.h"
#include "Entities/GEComponentUIElement.h"
#include "pugixml/pugixml.hpp"

using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Entities;
using namespace GE::Rendering;
using namespace Microsoft::WRL;

ComPtr<ID3D11Device1> dxDevice;
ComPtr<ID3D11DeviceContext1> dxContext;
ComPtr<IDXGISwapChain1> dxSwapChain;
ComPtr<ID3D11RenderTargetView> dxRenderTargetView;
ComPtr<ID3D11DepthStencilView> dxDepthStencilView;

D3D_FEATURE_LEVEL dxFeatureLevel;

#if defined(GE_PLATFORM_WP8)
Windows::Foundation::Rect cWindowBounds;
Platform::Agile<Windows::UI::Core::CoreWindow> cWindow;
#endif

// states
ID3D11BlendState* dxBlendStateNone = 0;
ID3D11BlendState* dxBlendStateAlpha = 0;
ID3D11BlendState* dxBlendStateAdditive = 0;
ID3D11SamplerState* dxSamplerStateClamp = 0;
ID3D11SamplerState* dxSamplerStateWrap = 0;
ID3D11DepthStencilState* dxDepthStencilStateNoDepth = 0;
ID3D11DepthStencilState* dxDepthStencilStateTestOnly = 0;
ID3D11DepthStencilState* dxDepthStencilStateTestAndWrite = 0;
ID3D11RasterizerState* dxRasterizerStateSolidCullBack = 0;
ID3D11RasterizerState* dxRasterizerStateSolidCullFront = 0;
ID3D11RasterizerState* dxRasterizerStateSolidNoCull = 0;
ID3D11RasterizerState* dxRasterizerStateWireFrame = 0;

// buffers
ID3D11Buffer* dxConstantBufferTransform = 0;
ID3D11Buffer* dxConstantBufferMaterial = 0;
ID3D11Buffer* dxConstantBufferLighting = 0;
ID3D11Buffer* dxConstantBufferVertexParameters = 0;
ID3D11Buffer* dxConstantBufferFragmentParameters = 0;

ID3D11Buffer* pCurrentVertexBuffer = 0;
ID3D11Buffer* pCurrentIndexBuffer = 0;

ShaderConstantsTransform sShaderConstantsTransform;
ShaderConstantsMaterial sShaderConstantsMaterial;
ShaderConstantsLighting sShaderConstantsLighting;

RenderTextureDX11* cShadowMap;


#if defined(GE_PLATFORM_WP8)
RenderSystemDX11::RenderSystemDX11(Windows::UI::Core::CoreWindow^ window)
   : RenderSystem(nullptr, false, Device::getScreenWidth(), Device::getScreenHeight())
{
   cWindow = window;
#else
RenderSystemDX11::RenderSystemDX11(HWND WindowHandle, bool Windowed)
   : RenderSystem(WindowHandle, Windowed, Device::getScreenWidth(), Device::getScreenHeight())
{
#endif
   RenderSystemDX11Helper::createDeviceResources();
   RenderSystemDX11Helper::createWindowSizeDependentResources();

   pDevice = dxDevice.Get();

   createBuffers();
   createStates();
   loadShaders();
   loadDefaultRenderingResources();

   // create render texture for shadow mapping
   cShadowMap = Allocator::alloc<RenderTextureDX11>();
   GEInvokeCtor(RenderTextureDX11, cShadowMap)(dxDevice.Get(), dxContext.Get(), ShadowMapSize, ShadowMapSize);

   // set default render target
   dxContext->OMSetRenderTargets(1, dxRenderTargetView.GetAddressOf(), dxDepthStencilView.Get());

   // set default viewport
   CD3D11_VIEWPORT dxViewport(0.0f, 0.0f, (float)Device::ScreenWidth, (float)Device::ScreenHeight);
   dxContext->RSSetViewports(1, &dxViewport);
}

RenderSystemDX11::~RenderSystemDX11()
{
   GEInvokeDtor(RenderTextureDX11, cShadowMap);
   Allocator::free(cShadowMap);

   releaseShaders();
   releaseStates();
   releaseBuffers();
}

void RenderSystem::loadTexture(PreloadedTexture* cPreloadedTexture)
{
   // create DX texture
   D3D11_TEXTURE2D_DESC desc;
   desc.Width = cPreloadedTexture->Data->getWidth();
   desc.Height = cPreloadedTexture->Data->getHeight();
   desc.MipLevels = 1;
   desc.ArraySize = 1;
   desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   desc.SampleDesc.Count = 1;
   desc.SampleDesc.Quality = 0;
   desc.Usage = D3D11_USAGE_DEFAULT;
   desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
   desc.CPUAccessFlags = 0;
   desc.MiscFlags = 0;

   D3D11_SUBRESOURCE_DATA initData;
   char* sBuffer = 0;

   // the image uses alpha channel
   if(cPreloadedTexture->Data->getBytesPerPixel() == 4)
   {
      initData.pSysMem = cPreloadedTexture->Data->getData();
      initData.SysMemPitch =
         static_cast<UINT>(cPreloadedTexture->Data->getWidth() * cPreloadedTexture->Data->getBytesPerPixel());
      initData.SysMemSlicePitch =
         static_cast<UINT>(cPreloadedTexture->Data->getWidth() * cPreloadedTexture->Data->getHeight() * cPreloadedTexture->Data->getBytesPerPixel());
   }
   // the image does not use alpha channel
   else
   {
      sBuffer = Allocator::alloc<char>(cPreloadedTexture->Data->getWidth() * cPreloadedTexture->Data->getHeight() * 4);
      GE::uint iBufferPos = 0;

      char* sImageData = cPreloadedTexture->Data->getData();
      GE::uint iImageDataPos = 0;

      for(int y = 0; y < cPreloadedTexture->Data->getHeight(); y++)
      {
         for(int x = 0; x < cPreloadedTexture->Data->getWidth(); x++)
         {
            sBuffer[iBufferPos++] = sImageData[iImageDataPos++];
            sBuffer[iBufferPos++] = sImageData[iImageDataPos++];
            sBuffer[iBufferPos++] = sImageData[iImageDataPos++];
            sBuffer[iBufferPos++] = (char)0xff;
         }
      }

      initData.pSysMem = sBuffer;
      initData.SysMemPitch = static_cast<UINT>(cPreloadedTexture->Data->getWidth() * 4);
      initData.SysMemSlicePitch = static_cast<UINT>(cPreloadedTexture->Data->getWidth() * cPreloadedTexture->Data->getHeight() * 4);
   }

   ID3D11Texture2D* dxTexture = nullptr;
   dxDevice->CreateTexture2D(&desc, &initData, &dxTexture);
   ID3D11ShaderResourceView* dxTextureResourceView = nullptr;
   dxDevice->CreateShaderResourceView(dxTexture, NULL, &dxTextureResourceView);

   if(sBuffer)
      Allocator::free(sBuffer);

   cPreloadedTexture->Data->unload();

   GEInvokeDtor(ImageData, cPreloadedTexture->Data);
   Allocator::free(cPreloadedTexture->Data);
   cPreloadedTexture->Data = 0;

   cPreloadedTexture->Tex->setHandler(dxTextureResourceView);
   mTextures.add(cPreloadedTexture->Tex);
}

void RenderSystem::loadRenderingData(const GeometryData& sData, GPUBufferPair& sBuffers, GE::uint)
{
   GEProfilerMarker("RenderSystem::loadRenderingData()");

   uint iVertexDataSize = sData.NumVertices * sData.VertexStride;
   uint iIndicesSize = sData.NumIndices * sizeof(ushort);

   D3D11_MAPPED_SUBRESOURCE dxResource;
   ID3D11Buffer* dxVertexBuffer = static_cast<ID3D11Buffer*>(sBuffers.VertexBuffer);
   dxContext->Map(dxVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &dxResource);
   memcpy((char*)dxResource.pData + sBuffers.CurrentVertexBufferOffset, sData.VertexData, iVertexDataSize);
   dxContext->Unmap(dxVertexBuffer, 0);

   ID3D11Buffer* dxIndexBuffer = static_cast<ID3D11Buffer*>(sBuffers.IndexBuffer);
   dxContext->Map(dxIndexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &dxResource);
   memcpy((char*)dxResource.pData + sBuffers.CurrentIndexBufferOffset, sData.Indices, iIndicesSize);
   dxContext->Unmap(dxIndexBuffer, 0);

   sBuffers.CurrentVertexBufferOffset += iVertexDataSize;
   sBuffers.CurrentIndexBufferOffset += iIndicesSize;
}

void RenderSystemDX11Helper::createDeviceResources()
{
   UINT dxDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
   dxDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

   D3D_FEATURE_LEVEL dxFeatureLevels[] = 
   {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3
   };

   ComPtr<ID3D11Device> pDXDevice;
   ComPtr<ID3D11DeviceContext> pDXContext;

   D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, dxDeviceFlags, dxFeatureLevels, ARRAYSIZE(dxFeatureLevels),
                     D3D11_SDK_VERSION, &pDXDevice, &dxFeatureLevel, &pDXContext);

   pDXDevice.As(&dxDevice);
   pDXContext.As(&dxContext);
}

void RenderSystemDX11::createBuffers()
{
   // vertex/index buffers
   for(uint i = 0; i < GeometryGroup::Count; i++)
   {
      // vertex buffer
      ID3D11Buffer* dxVertexBuffer = 0;

      CD3D11_BUFFER_DESC dxVertexBufferDesc;
      dxVertexBufferDesc.ByteWidth = VertexBufferSize;
      dxVertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
      dxVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      dxVertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      dxVertexBufferDesc.MiscFlags = 0;
      dxVertexBufferDesc.StructureByteStride = 0;
      dxDevice->CreateBuffer(&dxVertexBufferDesc, 0, &dxVertexBuffer);

      GEAssert(dxVertexBuffer);

      sGPUBufferPairs[i].VertexBuffer = dxVertexBuffer;

      // index buffer
      ID3D11Buffer* dxIndexBuffer = 0;

      CD3D11_BUFFER_DESC dxIndexBufferDesc;
      dxIndexBufferDesc.ByteWidth = IndexBufferSize;
      dxIndexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
      dxIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
      dxIndexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      dxIndexBufferDesc.MiscFlags = 0;
      dxIndexBufferDesc.StructureByteStride = 0;
      dxDevice->CreateBuffer(&dxIndexBufferDesc, 0, &dxIndexBuffer);

      GEAssert(dxIndexBuffer);

      sGPUBufferPairs[i].IndexBuffer = dxIndexBuffer;
   }

   // constant buffers
   CD3D11_BUFFER_DESC dxConstantBufferDesc(sizeof(ShaderConstantsTransform), D3D11_BIND_CONSTANT_BUFFER);
   dxDevice->CreateBuffer(&dxConstantBufferDesc, nullptr, &dxConstantBufferTransform);
   GEAssert(dxConstantBufferTransform);

   dxConstantBufferDesc.ByteWidth = sizeof(ShaderConstantsMaterial);
   dxDevice->CreateBuffer(&dxConstantBufferDesc, nullptr, &dxConstantBufferMaterial);
   GEAssert(dxConstantBufferMaterial);

   dxConstantBufferDesc.ByteWidth = sizeof(ShaderConstantsLighting);
   dxDevice->CreateBuffer(&dxConstantBufferDesc, nullptr, &dxConstantBufferLighting);
   GEAssert(dxConstantBufferLighting);

   dxConstantBufferDesc.ByteWidth = Material::ConstantBufferSize;
   dxDevice->CreateBuffer(&dxConstantBufferDesc, nullptr, &dxConstantBufferVertexParameters);
   GEAssert(dxConstantBufferVertexParameters);

   dxDevice->CreateBuffer(&dxConstantBufferDesc, nullptr, &dxConstantBufferFragmentParameters);
   GEAssert(dxConstantBufferFragmentParameters);
}

void RenderSystemDX11::createStates()
{
   // blend states
   D3D11_BLEND_DESC dxBlendDesc;
   ZeroMemory(&dxBlendDesc, sizeof(D3D11_BLEND_DESC));
   dxBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
   dxDevice->CreateBlendState(&dxBlendDesc, &dxBlendStateNone);

   dxBlendDesc.RenderTarget[0].BlendEnable = TRUE;
   dxBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
   dxBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
   dxBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
   dxBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
   dxBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
   dxBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
   dxDevice->CreateBlendState(&dxBlendDesc, &dxBlendStateAlpha);

   dxBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
   dxBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
   dxBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
   dxBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
   dxBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
   dxBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
   dxDevice->CreateBlendState(&dxBlendDesc, &dxBlendStateAdditive);

   // sampler states
   D3D11_SAMPLER_DESC dxSamplerDesc;
   ZeroMemory(&dxSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
   dxSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   dxSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
   dxSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
   dxSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
   dxSamplerDesc.MipLODBias = 0.0f;
   dxSamplerDesc.MaxAnisotropy = 1;
   dxSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
   dxSamplerDesc.BorderColor[0] = 1.0f;
   dxSamplerDesc.BorderColor[1] = 1.0f;
   dxSamplerDesc.BorderColor[2] = 1.0f;
   dxSamplerDesc.BorderColor[3] = 1.0f;
   dxSamplerDesc.MinLOD = -FLT_MAX;
   dxSamplerDesc.MaxLOD = FLT_MAX;
   dxDevice->CreateSamplerState(&dxSamplerDesc, &dxSamplerStateClamp);

   dxSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
   dxSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
   dxSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
   dxSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
   dxSamplerDesc.BorderColor[0] = 0.0f;
   dxSamplerDesc.BorderColor[1] = 0.0f;
   dxSamplerDesc.BorderColor[2] = 0.0f;
   dxSamplerDesc.BorderColor[3] = 0.0f;
   dxSamplerDesc.MinLOD = 0.0f;
   dxSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
   dxDevice->CreateSamplerState(&dxSamplerDesc, &dxSamplerStateWrap);

   // depth stencil states
   D3D11_DEPTH_STENCIL_DESC dxDepthStencilDesc;
   ZeroMemory(&dxDepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
   dxDepthStencilDesc.DepthEnable = FALSE;
   dxDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
   dxDepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
   dxDepthStencilDesc.StencilEnable = FALSE;
   dxDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
   dxDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
   dxDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
   dxDevice->CreateDepthStencilState(&dxDepthStencilDesc, &dxDepthStencilStateNoDepth);

   dxDepthStencilDesc.DepthEnable = TRUE;
   dxDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
   dxDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
   dxDepthStencilDesc.StencilEnable = TRUE;
   dxDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
   dxDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
   dxDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INCR;
   dxDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
   dxDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
   dxDevice->CreateDepthStencilState(&dxDepthStencilDesc, &dxDepthStencilStateTestOnly);

   dxDepthStencilDesc.DepthEnable = TRUE;
   dxDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
   dxDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
   dxDepthStencilDesc.StencilEnable = TRUE;
   dxDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
   dxDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
   dxDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
   dxDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INCR;
   dxDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
   dxDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
   dxDevice->CreateDepthStencilState(&dxDepthStencilDesc, &dxDepthStencilStateTestAndWrite);

   dxContext->OMSetDepthStencilState(dxDepthStencilStateNoDepth, 0);

   // rasterizer states
   D3D11_RASTERIZER_DESC dxRasterizerDesc;
   ZeroMemory(&dxRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
   dxRasterizerDesc.AntialiasedLineEnable = false;
   dxRasterizerDesc.CullMode = D3D11_CULL_BACK;
   dxRasterizerDesc.DepthBias = 0;
   dxRasterizerDesc.DepthBiasClamp = 0.0f;
   dxRasterizerDesc.DepthClipEnable = true;
   dxRasterizerDesc.FillMode = D3D11_FILL_SOLID;
   dxRasterizerDesc.FrontCounterClockwise = true;
   dxRasterizerDesc.MultisampleEnable = false;
   dxRasterizerDesc.ScissorEnable = false;
   dxRasterizerDesc.SlopeScaledDepthBias = 0.0f;
   dxDevice->CreateRasterizerState(&dxRasterizerDesc, &dxRasterizerStateSolidCullBack);

   dxRasterizerDesc.CullMode = D3D11_CULL_FRONT;
   dxDevice->CreateRasterizerState(&dxRasterizerDesc, &dxRasterizerStateSolidCullFront);

   dxRasterizerDesc.CullMode = D3D11_CULL_NONE;
   dxDevice->CreateRasterizerState(&dxRasterizerDesc, &dxRasterizerStateSolidNoCull);

   ZeroMemory(&dxRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
   dxRasterizerDesc.CullMode = D3D11_CULL_NONE;
   dxRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
   dxDevice->CreateRasterizerState(&dxRasterizerDesc, &dxRasterizerStateWireFrame);

   dxContext->RSSetState(dxRasterizerStateSolidCullBack);
}

#if defined(GE_PLATFORM_WP8)
float dipsToPixels(float fDips)
{
   static const float DipsPerInch = 96.0f;
   return floor(fDips * Windows::Graphics::Display::DisplayProperties::LogicalDpi / DipsPerInch + 0.5f);
}
#endif

void RenderSystemDX11Helper::createWindowSizeDependentResources()
{
   float fRenderTargetWidth = (float)Device::ScreenWidth;
   float fRenderTargetHeight = (float)Device::ScreenHeight;

#if defined(GE_PLATFORM_WP8)
   cWindowBounds = cWindow->Bounds;

   fRenderTargetWidth = dipsToPixels(cWindowBounds.Width);
   fRenderTargetHeight = dipsToPixels(cWindowBounds.Height);
#endif

   DXGI_SWAP_CHAIN_DESC1 dxSwapChainDesc;
   ZeroMemory(&dxSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
   dxSwapChainDesc.Width = static_cast<UINT>(fRenderTargetWidth);
   dxSwapChainDesc.Height = static_cast<UINT>(fRenderTargetHeight);
   dxSwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
   dxSwapChainDesc.Stereo = false;
   dxSwapChainDesc.SampleDesc.Count = 1;
   dxSwapChainDesc.SampleDesc.Quality = 0;
   dxSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   dxSwapChainDesc.BufferCount = 1;
   dxSwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
   dxSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
   dxSwapChainDesc.Flags = 0;

   ComPtr<IDXGIDevice1> dxgiDevice;
   dxDevice.As(&dxgiDevice);

   ComPtr<IDXGIAdapter> dxgiAdapter;
   dxgiDevice->GetAdapter(&dxgiAdapter);

   ComPtr<IDXGIFactory2> dxgiFactory;
   dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);

#if defined(GE_PLATFORM_WP8)
   Windows::UI::Core::CoreWindow^ pWindow = cWindow.Get();
   dxgiFactory->CreateSwapChainForCoreWindow(dxDevice.Get(), reinterpret_cast<IUnknown*>(pWindow), &dxSwapChainDesc, nullptr, &dxSwapChain);
#else
   DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxSwapChainFullScreenDesc;
   ZeroMemory(&dxSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
   DXGI_RATIONAL sRefreshRate = { 60, 1 };
   dxSwapChainFullScreenDesc.RefreshRate = sRefreshRate;
   dxSwapChainFullScreenDesc.Windowed = RenderSystem::getInstance()->getWindowedMode();

   const void* pWindow = RenderSystem::getInstance()->getWindowHandler();
   dxgiFactory->CreateSwapChainForHwnd(dxDevice.Get(), (HWND)pWindow, &dxSwapChainDesc, &dxSwapChainFullScreenDesc, nullptr, &dxSwapChain);
#endif
      
   // only render after each VSync
   dxgiDevice->SetMaximumFrameLatency(1);

   ComPtr<ID3D11Texture2D> pBackBuffer;
   dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
   dxDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &dxRenderTargetView);

   CD3D11_TEXTURE2D_DESC dxDepthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, (UINT)fRenderTargetWidth, (UINT)fRenderTargetHeight,
      1, 1, D3D11_BIND_DEPTH_STENCIL);

   ComPtr<ID3D11Texture2D> pDepthStencil;
   dxDevice->CreateTexture2D(&dxDepthStencilDesc, nullptr, &pDepthStencil);

   CD3D11_DEPTH_STENCIL_VIEW_DESC dxDepthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
   dxDevice->CreateDepthStencilView(pDepthStencil.Get(), &dxDepthStencilViewDesc, &dxDepthStencilView);
}

void RenderSystemDX11Helper::updateForWindowSizeChange()
{
#if defined(GE_PLATFORM_WP8)
   if(cWindow->Bounds.Width != cWindowBounds.Width || cWindow->Bounds.Height != cWindowBounds.Height)
   {
      ID3D11RenderTargetView* nullViews[] = {nullptr};
      dxContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
      dxRenderTargetView = nullptr;
      dxDepthStencilView = nullptr;
      dxContext->Flush();
      createWindowSizeDependentResources();
   }
#endif
}

void RenderSystem::loadShaders()
{
   ContentData cShadersData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Shaders", "shaders", "xml", &cShadersData);

      pugi::xml_document xml;
      xml.load_buffer(cShadersData.getData(), cShadersData.getDataSize());
      const pugi::xml_node& xmlShaders = xml.child("Shaders");

      for(const pugi::xml_node& xmlShader : xmlShaders.children("Shader"))
      {
         const char* sShaderProgramName = xmlShader.attribute("name").value();
         ShaderProgramDX11* cShaderProgram = static_cast<ShaderProgramDX11*>(mShaderPrograms.get(sShaderProgramName));
         bool bReload = cShaderProgram != 0;

         if(bReload)
         {
            GEInvokeDtor(ShaderProgramDX11, cShaderProgram);
         }
         else
         {
            cShaderProgram = Allocator::alloc<ShaderProgramDX11>();
         }

         GEInvokeCtor(ShaderProgramDX11, cShaderProgram)(sShaderProgramName);

         const char* sVertexSource = xmlShader.attribute("vertexSource").value();
         const char* sFragmentSource = xmlShader.attribute("fragmentSource").value();
         uint iVertexElementsMask = cShaderProgram->getVertexElementsMask(xmlShader);

         cShaderProgram->VS = Allocator::alloc<VertexShader>();
         GEInvokeCtor(VertexShader, cShaderProgram->VS)(sVertexSource, iVertexElementsMask, dxDevice.Get());
         cShaderProgram->PS = Allocator::alloc<PixelShader>();
         GEInvokeCtor(PixelShader, cShaderProgram->PS)(sFragmentSource, dxDevice.Get());

         cShaderProgram->parseParameters(xmlShader);
         cShaderProgram->loadFromXml(xmlShader);

         if(!bReload)
         {
            mShaderPrograms.add(cShaderProgram);
         }
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Shaders", "Shaders.hlsl", "ge", &cShadersData);
      ContentDataMemoryBuffer sMemoryBuffer(cShadersData);
      std::istream sStream(&sMemoryBuffer);

      uint iShadersCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();
      GESTLVector(char) vShaderByteCode;

      for(uint i = 0; i < iShadersCount; i++)
      {
         ObjectName cShaderProgramName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();

         ShaderProgramDX11* cShaderProgram = Allocator::alloc<ShaderProgramDX11>();
         GEInvokeCtor(ShaderProgramDX11, cShaderProgram)(cShaderProgramName);

         cShaderProgram->parseParameters(sStream);
         cShaderProgram->loadFromStream(sStream);

         uint iVertexElementsMask = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

         uint iShaderByteCodeSize = Value::fromStream(ValueType::UInt, sStream).getAsUInt();
         vShaderByteCode.resize(iShaderByteCodeSize);
         sStream.read(&vShaderByteCode[0], iShaderByteCodeSize);

         cShaderProgram->VS = Allocator::alloc<VertexShader>();
         GEInvokeCtor(VertexShader, cShaderProgram->VS)(&vShaderByteCode[0], iShaderByteCodeSize, iVertexElementsMask, dxDevice.Get());

         iShaderByteCodeSize = Value::fromStream(ValueType::UInt, sStream).getAsUInt();
         vShaderByteCode.resize(iShaderByteCodeSize);
         sStream.read(&vShaderByteCode[0], iShaderByteCodeSize);

         cShaderProgram->PS = Allocator::alloc<PixelShader>();
         GEInvokeCtor(PixelShader, cShaderProgram->PS)(&vShaderByteCode[0], iShaderByteCodeSize, dxDevice.Get());

         mShaderPrograms.add(cShaderProgram);
      }
   }

   iActiveProgram = 0;
}

void RenderSystemDX11::releaseShaders()
{
   mShaderPrograms.clear();
}

void RenderSystemDX11::releaseStates()
{
   dxDepthStencilStateNoDepth->Release();
   dxDepthStencilStateTestOnly->Release();
   dxDepthStencilStateTestAndWrite->Release();
   dxBlendStateAlpha->Release();
   dxRasterizerStateSolidCullBack->Release();
   dxRasterizerStateSolidCullFront->Release();
   dxRasterizerStateSolidNoCull->Release();
   dxRasterizerStateWireFrame->Release();
   dxSamplerStateClamp->Release();
}

void RenderSystemDX11::releaseBuffers()
{
   dxConstantBufferTransform->Release();
   dxConstantBufferMaterial->Release();
   dxConstantBufferLighting->Release();
   dxConstantBufferVertexParameters->Release();
   dxConstantBufferFragmentParameters->Release();

   for(uint i = 0; i < GeometryGroup::Count; i++)
   {
      static_cast<ID3D11Buffer*>(sGPUBufferPairs[i].VertexBuffer)->Release();
      static_cast<ID3D11Buffer*>(sGPUBufferPairs[i].IndexBuffer)->Release();
   }
}

void RenderSystem::bindBuffers(const GPUBufferPair& sBufferPair)
{
   ID3D11Buffer* dxVertexBuffer = reinterpret_cast<ID3D11Buffer*>(sBufferPair.VertexBuffer);
   ID3D11Buffer* dxIndexBuffer = reinterpret_cast<ID3D11Buffer*>(sBufferPair.IndexBuffer);

   if(pCurrentVertexBuffer != dxVertexBuffer)
   {
      GE::uint iOffset = 0;
      dxContext->IASetVertexBuffers(0, 1, &dxVertexBuffer, &sBufferPair.VertexStride, &iOffset);
      pCurrentVertexBuffer = dxVertexBuffer;
   }

   if(pCurrentIndexBuffer != dxIndexBuffer)
   {
      dxContext->IASetIndexBuffer(dxIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
      pCurrentIndexBuffer = dxIndexBuffer;
   }
}

void RenderSystem::bindTexture(const Texture* cTexture)
{
   if(pBoundTexture == cTexture)
      return;

   pBoundTexture = const_cast<Texture*>(cTexture);

   ID3D11ShaderResourceView* dxShaderResourceView = (ID3D11ShaderResourceView*)cTexture->getHandler();
   dxContext->PSSetShaderResources(0, 1, &dxShaderResourceView);
   dxContext->PSSetSamplers(0, 1, &dxSamplerStateClamp);
}

void RenderSystem::useShaderProgram(const ObjectName& cName)
{
   if(iActiveProgram == cName.getID())
      return;

   iActiveProgram = cName.getID();

   const ShaderProgramDX11* cShaderProgram = static_cast<ShaderProgramDX11*>(mShaderPrograms.get(cName));
   GEAssert(cShaderProgram);

   dxContext->IASetInputLayout(cShaderProgram->VS->getInputLayout());
   dxContext->VSSetShader(cShaderProgram->VS->getShader(), nullptr, 0);
   ID3D11Buffer* sVertexShaderConstantBuffers[2] =
   {
      dxConstantBufferTransform,
      dxConstantBufferVertexParameters
   };
   dxContext->VSSetConstantBuffers(0, 2, sVertexShaderConstantBuffers);

   dxContext->PSSetShader(cShaderProgram->PS->getShader(), nullptr, 0);
   ID3D11Buffer* sPixelShaderConstantBuffers[3] =
   {
      dxConstantBufferMaterial,
      dxConstantBufferLighting,
      dxConstantBufferFragmentParameters
   };
   dxContext->PSSetConstantBuffers(0, 3, sPixelShaderConstantBuffers);

   setDepthBufferMode(cShaderProgram->getDepthBufferMode());
   setCullingMode(cShaderProgram->getCullingMode());
}

void RenderSystem::renderShadowMap()
{
   if(vLightsToRender.empty())
      return;

   ComponentLight* cLight = vLightsToRender[0];

   if(cLight->getLightType() != LightType::Directional)
      return;

   CD3D11_VIEWPORT dxViewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)ShadowMapSize, (float)ShadowMapSize);
   dxContext->RSSetViewports(1, &dxViewport);

   cShadowMap->setAsRenderTarget();
   cShadowMap->clear(Color(1.0f, 1.0f, 1.0f));

   calculateLightViewProjectionMatrix(cLight);

   if(!vShadowedMeshesToRender.empty())
   {
      useShaderProgram(ShadowMapSolidProgram);

      GESTLVector(RenderOperation)::const_iterator it = vShadowedMeshesToRender.begin();

      for(; it != vShadowedMeshesToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;
         Entity* cEntity = sRenderOperation.Renderable->getOwner();

         // set uniform
         const Matrix4& matModel = cEntity->getComponent<ComponentTransform>()->getGlobalWorldMatrix();
         Matrix4Multiply(matLightViewProjection, matModel, &sShaderConstantsTransform.WorldViewProjectionMatrix);
         dxContext->UpdateSubresource(dxConstantBufferTransform, 0, NULL, &sShaderConstantsTransform, 0, 0);

         // draw
         GESTLMap(uint, GeometryRenderInfo)* mGeometryRegistry = 0;

         if(sRenderOperation.Renderable->getGeometryType() == GeometryType::Static)
         {
            mGeometryRegistry = &mStaticGeometryToRender;
            bindBuffers(sGPUBufferPairs[GeometryGroup::MeshStatic]);
         }
         else
         {
            mGeometryRegistry = &mDynamicGeometryToRender;
            bindBuffers(sGPUBufferPairs[GeometryGroup::MeshDynamic]);
         }

         GESTLMap(uint, GeometryRenderInfo)::const_iterator itInfo = mGeometryRegistry->find(cEntity->getFullName().getID());
         const GeometryRenderInfo& sGeometryInfo = itInfo->second;
         UINT iStartIndexLocation = sGeometryInfo.IndexBufferOffset / sizeof(ushort);
         INT iBaseVertexLocation = sGeometryInfo.VertexBufferOffset / sRenderOperation.Renderable->getGeometryData().VertexStride;

         dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
         dxContext->DrawIndexed(sRenderOperation.Renderable->getGeometryData().NumIndices, iStartIndexLocation, iBaseVertexLocation);
      }
   }

   if(!vShadowedParticlesToRender.empty())
   {
      useShaderProgram(ShadowMapAlphaProgram);

      GESTLVector(RenderOperation)::const_iterator it = vShadowedParticlesToRender.begin();

      for(; it != vShadowedParticlesToRender.end(); it++)
      {
         const RenderOperation& sRenderOperation = *it;
         Entity* cEntity = sRenderOperation.Renderable->getOwner();

         // set uniform
         memcpy(&sShaderConstantsTransform.WorldViewProjectionMatrix, &matLightViewProjection, sizeof(Matrix4));
         dxContext->UpdateSubresource(dxConstantBufferTransform, 0, NULL, &sShaderConstantsTransform, 0, 0);

         if(sRenderOperation.RenderMaterialPass->getMaterial()->getDiffuseTexture())
            bindTexture(sRenderOperation.RenderMaterialPass->getMaterial()->getDiffuseTexture());

         // draw
         bindBuffers(sGPUBufferPairs[GeometryGroup::Particles]);

         GESTLMap(uint, GeometryRenderInfo)::const_iterator itInfo = mDynamicGeometryToRender.find(cEntity->getFullName().getID());
         const GeometryRenderInfo& sGeometryInfo = itInfo->second;
         UINT iStartIndexLocation = sGeometryInfo.IndexBufferOffset / sizeof(ushort);
         INT iBaseVertexLocation = sGeometryInfo.VertexBufferOffset / sRenderOperation.Renderable->getGeometryData().VertexStride;

         dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
         dxContext->DrawIndexed(sRenderOperation.Renderable->getGeometryData().NumIndices, iStartIndexLocation, iBaseVertexLocation);
      }
   }

   dxContext->OMSetRenderTargets(1, dxRenderTargetView.GetAddressOf(), dxDepthStencilView.Get());

   dxViewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)Device::ScreenWidth, (float)Device::ScreenHeight);
   dxContext->RSSetViewports(1, &dxViewport);

   dxContext->PSSetSamplers(1, 1, &dxSamplerStateWrap);
}

void RenderSystem::renderBegin()
{
   dxContext->ClearRenderTargetView(dxRenderTargetView.Get(), &cBackgroundColor.Red);
   dxContext->ClearDepthStencilView(dxDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
   iDrawCalls = 0;

   sShaderConstantsLighting.AmbientLightColor = cAmbientLightColor;

   if(cActiveCamera)
   {
      sShaderConstantsLighting.EyePosition = cActiveCamera->getTransform()->getPosition();
   }
}

void RenderSystem::render(const RenderOperation& sRenderOperation)
{
   MaterialPass* cMaterialPass = sRenderOperation.RenderMaterialPass;
   ComponentRenderable* cRenderable = sRenderOperation.Renderable;

   ComponentMesh* cMesh = 0;

   if(cRenderable)
   {
      if(cRenderable->getRenderableType() == RenderableType::ParticleSystem)
      {
         const Matrix4& mViewProjection = cRenderable->getRenderingMode() == RenderingMode::_3D
            ? cActiveCamera->getViewProjectionMatrix()
            : mat2DViewProjection;

         matModelViewProjection = mViewProjection;
      }
      else
      {
         // get model matrix from the renderable
         ComponentTransform* cTransform = cRenderable->getTransform();
         const Matrix4& matModel = cTransform->getGlobalWorldMatrix();

         if(cRenderable->getRenderingMode() == RenderingMode::_2D)
            calculate2DTransformMatrix(matModel);
         else
            calculate3DTransformMatrix(matModel);

         // if the renderable is a 3D mesh, calculate the inverse transpose matrix
         if(cRenderable->getRenderableType() == RenderableType::Mesh)
         {
            cMesh = static_cast<ComponentMesh*>(cRenderable);
            calculate3DInverseTransposeMatrix(matModel);

            sShaderConstantsTransform.InverseTransposeWorldMatrix = matModelInverseTranspose;

            if(GEHasFlag(cMesh->getDynamicShadows(), DynamicShadowsBitMask::Receive))
            {
               Matrix4Multiply(matLightViewProjection, matModel, &sShaderConstantsTransform.LightWorldViewProjection);
            }
         }

         sShaderConstantsTransform.WorldMatrix = matModel;
      }
   }

   // update values in the constant buffers
   sShaderConstantsTransform.WorldViewProjectionMatrix = matModelViewProjection;
   dxContext->UpdateSubresource(dxConstantBufferTransform, 0, NULL, &sShaderConstantsTransform, 0, 0);

   sShaderConstantsMaterial.DiffuseColor = cMaterialPass->getMaterial()->getDiffuseColor() * cRenderable->getColor();
   sShaderConstantsMaterial.SpecularColor = cMaterialPass->getMaterial()->getSpecularColor();

   ComponentUIElement* cUIElement = cRenderable->getOwner()->getComponent<ComponentUIElement>();

   if(cUIElement)
      sShaderConstantsMaterial.DiffuseColor.Alpha *= cUIElement->getAlphaInHierarchy();

   dxContext->UpdateSubresource(dxConstantBufferMaterial, 0, NULL, &sShaderConstantsMaterial, 0, 0);

   if(cMaterialPass->hasVertexParameters())
      dxContext->UpdateSubresource(dxConstantBufferVertexParameters, 0, NULL, cMaterialPass->getConstantBufferDataVertex(), 0, 0);

   if(cMaterialPass->hasFragmentParameters())
      dxContext->UpdateSubresource(dxConstantBufferFragmentParameters, 0, NULL, cMaterialPass->getConstantBufferDataFragment(), 0, 0);

   // bind diffuse texture
   if(cRenderable->getRenderableType() == RenderableType::Label)
      bindTexture(static_cast<ComponentLabel*>(cRenderable)->getFont()->getTexture());
   else if(cMaterialPass->getMaterial()->getDiffuseTexture())
      bindTexture(cMaterialPass->getMaterial()->getDiffuseTexture());

   if(cMesh)
   {
      // if the renderable is a shadowed 3D mesh, bind shadow map
      if(GEHasFlag(cMesh->getDynamicShadows(), DynamicShadowsBitMask::Receive))
      {
         ID3D11ShaderResourceView* dxShaderResourceView = cShadowMap->getShaderResourceView();
         dxContext->PSSetShaderResources(1, 1, &dxShaderResourceView);
         dxContext->PSSetSamplers(0, 1, &dxSamplerStateClamp);
      }

      // individual light
      if(vLightsToRender.empty())
      {
         sShaderConstantsLighting.LightType = 0;
         sShaderConstantsLighting.LightColor = Color(0.0f, 0.0f, 0.0f);
      }
      else
      {
         ComponentLight* cLight = vLightsToRender[0];

         sShaderConstantsLighting.LightType = (GE::uint)cLight->getLightType();
         sShaderConstantsLighting.LightColor = cLight->getColor();
         sShaderConstantsLighting.LightPosition = cLight->getTransform()->getPosition();
         sShaderConstantsLighting.LightDirection = cLight->getDirection();
         sShaderConstantsLighting.Attenuation = cLight->getLinearAttenuation();
         sShaderConstantsLighting.ShadowIntensity = cLight->getShadowIntensity();
      }

      dxContext->UpdateSubresource(dxConstantBufferLighting, 0, NULL, &sShaderConstantsLighting, 0, 0);
   }

   // draw
   GESTLMap(uint, GeometryRenderInfo)* mGeometryToRenderMap = cRenderable->getGeometryType() == GeometryType::Static
      ? &mStaticGeometryToRender
      : &mDynamicGeometryToRender;

   bindBuffers(sGPUBufferPairs[sRenderOperation.Group]);
   GESTLMap(uint, GeometryRenderInfo)::const_iterator it = mGeometryToRenderMap->find(cRenderable->getOwner()->getFullName().getID());
   const GeometryRenderInfo& sGeometryInfo = it->second;
   uint iStartIndexLocation = sGeometryInfo.IndexBufferOffset / sizeof(ushort);
   uint iBaseVertexLocation = sGeometryInfo.VertexBufferOffset / sRenderOperation.Data.VertexStride;

   dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   dxContext->DrawIndexed(sRenderOperation.Data.NumIndices, iStartIndexLocation, iBaseVertexLocation);
   iDrawCalls++;
}

void RenderSystemDX11Helper::handleDeviceLost()
{
#if defined(GE_PLATFORM_WP8)
   cWindowBounds.Width = 0;
   cWindowBounds.Height = 0;
   dxSwapChain = nullptr;

   createDeviceResources();
   updateForWindowSizeChange();
#endif
}

void RenderSystem::renderEnd()
{
   HRESULT hr = dxSwapChain->Present(1, 0);

   dxContext->DiscardView(dxRenderTargetView.Get());
   dxContext->DiscardView(dxDepthStencilView.Get());

   if(hr == DXGI_ERROR_DEVICE_REMOVED)
   {
      RenderSystemDX11Helper::handleDeviceLost();
      pDevice = dxDevice.Get();
   }
}

void RenderSystem::setBlendingMode(BlendingMode Mode)
{
   if(eBlendingMode == Mode)
      return;

   eBlendingMode = Mode;

   switch(Mode)
   {
   case BlendingMode::None:
      dxContext->OMSetBlendState(dxBlendStateNone, 0, 0xffffffff);
      break;

   case BlendingMode::Alpha:
      dxContext->OMSetBlendState(dxBlendStateAlpha, 0, 0xffffffff);
      break;

   case BlendingMode::Additive:
      dxContext->OMSetBlendState(dxBlendStateAdditive, 0, 0xffffffff);
      break;

   default:
      break;
   }
}

void RenderSystem::setDepthBufferMode(DepthBufferMode Mode)
{
   if(eDepthBufferMode == Mode)
      return;

   eDepthBufferMode = Mode;

   switch(Mode)
   {
   case DepthBufferMode::NoDepth:
      dxContext->OMSetDepthStencilState(dxDepthStencilStateNoDepth, 0);
      break;

   case DepthBufferMode::TestOnly:
      dxContext->OMSetDepthStencilState(dxDepthStencilStateTestOnly, 0);
      break;

   case DepthBufferMode::TestAndWrite:
      dxContext->OMSetDepthStencilState(dxDepthStencilStateTestAndWrite, 0);
      break;

   default:
      break;
   }
}

void RenderSystem::setCullingMode(CullingMode Mode)
{
   if(eCullingMode == Mode)
      return;

   eCullingMode = Mode;

   switch(Mode)
   {
   case CullingMode::Back:
      dxContext->RSSetState(dxRasterizerStateSolidCullBack);
      break;

   case CullingMode::Front:
      dxContext->RSSetState(dxRasterizerStateSolidCullFront);
      break;

   case CullingMode::None:
      dxContext->RSSetState(dxRasterizerStateSolidNoCull);
      break;

   default:
      break;
   }
}

void RenderSystemDX11Helper::releaseResourcesForSuspending()
{
   dxSwapChain = nullptr;
   dxRenderTargetView = nullptr;
   dxDepthStencilView = nullptr;
}
