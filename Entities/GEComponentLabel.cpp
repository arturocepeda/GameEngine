
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentLabel.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentLabel.h"
#include "Rendering/GERenderSystem.h"
#include "Core/GEAllocator.h"
#include "Core/GELog.h"
#include "Core/GEEvents.h"
#include "Content/GELocalizedString.h"
#include "Content/GEResourcesManager.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;
using namespace GE::Content;

//
//  ComponentLabel
//
const float FontSizeScale = 0.0001f;
const unsigned char LineFeedChar = '~';

ComponentLabel::ComponentLabel(Entity* Owner)
   : ComponentRenderable(Owner, RenderableType::Label)
   , cFont(0)
   , mFontCharSetIndex(0)
   , fFontSize(12.0f)
   , iAlignment(Alignment::MiddleCenter)
   , eSettings(0)
   , fHorizontalSpacing(0.0f)
   , fVerticalSpacing(0.0f)
   , fLineWidth(0.0f)
{
   cClassName = ObjectName("Label");

   sGeometryData.VertexStride = (3 + 4 + 2) * sizeof(float);

#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::connectStaticEventCallback(Events::LocalizedStringsReloaded, this, [this](const EventArgs* args) -> bool
   {
      if(!cStringID.isEmpty())
      {
         setStringID(cStringID);
      }

      return false;
   });
#endif

   GERegisterProperty(ObjectName, FontName);
   GERegisterProperty(ObjectName, FontCharacterSet);
   GERegisterProperty(Float, FontSize);
   GERegisterPropertyEnum(Alignment, Alignment);
   GERegisterProperty(Float, HorizontalSpacing);
   GERegisterProperty(Float, VerticalSpacing);
   GERegisterProperty(Float, LineWidth);
   GERegisterPropertyBitMask(LabelSettingsBitMask, Settings);
   GERegisterProperty(String, Text);
   GERegisterProperty(ObjectName, StringID);
}

ComponentLabel::~ComponentLabel()
{
#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::disconnectStaticEventCallback(Events::LocalizedStringsReloaded, this);
#endif
}

void ComponentLabel::evaluateRichTextTag(Pen* pPen)
{
   unsigned char character = sText[pPen->mCharIndex];

   while(character == '<')
   {
      size_t i = (size_t)pPen->mCharIndex + 1;

      char tag[64];
      size_t tagLength = 0;
      bool tagClosing = false;

      char value[64];
      size_t valueLength = 0;

      char* currentString = tag;
      size_t* currentLength = &tagLength;

      while(i < sText.length() && sText[i] != '>')
      {
         character = sText[i++];

         if(character == '/' && i == (pPen->mCharIndex + 2))
         {
            tagClosing = true;
         }
         else if(character == '=')
         {
            currentString = value;
            currentLength = &valueLength;
         }
         else
         {
            currentString[(*currentLength)++] = character;
         }
      }

      if(i == sText.length())
         return;

      tag[tagLength] = '\0';
      value[valueLength] = '\0';

      // color
      if(strcmp(tag, "color") == 0)
      {
         if(tagClosing)
         {
            pPen->mColor = cColor;
         }
         else if(value[0] == '#')
         {
            const unsigned long colorIntValue = strtoul(value + 1, 0, 16);

            const unsigned long colorIntValueR = (colorIntValue & 0xff0000) >> 16;
            const unsigned long colorIntValueG = (colorIntValue & 0x00ff00) >> 8;
            const unsigned long colorIntValueB = (colorIntValue & 0x0000ff);

            pPen->mColor = Color
            (
               (float)colorIntValueR / 255.0f,
               (float)colorIntValueG / 255.0f,
               (float)colorIntValueB / 255.0f
            );
         }
      }
      // size
      else if(strcmp(tag, "size") == 0)
      {
         if(tagClosing)
         {
            pPen->mFontSize = fFontSize;
         }
         else
         {
            pPen->mFontSize = (float)strtod(value, 0);
         }
      }
      // charset
      else if(strcmp(tag, "charset") == 0)
      {
         if(tagClosing)
         {
            pPen->mCharSet = mFontCharSetIndex;
         }
         else
         {
            const ObjectName charSetName = ObjectName(value);
            pPen->mCharSet = cFont ? cFont->getCharacterSetIndex(charSetName) : 0;
         }
      }
      // yoffset
      else if(strcmp(tag, "yoffset") == 0)
      {
         if(tagClosing)
         {
            pPen->mYOffset = 0.0f;
         }
         else
         {
            pPen->mYOffset = (float)strtod(value, 0);
         }
      }

      pPen->mCharIndex = (uint32_t)i + 1;

      if(pPen->mCharIndex < (uint32_t)sText.size())
      {
         character = sText[pPen->mCharIndex];
      }
   }
}

void ComponentLabel::processVariables()
{
   size_t iVariableStartPos = sText.find('$');

   while(iVariableStartPos != GESTLString::npos)
   {
      size_t iVariableEndPos = iVariableStartPos + 1;

      while(iVariableEndPos < sText.length() &&
         (isalnum(sText[iVariableEndPos]) || sText[iVariableEndPos] == '_'))
      {
         iVariableEndPos++;
      }

      size_t iVariableNameLength = iVariableEndPos - iVariableStartPos - 1;

      if(iVariableNameLength > 0)
      {
         GESTLString sVariableName = sText.substr(iVariableStartPos + 1, iVariableNameLength);
         ObjectName cVariableName = ObjectName(sVariableName.c_str());
         const char* sVariableValue = LocalizedStringsManager::getInstance()->getVariable(cVariableName);

         if(sVariableValue)
         {
            sText.replace(iVariableStartPos, iVariableNameLength + 1, sVariableValue);
         }
      }

      iVariableStartPos = sText.find('$', iVariableStartPos + 1);
   }
}

void ComponentLabel::generateVertexData()
{
   if(!cFont)
      return;

   Pen sPen;
   sPen.mColor = cColor;
   sPen.mFontSize = fFontSize;
   sPen.mYOffset = 0.0f;
   sPen.mCharSet = mFontCharSetIndex;
   sPen.mCharIndex = 0;

   const uint iTextLength = (uint)sText.length();
   const bool bRichTextSupport = GEHasFlag(eSettings, LabelSettingsBitMask::RichTextSupport);

   vLineWidths.clear();
   vLineFeedIndices.clear();
   vLineJustifySpaces.clear();

   float fCurrentLineWidth = 0.0f;

   int iLastSpaceIndex = -1;
   float fLineWidthAtLastSpace = 0.0f;
   float fLastSpaceCharWidth = 0.0f;

   for(sPen.mCharIndex = 0; sPen.mCharIndex < iTextLength; sPen.mCharIndex++)
   {
      if(bRichTextSupport)
      {
         evaluateRichTextTag(&sPen);

         if(sPen.mCharIndex >= iTextLength)
            break;
      }

      unsigned char cChar = sText[sPen.mCharIndex];

      if(cChar == LineFeedChar)
      {
         vLineWidths.push_back(fCurrentLineWidth);
         vLineFeedIndices.push_back(sPen.mCharIndex);
         vLineJustifySpaces.push_back(0);

         fCurrentLineWidth = 0.0f;
         iLastSpaceIndex = -1;

         continue;
      }

      float fCharWidth = measureCharacter(sPen) + getKerning(sPen);

      if(cChar == ' ')
      {
         iLastSpaceIndex = (int)sPen.mCharIndex;
         fLineWidthAtLastSpace = fCurrentLineWidth;
         fLastSpaceCharWidth = fCharWidth;
      }

      fCurrentLineWidth += fCharWidth;

      if(fLineWidth > GE_EPSILON && fCurrentLineWidth > fLineWidth && iLastSpaceIndex >= 0)
      {
         vLineWidths.push_back(fLineWidthAtLastSpace);
         vLineFeedIndices.push_back((uint)iLastSpaceIndex);

         const uint iLineIndex = (uint)vLineFeedIndices.size() - 1;
         const uint iLineCharsCount = iLineIndex == 0
            ? vLineFeedIndices[0]
            : vLineFeedIndices[iLineIndex] - vLineFeedIndices[iLineIndex - 1] - 1;

         vLineJustifySpaces.push_back(iLineCharsCount - 1);

         fCurrentLineWidth -= fLineWidthAtLastSpace + fLastSpaceCharWidth;
         iLastSpaceIndex = -1;
      }
   }

   vLineWidths.push_back(fCurrentLineWidth);
   vLineFeedIndices.push_back(iTextLength);
   vLineJustifySpaces.push_back(0);

   float fPosX;
   float fPosY;

   const bool bJustifyText = GEHasFlag(eSettings, LabelSettingsBitMask::Justify);
   float fExtraLineWidth = 0.0f;

   if(bJustifyText && fLineWidth > GE_EPSILON && vLineJustifySpaces[0] > 0)
   {
      fExtraLineWidth = fLineWidth - vLineWidths[0];
   }

   switch(iAlignment)
   {
   case Alignment::TopLeft:
   case Alignment::MiddleLeft:
   case Alignment::BottomLeft:
      fPosX = 0.0f;
      break;
   case Alignment::None:
   case Alignment::TopCenter:
   case Alignment::MiddleCenter:
   case Alignment::BottomCenter:
      fPosX = -((vLineWidths[0] + fExtraLineWidth) * 0.5f);
      break;
   case Alignment::TopRight:
   case Alignment::MiddleRight:
   case Alignment::BottomRight:
      fPosX = -(vLineWidths[0] + fExtraLineWidth);
      break;
   default:
      break;
   }

   const float fFontOffsetY = (cFont->getOffsetYMin() + cFont->getOffsetYMax()) * 0.5f * fFontSize * FontSizeScale;
   const float fHalfFontOffsetY = fFontOffsetY * 0.5f;
   const float fLineHeight = fFontOffsetY + fVerticalSpacing;

   switch(iAlignment)
   {
   case Alignment::TopLeft:
   case Alignment::TopRight:
   case Alignment::TopCenter:
      fPosY = fHalfFontOffsetY - fFontOffsetY;
      break;
   case Alignment::None:
   case Alignment::MiddleLeft:
   case Alignment::MiddleRight:
   case Alignment::MiddleCenter:
      fPosY = fHalfFontOffsetY;
      fPosY += (vLineFeedIndices.size() - 1) * fLineHeight * 0.5f;
      break;
   case Alignment::BottomLeft:
   case Alignment::BottomRight:
   case Alignment::BottomCenter:
      fPosY = fHalfFontOffsetY + fFontOffsetY;
      fPosY += (vLineFeedIndices.size() - 1) * fLineHeight;
      break;
   default:
      break;
   }

   uint iCurrentLineIndex = 0;

   sPen.mColor = cColor;
   sPen.mFontSize = fFontSize;
   sPen.mYOffset = 0.0f;
   sPen.mCharSet = mFontCharSetIndex;
   sPen.mCharIndex = 0;

   vVertexData.clear();
   vIndices.clear();
   sGeometryData.NumVertices = 0;

   for(sPen.mCharIndex = 0; sPen.mCharIndex < iTextLength; sPen.mCharIndex++)
   {
      if(sPen.mCharIndex == vLineFeedIndices[iCurrentLineIndex])
      {
         iCurrentLineIndex++;

         fPosY -= fLineHeight;

         fExtraLineWidth = 0.0f;

         if(bJustifyText && fLineWidth > GE_EPSILON && vLineJustifySpaces[iCurrentLineIndex] > 0)
         {
            fExtraLineWidth = fLineWidth - vLineWidths[iCurrentLineIndex];
         }

         switch(iAlignment)
         {
         case Alignment::TopLeft:
         case Alignment::MiddleLeft:
         case Alignment::BottomLeft:
            fPosX = 0.0f;
            break;
         case Alignment::None:
         case Alignment::TopCenter:
         case Alignment::MiddleCenter:
         case Alignment::BottomCenter:
            fPosX = -((vLineWidths[iCurrentLineIndex] + fExtraLineWidth) * 0.5f);
            break;
         case Alignment::TopRight:
         case Alignment::MiddleRight:
         case Alignment::BottomRight:
            fPosX = -(vLineWidths[iCurrentLineIndex] + fExtraLineWidth);
            break;
         default:
            break;
         }

         continue;
      }

      if(bRichTextSupport)
      {
         evaluateRichTextTag(&sPen);

         if(sPen.mCharIndex >= iTextLength)
            break;
      }

      unsigned char cCurrentChar = sText[sPen.mCharIndex];
      const Glyph& sGlyph = cFont->getGlyph(sPen.mCharSet, cCurrentChar);
      const float fCharacterSize = sPen.mFontSize * FontSizeScale;

      float fAdvanceX = measureCharacter(sPen);

      if(cCurrentChar != ' ')
      {
         fPosX += getKerning(sPen);

         const float fGlyphWidth = sGlyph.Width * fCharacterSize;
         const float fGlyphHeight = sGlyph.Height * fCharacterSize;

         const float fHalfGlyphWidth = fGlyphWidth * 0.5f;
         const float fHalfGlyphHeight = fGlyphHeight * 0.5f;
         const float fHalfGlyphOffsetX = (sGlyph.OffsetX * fCharacterSize) * 0.5f;
         const float fHalfGlyphOffsetY = (sGlyph.OffsetY * fCharacterSize) * 0.5f;
         const float fHalfAdvanceX = fAdvanceX * 0.5f;

         vVertexData.push_back(fPosX - fHalfGlyphWidth + fHalfGlyphOffsetX + fHalfAdvanceX);
         vVertexData.push_back(fPosY - fHalfGlyphHeight - fHalfGlyphOffsetY + sPen.mYOffset);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sPen.mColor.Red);
         vVertexData.push_back(sPen.mColor.Green);
         vVertexData.push_back(sPen.mColor.Blue);
         vVertexData.push_back(sPen.mColor.Alpha);
         vVertexData.push_back(sGlyph.UV.U0); vVertexData.push_back(sGlyph.UV.V1);

         vVertexData.push_back(fPosX + fHalfGlyphWidth + fHalfGlyphOffsetX + fHalfAdvanceX);
         vVertexData.push_back(fPosY - fHalfGlyphHeight - fHalfGlyphOffsetY + sPen.mYOffset);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sPen.mColor.Red);
         vVertexData.push_back(sPen.mColor.Green);
         vVertexData.push_back(sPen.mColor.Blue);
         vVertexData.push_back(sPen.mColor.Alpha);
         vVertexData.push_back(sGlyph.UV.U1); vVertexData.push_back(sGlyph.UV.V1);

         vVertexData.push_back(fPosX - fHalfGlyphWidth + fHalfGlyphOffsetX + fHalfAdvanceX);
         vVertexData.push_back(fPosY + fHalfGlyphHeight - fHalfGlyphOffsetY + sPen.mYOffset);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sPen.mColor.Red);
         vVertexData.push_back(sPen.mColor.Green);
         vVertexData.push_back(sPen.mColor.Blue);
         vVertexData.push_back(sPen.mColor.Alpha);
         vVertexData.push_back(sGlyph.UV.U0); vVertexData.push_back(sGlyph.UV.V0);

         vVertexData.push_back(fPosX + fHalfGlyphWidth + fHalfGlyphOffsetX + fHalfAdvanceX);
         vVertexData.push_back(fPosY + fHalfGlyphHeight - fHalfGlyphOffsetY + sPen.mYOffset);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sPen.mColor.Red);
         vVertexData.push_back(sPen.mColor.Green);
         vVertexData.push_back(sPen.mColor.Blue);
         vVertexData.push_back(sPen.mColor.Alpha);
         vVertexData.push_back(sGlyph.UV.U1); vVertexData.push_back(sGlyph.UV.V0);

         vIndices.push_back(sGeometryData.NumVertices);
         vIndices.push_back(sGeometryData.NumVertices + 1);
         vIndices.push_back(sGeometryData.NumVertices + 2);
         vIndices.push_back(sGeometryData.NumVertices + 3);
         vIndices.push_back(sGeometryData.NumVertices + 2);
         vIndices.push_back(sGeometryData.NumVertices + 1);

         sGeometryData.NumVertices += 4;
      }

      if(bJustifyText && fLineWidth > GE_EPSILON && vLineJustifySpaces[iCurrentLineIndex] > 0)
      {
         fAdvanceX += (fLineWidth - vLineWidths[iCurrentLineIndex]) / vLineJustifySpaces[iCurrentLineIndex];
      }

      fPosX += fAdvanceX;
   }

   sGeometryData.NumIndices = (uint)vIndices.size();

   if(sGeometryData.NumIndices > 0)
   {
      sGeometryData.VertexData = &vVertexData[0];
      sGeometryData.Indices = &vIndices[0];
   }
   else
   {
      sGeometryData.VertexData = 0;
      sGeometryData.Indices = 0;
   }
}

float ComponentLabel::measureCharacter(const Pen& pPen)
{
   GEAssert(pPen.mCharIndex < sText.length());

   unsigned char cChar = sText[pPen.mCharIndex];
   const Glyph& sGlyph = cFont->getGlyph(pPen.mCharSet, cChar);
   const float fCharacterSize = pPen.mFontSize * FontSizeScale;

   return (sGlyph.AdvanceX * fCharacterSize) + fHorizontalSpacing;
}

float ComponentLabel::getKerning(const Pen& pPen)
{
   GEAssert(pPen.mCharIndex < sText.length());

   unsigned char cChar = sText[pPen.mCharIndex];
   float fKerning = 0.0f;

   if(cChar != ' ')
   {
      fKerning = pPen.mCharIndex > 0
         ? cFont->getKerning(pPen.mCharSet, sText[pPen.mCharIndex - 1], cChar)
         : 0.0f;

      const float fCharacterSize = pPen.mFontSize * FontSizeScale;
      fKerning *= fCharacterSize;
   }

   return fKerning;
}

Font* ComponentLabel::getFont() const
{
   return cFont;
}

const ObjectName& ComponentLabel::getFontName() const
{
   return cFont ? cFont->getName() : ObjectName::Empty;
}

const ObjectName& ComponentLabel::getFontCharacterSet() const
{
   return cFont ? cFont->getFontCharacterSet(mFontCharSetIndex)->getName() : ObjectName::Empty;
}

float ComponentLabel::getFontSize() const
{
   return fFontSize;
}

Alignment ComponentLabel::getAlignment() const
{
   return iAlignment;
}

const char* ComponentLabel::getText() const
{
   return sText.c_str();
}

const ObjectName& ComponentLabel::getStringID() const
{
   return cStringID;
}

float ComponentLabel::getHorizontalSpacing() const
{
   return fHorizontalSpacing;
}

float ComponentLabel::getVerticalSpacing() const
{
   return fVerticalSpacing;
}

float ComponentLabel::getLineWidth() const
{
   return fLineWidth;
}

uint8_t ComponentLabel::getSettings() const
{
   return eSettings;
}

void ComponentLabel::setFont(Font* TextFont)
{
   cFont = TextFont;
}

void ComponentLabel::setFontName(const Core::ObjectName& FontName)
{
   cFont = RenderSystem::getInstance()->getFont(FontName);

#if defined (GE_EDITOR_SUPPORT)
   const ObjectName propertyName = ObjectName("FontCharacterSet");
   Property* cProperty = const_cast<Property*>(getProperty(propertyName));
   cProperty->DataPtr = cFont ? (void*)cFont->getCharacterSetRegistry() : 0;

   EventArgs sArgs;
   sArgs.Data = cOwner;
   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
#endif

   if(!cFont)
   {
      Log::log(LogType::Warning, "No font found in '%s'. The entity will not be rendered.", cOwner->getFullName().getString());
      hide();
      return;
   }

   if(!sText.empty())
   {
      generateVertexData();
   }
}

void ComponentLabel::setFontCharacterSet(const ObjectName& pCharSetName)
{
   if(pCharSetName == getFontCharacterSet())
      return;

   mFontCharSetIndex = 0;

   if(cFont)
   {
      mFontCharSetIndex = cFont->getCharacterSetIndex(pCharSetName);
      generateVertexData();
   }
}

void ComponentLabel::setFontSize(float FontSize)
{
   fFontSize = FontSize;

   if(!sText.empty())
   {
      generateVertexData();
   }
}

void ComponentLabel::setAlignment(Alignment Align)
{
   iAlignment = Align;

   if(!sText.empty())
   {
      generateVertexData();
   }
}

void ComponentLabel::setText(const char* Text)
{
   sText.clear();
   const uint iStrLength = (uint)strlen(Text);

   for(uint i = 0; i < iStrLength; i++)
   {
      // UTF-8 Latin character
      if((unsigned char)Text[i] == 0xc3)
      {
         unsigned char cFontCharIndex = Text[++i] - 0x80 + 0xc0;
         sText.push_back(cFontCharIndex);
      }
      // standard character
      else
      {
         sText.push_back(Text[i]);
      }
   }

   if(GEHasFlag(eSettings, LabelSettingsBitMask::VariableReplacement))
   {
      processVariables();
   }

   generateVertexData();
}

void ComponentLabel::setStringID(const ObjectName& StringID)
{
   cStringID = StringID;
   LocalizedString* cLocaString = 0;
    
   if(!StringID.isEmpty())
   {
      cLocaString = LocalizedStringsManager::getInstance()->get(StringID);

      if(cLocaString)
      {
         setText(cLocaString->getString());
      }
   }
}

void ComponentLabel::setHorizontalSpacing(float HorizontalSpacing)
{
   fHorizontalSpacing = HorizontalSpacing;

   if(!sText.empty())
   {
      generateVertexData();
   }
}

void ComponentLabel::setVerticalSpacing(float VerticalSpacing)
{
   fVerticalSpacing = VerticalSpacing;

   if(!sText.empty())
   {
      generateVertexData();
   }
}

void ComponentLabel::setLineWidth(float LineWidth)
{
   fLineWidth = LineWidth;

   if(!sText.empty())
   {
      generateVertexData();
   }
}

void ComponentLabel::setSettings(uint8_t Settings)
{
   eSettings = Settings;

   if(!sText.empty())
   {
      generateVertexData();
   }
}
