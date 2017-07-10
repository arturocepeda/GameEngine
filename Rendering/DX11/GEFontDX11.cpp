
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (DirectX 11)
//
//  --- GEFontDX11.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "Rendering/GEFont.h"
#include "Core/GEAllocator.h"
#include "pugixml/pugixml.hpp"
#include <d3d11_1.h>
#include <DirectXMath.h>

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Rendering;

void Font::createFontTexture(ImageData& cImageData)
{
   D3D11_TEXTURE2D_DESC desc;
   desc.Width = cImageData.getWidth();
   desc.Height = cImageData.getHeight();
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
   initData.pSysMem = cImageData.getData();
   initData.SysMemPitch = static_cast<UINT>(cImageData.getWidth() * cImageData.getBytesPerPixel());
   initData.SysMemSlicePitch = static_cast<UINT>(cImageData.getWidth() * cImageData.getHeight() * cImageData.getBytesPerPixel());

   ID3D11Device1* dxDevice = static_cast<ID3D11Device1*>(pRenderDevice);
   ID3D11Texture2D* dxTexture = 0;

   ID3D11ShaderResourceView* dxResourceView;
   dxDevice->CreateTexture2D(&desc, &initData, &dxTexture);
   dxDevice->CreateShaderResourceView(dxTexture, 0, &dxResourceView);

   cTexture = Allocator::alloc<Texture>();
   GEInvokeCtor(Texture, cTexture)(cName, "FontTextures", cImageData.getWidth(), cImageData.getHeight());
   cTexture->setHandler(dxResourceView);
}

void Font::releaseFontTexture()
{
   static_cast<ID3D11ShaderResourceView*>(const_cast<void*>(cTexture->getHandler()))->Release();
   static_cast<ID3D11Device1*>(pRenderDevice)->Release();

   GEInvokeDtor(Texture, cTexture);
   Allocator::free(cTexture);
   cTexture = 0;
}
