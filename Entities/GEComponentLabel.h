
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


   class ComponentLabelBase : public ComponentRenderable
   {
   protected:
      ComponentLabelBase(Entity* pOwner);
      ~ComponentLabelBase();

      struct Pen
      {
         Core::ObjectName mFontName;
         Core::ObjectName mFontStyle;
         Color mColor;
         float mFontSize;
         float mYOffset;
         uint32_t mCharIndex;
      };

      Core::ObjectName mFontName;
      Core::ObjectName mFontStyle;
      float mFontSize;
      float mFontResizeFactor;
      Alignment mAlignment;
      SpriteLayer mLayer;
      uint8_t mSettings;
      Core::ObjectName mStringID;
      GESTLString mText;
      GESTLString mTextExtension;
      uint32_t mTextLength;
      uint32_t mCharacterCountLimit;

      uint16_t getGlyphIndex(size_t pCharIndex) const;

      virtual float getInternalFontSize() const;
      virtual float getDefaultVerticalOffset() const;

      bool evaluateRichTextTag(Pen* pPen);
      void processVariables();

      virtual void generateText() = 0;

   public:
      float getFontSize() const;
      Alignment getAlignment() const;
      const char* getText() const;
      const Core::ObjectName& getStringID() const;
      uint32_t getTextLength() const;
      uint32_t getCharacterCountLimit() const;
      SpriteLayer getLayer() const;
      uint8_t getSettings() const;

      void setFontSize(float pFontSize);
      void setAlignment(Alignment pAlignment);
      void setText(const char* pText);
      void setStringID(const Core::ObjectName& pStringID);
      void setCharacterCountLimit(uint32_t pLimit);
      void setLayer(SpriteLayer pLayer);
      void setSettings(uint8_t pSettings);
   };


   class ComponentLabel : public ComponentLabelBase
   {
   private:
      Rendering::Font* mFont;
      Rendering::FontReplacement* mFontReplacement;

      float mHorizontalSpacing;
      float mVerticalSpacing;
      float mLineWidth;
      float mTextWidth;

      GESTLVector(float) mLineWidths;
      GESTLVector(uint) mLineFeedIndices;
      GESTLVector(bool) mLineFeedSkipChar;
      GESTLVector(uint) mLineJustifySpaces;

      GESTLVector(float) mVertexData;
      GESTLVector(ushort) mIndices;

      virtual float getInternalFontSize() const override;
      virtual float getDefaultVerticalOffset() const override;

      virtual void generateText() override;

      float measureCharacter(const Pen& pPen);
      float getKerning(const Pen& pPen);

   public:
      static const Core::ObjectName ClassName;

      ComponentLabel(Entity* pOwner);
      ~ComponentLabel();

      Rendering::Font* getFont();
      const Core::ObjectName& getFontName() const;
      const Core::ObjectName& getFontCharacterSet() const;
      float getHorizontalSpacing() const;
      float getVerticalSpacing() const;
      float getLineWidth() const;
      float getTextWidth() const;

      void setFont(Rendering::Font* pTextFont);
      void setFontName(const Core::ObjectName& pFontName);
      void setFontCharacterSet(const Core::ObjectName& pCharSetName);
      void setHorizontalSpacing(float pHorizontalSpacing);
      void setVerticalSpacing(float pVerticalSpacing);
      void setLineWidth(float pLineWidth);
   };

#if defined (GE_TEXT_RASTERIZER_SUPPORT)
   class ComponentLabelRaster : public ComponentLabelBase
   {
   private:
      Core::ObjectName mFontFamily;
      Core::ObjectName mFontStyle;

      virtual void generateText() override;

   public:
      static const Core::ObjectName ClassName;

      ComponentLabelRaster(Entity* pOwner);
      ~ComponentLabelRaster();

      GEDefaultGetter(const Core::ObjectName&, FontFamily, m)
      GEDefaultGetter(const Core::ObjectName&, FontStyle, m)

      void setFontFamily(const Core::ObjectName& pFontFamily);
      void setFontStyle(const Core::ObjectName& pFontStyle);
   };
#endif
}}
