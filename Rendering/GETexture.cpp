
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

Texture::Texture(const ObjectName& Name, const ObjectName& GroupName, uint Width, uint Height)
   : Resource(Name, GroupName)
   , iWidth(Width)
   , iHeight(Height)
   , pHandlePtr(0)
{
   TextureCoordinates tDefaultUV = { 0.0f, 1.0f, 0.0f, 1.0f };
   TextureAtlasEntry tDefaultAtlas = { ObjectName::Empty, tDefaultUV };
   AtlasUV.push_back(tDefaultAtlas);
}

uint Texture::getWidth() const
{
   return iWidth;
}

uint Texture::getHeight() const
{
   return iHeight;
}

void Texture::setHandler(void* HandlePtr)
{
   pHandlePtr = HandlePtr;
}

const void* Texture::getHandler() const
{
   return pHandlePtr;
}
