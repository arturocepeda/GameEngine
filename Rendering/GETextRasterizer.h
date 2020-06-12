
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

#include "Core/GEObject.h"
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
   class TextRasterizer
   {
   private:
      uint32_t mCanvasWidth;
      uint32_t mCanvasHeight;
      uint8_t* mBitmap;

      FT_Library mFTLibrary;

      GESTLVector(Content::ContentData) mFontContentData;

      struct FontStyle
      {
         Core::ObjectName mName;
         FT_Face mFTFace;
      };
      struct Font
      {
         Core::ObjectName mName;
         GESTLVector(FontStyle) mStyles;
      };
      GESTLMap(uint32_t, Font) mFonts;

      Point mCursor;
      FontStyle* mFontStyle;
      float mFontSize;
      uint32_t mColor;

      void loadFontFile(const char* pFileName, const char* pFileExtension);

   public:
      TextRasterizer(uint32_t pCanvasWidth, uint32_t pCanvasHeight);
      ~TextRasterizer();

      void loadFontFiles();

      void clearCanvas();
      void resizeCanvas(uint32_t pCanvasWidth, uint32_t pCanvasHeight);

      void setCursor(const Vector2& pScreenPosition);
      bool setFont(const Core::ObjectName& pFontName, const Core::ObjectName& pFontStyleName);
      void setFontSize(float pFontSize);
      void setColor(const Color& pColor);

      bool renderGlyph(uint16_t pGlyphIndex, const Matrix4& pTransform);

      void writeToFile(const char* pFilePath);
   };
}}
