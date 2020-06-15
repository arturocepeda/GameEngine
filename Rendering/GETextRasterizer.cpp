
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
#include "Core/GELog.h"
#include "Content/GEResourcesManager.h"

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

static const char* kSubDir = "Fonts";
static constexpr char* kSupportedExtensions[] = { "ttf", "otf" };
static constexpr size_t kSupportedExtensionsCount = sizeof(kSupportedExtensions) / sizeof(const char*);


//
//  FontStyle
//
FontStyle::FontStyle(const ObjectName& pName)
   : Object(pName)
{
}


//
//  FontFamily
//
const ObjectName FontFamily::TypeName("FontFamily");

FontFamily::FontFamily(const ObjectName& pName)
   : Object(pName)
{
}


//
//  TextRasterizer
//
TextRasterizer::TextRasterizer(uint32_t pCanvasWidth, uint32_t pCanvasHeight)
   : mBitmap(nullptr)
   , mFontStyle(nullptr)
   , mFontSize(12.0f)
   , mColor(0xffffffff)
   , mLastGlyph(0u)
{
   ResourcesManager::getInstance()->registerObjectManager<FontFamily>(FontFamily::TypeName, &mFonts);

   resizeCanvas(pCanvasWidth, pCanvasHeight);

   FT_Init_FreeType(&mFTLibrary);
}

TextRasterizer::~TextRasterizer()
{
   mFonts.iterate([](FontFamily* pFontFamily)
   {
      pFontFamily->mStyles.iterate([](FontStyle* pFontStyle)
      {
         FT_Done_Face(pFontStyle->mFTFace);
         return true;
      });

      return true;
   });

   for(size_t i = 0u; i < mFontContentData.size(); i++)
   {
      mFontContentData[i].unload();
   }

   FT_Done_FreeType(mFTLibrary);

   Allocator::free(mBitmap);
}

void TextRasterizer::loadFontFiles()
{
   FileNamesList fileNames[kSupportedExtensionsCount];
   size_t filesCount = 0u;

   for(size_t i = 0u; i < kSupportedExtensionsCount; i++)
   {
      Device::getContentFileNames(kSubDir, kSupportedExtensions[i], &fileNames[i]);
      filesCount += fileNames[i].size();
   }

   mFontContentData.reserve(filesCount);

   for(size_t i = 0u; i < kSupportedExtensionsCount; i++)
   {
      for(size_t j = 0u; j < fileNames[i].size(); j++)
      {
         loadFontFile(fileNames[i][j].c_str(), kSupportedExtensions[i]);
      }
   }
}

void TextRasterizer::loadFontFile(const char* pFileName, const char* pFileExtension)
{
   mFontContentData.emplace_back();
   ContentData& contentData = mFontContentData.back();
   Device::readContentFile(ContentType::GenericBinaryData, kSubDir, pFileName, pFileExtension, &contentData);

   FT_Face ftFace;
   FT_New_Memory_Face(mFTLibrary, (FT_Byte*)contentData.getData(), (FT_Long)contentData.getDataSize(), -1, &ftFace);
   FT_Long ftFacesCount = ftFace->num_faces;
   FT_Done_Face(ftFace);

   for(FT_Long ftFaceIndex = 0; ftFaceIndex < ftFacesCount; ftFaceIndex++)
   {
      FT_Error ftError =
         FT_New_Memory_Face(mFTLibrary, (FT_Byte*)contentData.getData(), (FT_Long)contentData.getDataSize(), ftFaceIndex++, &ftFace);

      if(ftError)
      {
         Log::log(LogType::Warning, "Font file could not be loaded: '%s.%s'", pFileName, pFileExtension);
         continue;
      }

      const ObjectName fontFamilyName(ftFace->family_name);
      FontFamily* fontFamily = mFonts.get(fontFamilyName);

      if(!fontFamily)
      {
         fontFamily = Allocator::alloc<FontFamily>();
         GEInvokeCtor(FontFamily, fontFamily)(fontFamilyName);
         mFonts.add(fontFamily);
      }

      FontStyle* fontStyle = Allocator::alloc<FontStyle>();
      GEInvokeCtor(FontStyle, fontStyle)(ftFace->style_name);
      fontStyle->mFTFace = ftFace;

      fontFamily->mStyles.add(fontStyle);
   }

   Log::log(LogType::Info, "Font file loaded: '%s.%s'", pFileName, pFileExtension);
}

FontFamily* TextRasterizer::getFontFamily(const Core::ObjectName& pFontFamilyName)
{
   return mFonts.get(pFontFamilyName);
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

bool TextRasterizer::setFont(const Core::ObjectName& pFontFamilyName, const Core::ObjectName& pFontStyleName)
{
   FontFamily* fontFamily = mFonts.get(pFontFamilyName);

   if(fontFamily)
   {
      FontStyle* fontStyle = fontFamily->mStyles.get(pFontStyleName);
      
      if(fontStyle)
      {
         mFontStyle = fontStyle;
         return true;
      }
   }

   return false;
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

void TextRasterizer::reset()
{
   clearCanvas();
   mLastGlyph = 0u;
}

bool TextRasterizer::renderGlyph(uint16_t pGlyphIndex, const Matrix4& pTransform)
{
   FT_Error error =
      FT_Set_Char_Size(mFontStyle->mFTFace, (FT_F26Dot6)(mFontSize * kPointsPerPixel), (FT_F26Dot6)0, kDPI, 0u);

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

   const FT_ULong glyph = (FT_ULong)pGlyphIndex;

   if(FT_HAS_KERNING(mFontStyle->mFTFace))
   {
      FT_Vector kerning;
      FT_Get_Kerning(mFontStyle->mFTFace, mLastGlyph, glyph, FT_KERNING_DEFAULT, &kerning);
      cursor.x += kerning.x;
   }

   FT_Set_Transform(mFontStyle->mFTFace, &matrix, &cursor);

   error = FT_Load_Char(mFontStyle->mFTFace, glyph, FT_LOAD_RENDER);
   
   if(error)
   {
      return false;
   }

   const FT_GlyphSlot slot = mFontStyle->mFTFace->glyph;

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

   mLastGlyph = glyph;

   return true;
}

void TextRasterizer::writeToFile(const char* pFilePath)
{
   stbi_write_png(pFilePath, (int)mCanvasWidth, (int)mCanvasHeight, 4, mBitmap, (int)mCanvasWidth * 4);
}
