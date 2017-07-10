
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (OpenGL ES)
//
//  --- GEFontES20.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "Rendering/GEFont.h"
#include "Core/GEAllocator.h"
#include "pugixml/pugixml.hpp"
#include "GEOpenGLES20.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Rendering;

void Font::createFontTexture(ImageData& cImageData)
{
   GLuint iTexture;

   glGenTextures(1, &iTexture);
   glBindTexture(GL_TEXTURE_2D, iTexture);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   GLenum glFormat = cImageData.getBytesPerPixel() == 4 ? GL_RGBA : GL_RGB;
   glTexImage2D(GL_TEXTURE_2D, 0, glFormat, cImageData.getWidth(), cImageData.getHeight(),
      0, glFormat, GL_UNSIGNED_BYTE, cImageData.getData());

   cTexture = Allocator::alloc<Texture>();
   GEInvokeCtor(Texture, cTexture)(cName, "FontTextures", cImageData.getWidth(), cImageData.getHeight());
   cTexture->setHandler((void*)((GLuintPtrSize)iTexture));
}

void Font::releaseFontTexture()
{
   GLuint iTexture = (GLuint)((GLuintPtrSize)cTexture->getHandler());
   glBindTexture(GL_TEXTURE_2D, 0);
   glDeleteTextures(1, &iTexture);
}
