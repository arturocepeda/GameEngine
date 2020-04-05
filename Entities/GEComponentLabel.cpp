
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
const float kFontSizeScale = 0.0001f;
const unsigned char kLineFeedChar = '~';

static void utf8ToUnicode(const char* pSequence, int* pOutUnicode, int* pOutExtraChars)
{
   int sequenceChar = pSequence[0] & 0x000000ff;
   int unicode = 0;
   int extraChars = 0;

   if(sequenceChar < 0x80)
   {
      // ASCII character
      unicode = sequenceChar;
   }
   else
   {
      // non-ASCII character
      while(sequenceChar & 0x40)
      {
         // multi-byte symbol
         extraChars++;
         const int nextSequenceChar = pSequence[extraChars] & 0x000000ff;
         unicode = (unicode << 6) | (nextSequenceChar & 0x3f);
         sequenceChar <<= 1;
      }

      unicode |= (sequenceChar & 0x7f) << (extraChars * 5);
   }

   *pOutUnicode = unicode;
   *pOutExtraChars = extraChars;
}

const ObjectName ComponentLabel::ClassName = ObjectName("Label");

ComponentLabel::ComponentLabel(Entity* Owner)
   : ComponentRenderable(Owner)
   , cFont(nullptr)
   , mFontCharSetIndex(0u)
   , fFontSize(12.0f)
   , iAlignment(Alignment::MiddleCenter)
   , mLayer(SpriteLayer::GUI)
   , eSettings(0u)
   , fHorizontalSpacing(0.0f)
   , fVerticalSpacing(0.0f)
   , fLineWidth(0.0f)
   , mTextWidth(0.0f)
   , mTextLength(0u)
   , mCharacterCountLimit(0u)
   , mFontResizeFactor(1.0f)
{
   mClassNames.push_back(ClassName);

   sGeometryData.VertexStride = (3 + 4 + 2) * sizeof(float);

   cOwner->connectEventCallback(Events::RenderableColorChanged, this, [this](const EventArgs* args) -> bool
   {
      if(!sText.empty())
      {
         generateVertexData();
      }

      return false;
   });

   EventHandlingObject::connectStaticEventCallback(Events::LocalizedStringsReloaded, this, [this](const EventArgs* args) -> bool
   {
      if(!cStringID.isEmpty())
      {
         setStringID(cStringID);
      }

      return false;
   });

   GERegisterProperty(ObjectName, FontName);
   GERegisterProperty(ObjectName, FontCharacterSet);
   GERegisterProperty(Float, FontSize);
   GERegisterPropertyEnum(Alignment, Alignment);
   GERegisterProperty(Float, HorizontalSpacing);
   GERegisterProperty(Float, VerticalSpacing);
   GERegisterProperty(Float, LineWidth);
   GERegisterProperty(UInt, CharacterCountLimit);
   GERegisterPropertyEnum(SpriteLayer, Layer);
   GERegisterPropertyBitMask(LabelSettingsBitMask, Settings);
   GERegisterProperty(String, Text);
   GERegisterProperty(ObjectName, StringID);
   GERegisterPropertyReadonly(UInt, TextLength);
   GERegisterPropertyReadonly(Float, TextWidth);
}

ComponentLabel::~ComponentLabel()
{
   cOwner->disconnectEventCallback(Events::RenderableColorChanged, this);

   EventHandlingObject::disconnectStaticEventCallback(Events::LocalizedStringsReloaded, this);
}

uint16_t ComponentLabel::getGlyphIndex(size_t pCharIndex)
{
   GEAssert(pCharIndex < sText.length());

   return
      ((uint16_t)sText[pCharIndex] & 0x00ff) |
      (((uint16_t)sTextExtension[pCharIndex] & 0x00ff) << 8);
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
      {
         return;
      }

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
            pPen->mFontSize = fFontSize * mFontResizeFactor;
         }
         else
         {
            pPen->mFontSize = (float)strtod(value, 0) * mFontResizeFactor;
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
   mTextLength = 0u;
   mTextWidth = 0.0f;

   if(!cFont)
   {
      return;
   }

   Pen sPen;
   sPen.mColor = cColor;
   sPen.mFontSize = fFontSize;
   sPen.mYOffset = 0.0f;
   sPen.mCharSet = mFontCharSetIndex;
   sPen.mCharIndex = 0;

   const uint32_t iTextLength = (uint32_t)sText.length();

   const bool bJustifyText = GEHasFlag(eSettings, LabelSettingsBitMask::Justify);
   const bool bRichTextSupport = GEHasFlag(eSettings, LabelSettingsBitMask::RichTextSupport);
   const bool bFixSizeToLineWidth = GEHasFlag(eSettings, LabelSettingsBitMask::FitSizeToLineWidth);

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
         {
            break;
         }
      }

      const uint8_t cChar = sText[sPen.mCharIndex];

      if(cChar == kLineFeedChar)
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

      if(fLineWidth > GE_EPSILON &&
         !bFixSizeToLineWidth &&
         fCurrentLineWidth > fLineWidth &&
         iLastSpaceIndex >= 0)
      {
         vLineWidths.push_back(fLineWidthAtLastSpace);
         vLineFeedIndices.push_back((uint32_t)iLastSpaceIndex);

         if(bJustifyText)
         {
            const uint32_t iLineIndex = (uint32_t)vLineFeedIndices.size() - 1;

            const uint32_t iLineFirstCharIndex = iLineIndex == 0 ? 0 : vLineFeedIndices[iLineIndex - 1] + 1;
            const uint32_t iLineLastCharIndex = vLineFeedIndices[iLineIndex];
            uint32_t iLineCharsCount = iLineLastCharIndex - iLineFirstCharIndex;
         
            if(bRichTextSupport)
            {
               bool bInRichTag = false;

               for(uint32_t i = iLineFirstCharIndex; i <= iLineLastCharIndex; i++)
               {
                  if(!bInRichTag && sText[i] == '<')
                  {
                     bInRichTag = true;
                  }

                  if(bInRichTag)
                  {
                     iLineCharsCount--;
                  }

                  if(bInRichTag && sText[i] == '>')
                  {
                     bInRichTag = false;
                  }
               }
            }

            vLineJustifySpaces.push_back(iLineCharsCount - 1);
         }

         fCurrentLineWidth -= fLineWidthAtLastSpace + fLastSpaceCharWidth;
         iLastSpaceIndex = -1;
      }
   }

   vLineWidths.push_back(fCurrentLineWidth);
   vLineFeedIndices.push_back(iTextLength);
   vLineJustifySpaces.push_back(0u);

   mFontResizeFactor = 1.0f;

   for(size_t i = 0u; i < vLineWidths.size(); i++)
   {
      if(vLineWidths[i] > mTextWidth)
      {
         mTextWidth = vLineWidths[i];
      }
   }

   if(fLineWidth > GE_EPSILON && bFixSizeToLineWidth)
   {
      for(size_t i = 0u; i < vLineWidths.size(); i++)
      {
         if(vLineWidths[i] > fLineWidth)
         {
            vLineWidths[i] = fLineWidth;
         }
      }

      if(mTextWidth > fLineWidth)
      {
         mFontResizeFactor = fLineWidth / mTextWidth;
         sPen.mFontSize *= mFontResizeFactor;
      }
   }

   float fPosX;
   float fPosY;

   float fExtraLineWidth = 0.0f;

   if(bJustifyText && fLineWidth > GE_EPSILON && vLineJustifySpaces[0] > 0u)
   {
      fExtraLineWidth = fLineWidth - vLineWidths[0];
      mTextWidth = fLineWidth;
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

   const float fFontOffsetY =
      cFont->getLineHeight(mFontCharSetIndex) * fFontSize * kFontSizeScale * mFontResizeFactor;
   const float fHalfFontOffsetY = fFontOffsetY * 0.5f;
   const float fLineHeight = fFontOffsetY + fVerticalSpacing;

   switch(iAlignment)
   {
   case Alignment::TopLeft:
   case Alignment::TopRight:
   case Alignment::TopCenter:
      fPosY = -fLineHeight * 0.5f;
      break;
   case Alignment::None:
   case Alignment::MiddleLeft:
   case Alignment::MiddleRight:
   case Alignment::MiddleCenter:
      fPosY = (vLineFeedIndices.size() - 1) * fLineHeight * 0.5f;
      break;
   case Alignment::BottomLeft:
   case Alignment::BottomRight:
   case Alignment::BottomCenter:
      fPosY = ((float)vLineFeedIndices.size() - 0.5f) * fLineHeight;
      break;
   default:
      break;
   }

   uint32_t iCurrentCharIndex = 0u;
   uint32_t iCurrentLineIndex = 0u;

   sPen.mColor = cColor;
   sPen.mFontSize = fFontSize * mFontResizeFactor;
   sPen.mYOffset = 0.0f;
   sPen.mCharSet = mFontCharSetIndex;
   sPen.mCharIndex = 0u;

   vVertexData.clear();
   vIndices.clear();
   sGeometryData.NumVertices = 0u;

   for(sPen.mCharIndex = 0u; sPen.mCharIndex < iTextLength; sPen.mCharIndex++)
   {
      if(bRichTextSupport)
      {
         evaluateRichTextTag(&sPen);

         if(sPen.mCharIndex >= iTextLength)
         {
            break;
         }
      }

      if(sPen.mCharIndex == vLineFeedIndices[iCurrentLineIndex])
      {
         iCurrentLineIndex++;

         fPosY -= fLineHeight;

         fExtraLineWidth = 0.0f;

         if(bJustifyText && fLineWidth > GE_EPSILON && vLineJustifySpaces[iCurrentLineIndex] > 0u)
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

      float fAdvanceX = measureCharacter(sPen);

      if(sText[sPen.mCharIndex] != ' ')
      {
         const uint16_t iGlyphIndex = getGlyphIndex((size_t)sPen.mCharIndex);
         const Glyph& sGlyph = cFont->getGlyph(sPen.mCharSet, iGlyphIndex);
         const float fCharacterSize = sPen.mFontSize * kFontSizeScale;

         fPosX += getKerning(sPen);

         const float glyphWidth = sGlyph.Width * fCharacterSize;
         const float glyphHeight = sGlyph.Height * fCharacterSize;

         const float glyphOffsetX = sGlyph.OffsetX * fCharacterSize;
         const float glyphOffsetY = sGlyph.OffsetY * fCharacterSize;

         vVertexData.push_back(fPosX + glyphOffsetX);
         vVertexData.push_back(fPosY + glyphOffsetY - glyphHeight);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sPen.mColor.Red);
         vVertexData.push_back(sPen.mColor.Green);
         vVertexData.push_back(sPen.mColor.Blue);
         vVertexData.push_back(sPen.mColor.Alpha);
         vVertexData.push_back(sGlyph.UV.U0); vVertexData.push_back(sGlyph.UV.V1);

         vVertexData.push_back(fPosX + glyphOffsetX + glyphWidth);
         vVertexData.push_back(fPosY + glyphOffsetY - glyphHeight);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sPen.mColor.Red);
         vVertexData.push_back(sPen.mColor.Green);
         vVertexData.push_back(sPen.mColor.Blue);
         vVertexData.push_back(sPen.mColor.Alpha);
         vVertexData.push_back(sGlyph.UV.U1); vVertexData.push_back(sGlyph.UV.V1);

         vVertexData.push_back(fPosX + glyphOffsetX);
         vVertexData.push_back(fPosY + glyphOffsetY);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sPen.mColor.Red);
         vVertexData.push_back(sPen.mColor.Green);
         vVertexData.push_back(sPen.mColor.Blue);
         vVertexData.push_back(sPen.mColor.Alpha);
         vVertexData.push_back(sGlyph.UV.U0); vVertexData.push_back(sGlyph.UV.V0);

         vVertexData.push_back(fPosX + glyphOffsetX + glyphWidth);
         vVertexData.push_back(fPosY + glyphOffsetY);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sPen.mColor.Red);
         vVertexData.push_back(sPen.mColor.Green);
         vVertexData.push_back(sPen.mColor.Blue);
         vVertexData.push_back(sPen.mColor.Alpha);
         vVertexData.push_back(sGlyph.UV.U1); vVertexData.push_back(sGlyph.UV.V0);

         vIndices.push_back(sGeometryData.NumVertices);
         vIndices.push_back(sGeometryData.NumVertices + 1u);
         vIndices.push_back(sGeometryData.NumVertices + 2u);
         vIndices.push_back(sGeometryData.NumVertices + 3u);
         vIndices.push_back(sGeometryData.NumVertices + 2u);
         vIndices.push_back(sGeometryData.NumVertices + 1u);

         sGeometryData.NumVertices += 4u;
      }

      iCurrentCharIndex++;

      if(iCurrentCharIndex == mCharacterCountLimit)
      {
         break;
      }

      if(bJustifyText && fLineWidth > GE_EPSILON && vLineJustifySpaces[iCurrentLineIndex] > 0u)
      {
         fAdvanceX += (fLineWidth - vLineWidths[iCurrentLineIndex]) / vLineJustifySpaces[iCurrentLineIndex];
      }

      fPosX += fAdvanceX;
   }

   sGeometryData.NumIndices = (uint32_t)vIndices.size();

   if(sGeometryData.NumIndices > 0u)
   {
      sGeometryData.VertexData = &vVertexData[0];
      sGeometryData.Indices = &vIndices[0];
   }
   else
   {
      sGeometryData.VertexData = nullptr;
      sGeometryData.Indices = nullptr;
   }

   mTextLength = iCurrentCharIndex;
}

float ComponentLabel::measureCharacter(const Pen& pPen)
{
   GEAssert(pPen.mCharIndex < sText.length());

   const uint16_t iGlyphIndex = getGlyphIndex(pPen.mCharIndex);
   const Glyph& sGlyph = cFont->getGlyph(pPen.mCharSet, iGlyphIndex);
   const float fCharacterSize = pPen.mFontSize * kFontSizeScale;

   return (sGlyph.AdvanceX * fCharacterSize) + fHorizontalSpacing;
}

float ComponentLabel::getKerning(const Pen& pPen)
{
   GEAssert(pPen.mCharIndex < sText.length());

   unsigned char cChar = sText[pPen.mCharIndex];
   float fKerning = 0.0f;

   if(cChar != ' ' && pPen.mCharIndex > 0u)
   {
      const uint16_t previousGlyphIndex = getGlyphIndex(pPen.mCharIndex - 1u);
      const uint16_t currentGlyphIndex = getGlyphIndex(pPen.mCharIndex);
      fKerning = cFont->getKerning(pPen.mCharSet, previousGlyphIndex, currentGlyphIndex);

      const float fCharacterSize = pPen.mFontSize * kFontSizeScale;
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

float ComponentLabel::getTextWidth() const
{
   return mTextWidth;
}

uint32_t ComponentLabel::getCharacterCountLimit() const
{
   return mCharacterCountLimit;
}

SpriteLayer ComponentLabel::getLayer() const
{
   return mLayer;
}

uint8_t ComponentLabel::getSettings() const
{
   return eSettings;
}

uint32_t ComponentLabel::getTextLength() const
{
   return mTextLength;
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
   {
      return;
   }

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
   sTextExtension.clear();

   const uint32_t iStrLength = (uint32_t)strlen(Text);

   for(uint32_t i = 0u; i < iStrLength; i++)
   {
      int glyphIndex = 0;
      int extraChars = 0;
      utf8ToUnicode(Text + i, &glyphIndex, &extraChars);

      sText.push_back(glyphIndex & 0x000000ff);
      sTextExtension.push_back((glyphIndex & 0x0000ff00) >> 8);

      i += (uint32_t)extraChars;
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

void ComponentLabel::setCharacterCountLimit(uint32_t Limit)
{
   mCharacterCountLimit = Limit;

   if(!sText.empty())
   {
      generateVertexData();
   }
}

void ComponentLabel::setLayer(SpriteLayer pLayer)
{
   mLayer = pLayer;
}

void ComponentLabel::setSettings(uint8_t Settings)
{
   eSettings = Settings;

   if(!sText.empty())
   {
      generateVertexData();
   }
}
