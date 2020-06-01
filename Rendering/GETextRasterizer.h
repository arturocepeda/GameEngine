
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

      struct Font
      {
         Content::ContentData mContentData;
         FT_Face mFTFace;
      };
      GESTLMap(uint32_t, Font) mFonts;

      Point mCursor;
      FT_Face mFTFace;
      float mFontSize;
      uint32_t mColor;

   public:
      TextRasterizer(uint32_t pCanvasWidth, uint32_t pCanvasHeight);
      ~TextRasterizer();

      void clearCanvas();
      void resizeCanvas(uint32_t pCanvasWidth, uint32_t pCanvasHeight);

      void setCursor(const Vector2& pScreenPosition);
      bool setFont(const Core::ObjectName& pFontName);
      void setFontSize(float pFontSize);
      void setColor(const Color& pColor);

      bool renderGlyph(uint16_t pGlyphIndex, const Matrix4& pTransform);

      void writeToFile(const char* pFilePath);
   };
}}
