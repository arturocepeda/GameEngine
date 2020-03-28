
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentLabel.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentRenderable.h"
#include "Rendering/GEFont.h"

#include <vector>

namespace GE { namespace Entities
{
   GESerializableEnum(LabelSettingsBitMask)
   {
      Justify              = 1 << 0,
      VariableReplacement  = 1 << 1,
      RichTextSupport      = 1 << 2,
      FitSizeToLineWidth   = 1 << 3,

      Count = 4
   };


   class ComponentLabel : public ComponentRenderable
   {
   private:
      struct Pen
      {
         Color mColor;
         float mFontSize;
         float mYOffset;
         uint32_t mCharSet;
         uint32_t mCharIndex;
      };

      Rendering::Font* cFont;
      uint32_t mFontCharSetIndex;
      float fFontSize;
      Alignment iAlignment;
      SpriteLayer mLayer;
      uint8_t eSettings;
      Core::ObjectName cStringID;
      GESTLString sText;
      GESTLString sTextExtension;

      float fHorizontalSpacing;
      float fVerticalSpacing;
      float fLineWidth;
      uint32_t mTextLength;
      uint32_t mCharacterCountLimit;

      float mFontResizeFactor;

      GESTLVector(float) vLineWidths;
      GESTLVector(uint) vLineFeedIndices;
      GESTLVector(uint) vLineJustifySpaces;

      GESTLVector(float) vVertexData;
      GESTLVector(ushort) vIndices;

      uint16_t getGlyphIndex(size_t pCharIndex);

      void evaluateRichTextTag(Pen* pPen);
      void processVariables();
      void generateVertexData();

      float measureCharacter(const Pen& pPen);
      float getKerning(const Pen& pPen);

   public:
      static const Core::ObjectName ClassName;

      ComponentLabel(Entity* Owner);
      ~ComponentLabel();

      Rendering::Font* getFont() const;
      const Core::ObjectName& getFontName() const;
      const Core::ObjectName& getFontCharacterSet() const;
      float getFontSize() const;
      Alignment getAlignment() const;
      const char* getText() const;
      const Core::ObjectName& getStringID() const;
      float getHorizontalSpacing() const;
      float getVerticalSpacing() const;
      float getLineWidth() const;
      uint32_t getTextLength() const;
      uint32_t getCharacterCountLimit() const;
      SpriteLayer getLayer() const;
      uint8_t getSettings() const;

      void setFont(Rendering::Font* TextFont);
      void setFontName(const Core::ObjectName& FontName);
      void setFontCharacterSet(const Core::ObjectName& pCharSetName);
      void setFontSize(float FontSize);
      void setAlignment(Alignment Align);
      void setText(const char* Text);
      void setStringID(const Core::ObjectName& StringID);
      void setHorizontalSpacing(float HorizontalSpacing);
      void setVerticalSpacing(float VerticalSpacing);
      void setLineWidth(float LineWidth);
      void setCharacterCountLimit(uint32_t Limit);
      void setLayer(SpriteLayer pLayer);
      void setSettings(uint8_t Settings);
   };
}}
