
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GETextRasterizer.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEObjectManager.h"
#include "Content/GEContentData.h"


//
//  FreeType2 -- Forward declarations
//
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_* FT_Library;
struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;


namespace GE { namespace Rendering
{
   class FontStyle : public Core::Object
   {
   public:
      FT_Face mFTFace;

      FontStyle(const Core::ObjectName& pName);
   };

   class FontFamily : public Core::Object
   {
   public:
      static const Core::ObjectName TypeName;

      Core::ObjectManager<FontStyle> mStyles;

      FontFamily(const Core::ObjectName& pName);
   };


   class TextRasterizer
   {
   private:
      uint32_t mCanvasWidth;
      uint32_t mCanvasHeight;
      uint8_t* mBitmap;

      FT_Library mFTLibrary;

      GESTLVector(Content::ContentData) mFontContentData;
      Core::ObjectManager<FontFamily> mFonts;

      Point mCursor;
      FontStyle* mFontStyle;
      float mFontSize;
      uint32_t mColor;

      uint32_t mLastGlyph;

      void loadFontFile(const char* pFileName, const char* pFileExtension);

   public:
      TextRasterizer(uint32_t pCanvasWidth, uint32_t pCanvasHeight);
      ~TextRasterizer();

      void loadFontFiles();
      FontFamily* getFontFamily(const Core::ObjectName& pFontFamilyName);

      void clearCanvas();
      void resizeCanvas(uint32_t pCanvasWidth, uint32_t pCanvasHeight);

      void setCursor(const Vector2& pScreenPosition);
      bool setFont(const Core::ObjectName& pFontFamilyName, const Core::ObjectName& pFontStyleName);
      void setFontSize(float pFontSize);
      void setColor(const Color& pColor);

      void reset();
      bool renderGlyph(uint16_t pGlyphIndex, const Matrix4& pTransform);

      void writeToFile(const char* pFilePath);
   };
}}
