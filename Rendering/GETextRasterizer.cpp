
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GETextRasterizer.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GETextRasterizer.h"

#include "Core/GEDevice.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Externals/stblib/stb_image_write.h"

using namespace GE;
using namespace GE::Rendering;
using namespace GE::Core;
using namespace GE::Content;

static const FT_UInt kDPI = 96u;
static const float kPointsPerPixel = 64.0f;

TextRasterizer::TextRasterizer(uint32_t pCanvasWidth, uint32_t pCanvasHeight)
   : mBitmap(nullptr)
   , mFTFace(nullptr)
   , mFontSize(12.0f)
   , mColor(0xffffffff)
{
   resizeCanvas(pCanvasWidth, pCanvasHeight);

   FT_Init_FreeType(&mFTLibrary);
}

TextRasterizer::~TextRasterizer()
{
   for (GESTLMap(uint32_t, Font)::iterator it = mFonts.begin(); it != mFonts.end(); it++)
   {
      FT_Done_Face(it->second.mFTFace);
      it->second.mContentData.unload();
   }

   FT_Done_FreeType(mFTLibrary);

   Allocator::free(mBitmap);
}

void TextRasterizer::clearCanvas()
{
   memset(mBitmap, 0, mCanvasWidth * mCanvasHeight * 4u);
}

void TextRasterizer::resizeCanvas(uint32_t pCanvasWidth, uint32_t pCanvasHeight)
{
   if(mBitmap)
   {
      Allocator::free(mBitmap);
   }

   mCanvasWidth = pCanvasWidth;
   mCanvasHeight = pCanvasHeight;

   mBitmap = Allocator::alloc<uint8_t>(pCanvasWidth * pCanvasHeight * 4u);
}

void TextRasterizer::setCursor(const Vector2& pScreenPosition)
{
   const Vector2 pixelPosition
   (
      (pScreenPosition.X + 1.0f) * 0.5f * (float)mCanvasWidth,
      (pScreenPosition.Y + 1.0f) * 0.5f * (float)mCanvasHeight
   );

   mCursor.X = (int)(pixelPosition.X * kPointsPerPixel);
   mCursor.Y = (int)(pixelPosition.Y * kPointsPerPixel);
}

bool TextRasterizer::setFont(const Core::ObjectName& pFontName)
{
   GESTLMap(uint32_t, Font)::iterator it = mFonts.find(pFontName.getID());

   if(it != mFonts.end())
   {
      mFTFace = it->second.mFTFace;
   }
   else
   {
      mFonts[pFontName.getID()] = Font();
      it = mFonts.find(pFontName.getID());
      Font& font = it->second;

      Device::readContentFile(ContentType::GenericBinaryData, "Fonts", pFontName.getString(), "ttf", &font.mContentData);

      FT_Error error =
         FT_New_Memory_Face(mFTLibrary, (FT_Byte*)font.mContentData.getData(), (FT_Long)font.mContentData.getDataSize(), 0, &mFTFace);

      if(error)
      {
         return false;
      }

      font.mFTFace = mFTFace;
   }

   return true;
}

void TextRasterizer::setFontSize(float pFontSize)
{
   mFontSize = pFontSize;
}

void TextRasterizer::setColor(const Color& pColor)
{
   mColor = (uint32_t)(pColor.Alpha * 255.0f);
   mColor |= (uint32_t)(pColor.Blue * 255.0f) << 8;
   mColor |= (uint32_t)(pColor.Green * 255.0f) << 16;
   mColor |= (uint32_t)(pColor.Red * 255.0f) << 24;
}

bool TextRasterizer::renderGlyph(uint16_t pGlyphIndex, const Matrix4& pTransform)
{
   FT_Error error =
      FT_Set_Char_Size(mFTFace, (FT_F26Dot6)(mFontSize * kPointsPerPixel), (FT_F26Dot6)0, kDPI, 0u);

   if(error)
   {
      return false;
   }
 
   FT_Matrix matrix;
   matrix.xx = (FT_Fixed)(pTransform.m[GE_M4_1_1] * 65536.0f);
   matrix.xy = (FT_Fixed)(pTransform.m[GE_M4_1_2] * 65536.0f);
   matrix.xw = (FT_Fixed)(pTransform.m[GE_M4_1_4] * 65536.0f);
   matrix.yx = (FT_Fixed)(pTransform.m[GE_M4_2_1] * 65536.0f);
   matrix.yy = (FT_Fixed)(pTransform.m[GE_M4_2_2] * 65536.0f);
   matrix.yw = (FT_Fixed)(pTransform.m[GE_M4_2_4] * 65536.0f);
   matrix.wx = (FT_Fixed)(pTransform.m[GE_M4_4_1] * 65536.0f);
   matrix.wy = (FT_Fixed)(pTransform.m[GE_M4_4_2] * 65536.0f);
   matrix.ww = (FT_Fixed)(pTransform.m[GE_M4_4_4] * 65536.0f);

   FT_Vector cursor;
   cursor.x = (FT_Pos)mCursor.X;
   cursor.y = (FT_Pos)mCursor.Y;

   FT_Set_Transform(mFTFace, &matrix, &cursor);

   error = FT_Load_Char(mFTFace, (FT_ULong)pGlyphIndex, FT_LOAD_RENDER);
   
   if(error)
   {
      return false;
   }

   const FT_GlyphSlot slot = mFTFace->glyph;

   const FT_Int x = slot->bitmap_left;
   const FT_Int y = mCanvasHeight - slot->bitmap_top; 

   const FT_Int xMax = x + slot->bitmap.width;
   const FT_Int yMax = y + slot->bitmap.rows;

   for(FT_Int i = x, p = 0; i < xMax; i++, p++)
   {
      for(FT_Int j = y, q = 0; j < yMax; j++, q++)
      {
         if(i < 0 || j < 0 || i >= (FT_Int)mCanvasWidth || j >= (FT_Int)mCanvasHeight)
         {
            continue;
         }

         uint8_t* pixel = &mBitmap[(j * mCanvasWidth + i) * 4];
         *pixel++ = (uint8_t)((mColor & 0xff000000) >> 24);
         *pixel++ = (uint8_t)((mColor & 0x00ff0000) >> 16);
         *pixel++ = (uint8_t)((mColor & 0x0000ff00) >> 8);
         *pixel++ |= slot->bitmap.buffer[(q * slot->bitmap.width) + p];
      }
   }

   mCursor.X += (int)slot->advance.x;
   mCursor.Y += (int)slot->advance.y;

   return true;
}

void TextRasterizer::writeToFile(const char* pFilePath)
{
   stbi_write_png(pFilePath, (int)mCanvasWidth, (int)mCanvasHeight, 4, mBitmap, (int)mCanvasWidth * 4);
}
