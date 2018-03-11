
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

      Count = 2
   };


   class ComponentLabel : public ComponentRenderable
   {
   private:
      Rendering::Font* cFont;
      float fFontSize;
      Alignment iAlignment;
      uint8_t eSettings;
      Core::ObjectName cStringID;
      GESTLString sText;

      float fHorizontalSpacing;
      float fVerticalSpacing;
      float fLineWidth;

      float fCharacterSize;

      GESTLVector(float) vLineWidths;
      GESTLVector(uint) vLineFeedIndices;
      GESTLVector(uint) vLineJustifySpaces;

      GESTLVector(float) vVertexData;
      GESTLVector(ushort) vIndices;

      void processVariables();
      void generateVertexData();

      float measureCharacter(uint iCharIndex);
      float getKerning(uint iCharIndex);

   public:
      ComponentLabel(Entity* Owner);
      ~ComponentLabel();

      Rendering::Font* getFont() const;
      const Core::ObjectName& getFontName() const;
      float getFontSize() const;
      Alignment getAlignment() const;
      const char* getText() const;
      const Core::ObjectName& getStringID() const;
      float getHorizontalSpacing() const;
      float getVerticalSpacing() const;
      float getLineWidth() const;
      uint8_t getSettings() const;

      void setFont(Rendering::Font* TextFont);
      void setFontName(const Core::ObjectName& FontName);
      void setFontSize(float FontSize);
      void setAlignment(Alignment Align);
      void setText(const char* Text);
      void setStringID(const Core::ObjectName& StringID);
      void setHorizontalSpacing(float HorizontalSpacing);
      void setVerticalSpacing(float VerticalSpacing);
      void setLineWidth(float LineWidth);
      void setSettings(uint8_t Settings);
   };
}}
