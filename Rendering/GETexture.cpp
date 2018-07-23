
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GETexture.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GETexture.h"

using namespace GE;
using namespace GE::Rendering;
using namespace GE::Core;
using namespace GE::Content;

const ObjectName Texture::TypeName = ObjectName("Texture");

Texture::Texture(const ObjectName& Name, const ObjectName& GroupName)
   : Resource(Name, GroupName, TypeName)
   , iWidth(0)
   , iHeight(0)
   , eSettings(0)
   , eWrapMode(TextureWrapMode::Clamp)
   , pHandlePtr(0)
{
   sFormat[0] = '\0';

   TextureCoordinates tDefaultUV = { 0.0f, 1.0f, 0.0f, 1.0f };
   TextureAtlasEntry tDefaultAtlas = { ObjectName::Empty, tDefaultUV };
   AtlasUV.push_back(tDefaultAtlas);

   GERegisterPropertyReadonly(UInt, Width);
   GERegisterPropertyReadonly(UInt, Height);
   GERegisterProperty(String, Format);
   GERegisterPropertyBitMask(TextureSettingsBitMask, Settings);
   GERegisterPropertyEnum(TextureWrapMode, WrapMode);
}

uint Texture::getAtlasSize() const
{
   return (uint)AtlasUV.size();
}

const ObjectName& Texture::getAtlasName(uint Index) const
{
   GEAssert(Index < AtlasUV.size());
   return AtlasUV[Index].getName();
}

void Texture::populateAtlasUVManager()
{
   for(uint i = 0; i < AtlasUV.size(); i++)
      AtlasUVManager.add(&AtlasUV[i]);
}
