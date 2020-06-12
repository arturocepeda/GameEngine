
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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


//
//  ComponentLabelBase
//
ComponentLabelBase::ComponentLabelBase(Entity* pOwner)
   : ComponentRenderable(pOwner)
   , mFontSize(12.0f)
   , mFontResizeFactor(1.0f)
   , mAlignment(Alignment::MiddleCenter)
   , mLayer(SpriteLayer::GUI)
   , mSettings(0u)
   , mTextLength(0u)
   , mCharacterCountLimit(0u)
{
   cOwner->connectEventCallback(Events::RenderableColorChanged, this, [this](const EventArgs* args) -> bool
   {
      if(!mText.empty())
      {
         generateText();
      }

      return false;
   });

   EventHandlingObject::connectStaticEventCallback(Events::LocalizedStringsReloaded, this, [this](const EventArgs* args) -> bool
   {
      if(!mStringID.isEmpty())
      {
         setStringID(mStringID);
      }

      return false;
   });
}

ComponentLabelBase::~ComponentLabelBase()
{
}

uint16_t ComponentLabelBase::getGlyphIndex(size_t pCharIndex)
{
   GEAssert(pCharIndex < mText.length());

   return
      ((uint16_t)mText[pCharIndex] & 0x00ff) |
      (((uint16_t)mTextExtension[pCharIndex] & 0x00ff) << 8);
}

void ComponentLabelBase::evaluateRichTextTag(Pen* pPen)
{
   unsigned char character = mText[pPen->mCharIndex];

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

      while(i < mText.length() && mText[i] != '>')
      {
         character = mText[i++];

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

      if(i == mText.length())
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
            pPen->mFontSize = mFontSize * mFontResizeFactor;
         }
         else
         {
            pPen->mFontSize = (float)strtod(value, 0) * mFontResizeFactor;
         }
      }
      // style
      else if(strcmp(tag, "style") == 0)
      {
         if(tagClosing)
         {
            pPen->mFontStyle = mFontStyle;
         }
         else
         {
            pPen->mFontStyle = ObjectName(value);
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

      if(pPen->mCharIndex < (uint32_t)mText.size())
      {
         character = mText[pPen->mCharIndex];
      }
   }
}

void ComponentLabelBase::processVariables()
{
   size_t iVariableStartPos = mText.find('$');

   while(iVariableStartPos != GESTLString::npos)
   {
      size_t iVariableEndPos = iVariableStartPos + 1;

      while(iVariableEndPos < mText.length() &&
         (isalnum(mText[iVariableEndPos]) || mText[iVariableEndPos] == '_'))
      {
         iVariableEndPos++;
      }

      size_t iVariableNameLength = iVariableEndPos - iVariableStartPos - 1;

      if(iVariableNameLength > 0)
      {
         GESTLString sVariableName = mText.substr(iVariableStartPos + 1, iVariableNameLength);
         ObjectName cVariableName = ObjectName(sVariableName.c_str());
         const char* sVariableValue = LocalizedStringsManager::getInstance()->getVariable(cVariableName);

         if(sVariableValue)
         {
            mText.replace(iVariableStartPos, iVariableNameLength + 1, sVariableValue);
         }
      }

      iVariableStartPos = mText.find('$', iVariableStartPos + 1);
   }
}

float ComponentLabelBase::getFontSize() const
{
   return mFontSize;
}

Alignment ComponentLabelBase::getAlignment() const
{
   return mAlignment;
}

const char* ComponentLabelBase::getText() const
{
   return mText.c_str();
}

const ObjectName& ComponentLabelBase::getStringID() const
{
   return mStringID;
}

uint32_t ComponentLabelBase::getCharacterCountLimit() const
{
   return mCharacterCountLimit;
}

SpriteLayer ComponentLabelBase::getLayer() const
{
   return mLayer;
}

uint8_t ComponentLabelBase::getSettings() const
{
   return mSettings;
}

uint32_t ComponentLabelBase::getTextLength() const
{
   return mTextLength;
}

void ComponentLabelBase::setFontSize(float pFontSize)
{
   mFontSize = pFontSize;

   if(!mText.empty())
   {
      generateText();
   }
}

void ComponentLabelBase::setAlignment(Alignment pAlignment)
{
   mAlignment = pAlignment;

   if(!mText.empty())
   {
      generateText();
   }
}

void ComponentLabelBase::setText(const char* pText)
{
   mText.clear();
   mTextExtension.clear();

   const uint32_t iStrLength = (uint32_t)strlen(pText);

   for(uint32_t i = 0u; i < iStrLength; i++)
   {
      int glyphIndex = 0;
      int extraChars = 0;
      utf8ToUnicode(pText + i, &glyphIndex, &extraChars);

      mText.push_back(glyphIndex & 0x000000ff);
      mTextExtension.push_back((glyphIndex & 0x0000ff00) >> 8);

      i += (uint32_t)extraChars;
   }

   if(GEHasFlag(mSettings, LabelSettingsBitMask::VariableReplacement))
   {
      processVariables();
   }

   generateText();
}

void ComponentLabelBase::setStringID(const ObjectName& pStringID)
{
   mStringID = pStringID;
   LocalizedString* locaString = nullptr;
    
   if(!pStringID.isEmpty())
   {
      locaString = LocalizedStringsManager::getInstance()->get(pStringID);

      if(locaString)
      {
         setText(locaString->getString());
      }
   }
}

void ComponentLabelBase::setCharacterCountLimit(uint32_t pLimit)
{
   mCharacterCountLimit = pLimit;

   if(!mText.empty())
   {
      generateText();
   }
}

void ComponentLabelBase::setLayer(SpriteLayer pLayer)
{
   mLayer = pLayer;
}

void ComponentLabelBase::setSettings(uint8_t pSettings)
{
   mSettings = pSettings;

   if(!mText.empty())
   {
      generateText();
   }
}


//
//  ComponentLabel
//
const float kFontSizeScale = 0.0001f;
const unsigned char kLineFeedChar = '~';

const ObjectName ComponentLabel::ClassName = ObjectName("Label");

ComponentLabel::ComponentLabel(Entity* pOwner)
   : ComponentLabelBase(pOwner)
   , mFont(nullptr)
   , mHorizontalSpacing(0.0f)
   , mVerticalSpacing(0.0f)
   , mLineWidth(0.0f)
   , mTextWidth(0.0f)
{
   mClassNames.push_back(ClassName);

   sGeometryData.VertexStride = (3 + 4 + 2) * sizeof(float);

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

void ComponentLabel::generateText()
{
   mTextLength = 0u;
   mTextWidth = 0.0f;

   if(!mFont)
   {
      return;
   }

   Pen sPen;
   sPen.mColor = cColor;
   sPen.mFontSize = mFontSize;
   sPen.mYOffset = 0.0f;
   sPen.mFontStyle = mFontStyle;
   sPen.mCharIndex = 0;

   const uint32_t iTextLength = (uint32_t)mText.length();

   const bool bJustifyText = GEHasFlag(mSettings, LabelSettingsBitMask::Justify);
   const bool bRichTextSupport = GEHasFlag(mSettings, LabelSettingsBitMask::RichTextSupport);
   const bool bFixSizeToLineWidth = GEHasFlag(mSettings, LabelSettingsBitMask::FitSizeToLineWidth);

   mLineWidths.clear();
   mLineFeedIndices.clear();
   mLineJustifySpaces.clear();

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

      const uint8_t cChar = mText[sPen.mCharIndex];

      if(cChar == kLineFeedChar)
      {
         mLineWidths.push_back(fCurrentLineWidth);
         mLineFeedIndices.push_back(sPen.mCharIndex);
         mLineJustifySpaces.push_back(0);

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

      if(mLineWidth > GE_EPSILON &&
         !bFixSizeToLineWidth &&
         fCurrentLineWidth > mLineWidth &&
         iLastSpaceIndex >= 0)
      {
         mLineWidths.push_back(fLineWidthAtLastSpace);
         mLineFeedIndices.push_back((uint32_t)iLastSpaceIndex);

         if(bJustifyText)
         {
            const uint32_t iLineIndex = (uint32_t)mLineFeedIndices.size() - 1;

            const uint32_t iLineFirstCharIndex = iLineIndex == 0 ? 0 : mLineFeedIndices[iLineIndex - 1] + 1;
            const uint32_t iLineLastCharIndex = mLineFeedIndices[iLineIndex];
            uint32_t iLineCharsCount = iLineLastCharIndex - iLineFirstCharIndex;
         
            if(bRichTextSupport)
            {
               bool bInRichTag = false;

               for(uint32_t i = iLineFirstCharIndex; i <= iLineLastCharIndex; i++)
               {
                  if(!bInRichTag && mText[i] == '<')
                  {
                     bInRichTag = true;
                  }

                  if(bInRichTag)
                  {
                     iLineCharsCount--;
                  }

                  if(bInRichTag && mText[i] == '>')
                  {
                     bInRichTag = false;
                  }
               }
            }

            mLineJustifySpaces.push_back(iLineCharsCount - 1);
         }

         fCurrentLineWidth -= fLineWidthAtLastSpace + fLastSpaceCharWidth;
         iLastSpaceIndex = -1;
      }
   }

   mLineWidths.push_back(fCurrentLineWidth);
   mLineFeedIndices.push_back(iTextLength);
   mLineJustifySpaces.push_back(0u);

   mFontResizeFactor = 1.0f;

   for(size_t i = 0u; i < mLineWidths.size(); i++)
   {
      if(mLineWidths[i] > mTextWidth)
      {
         mTextWidth = mLineWidths[i];
      }
   }

   if(mLineWidth > GE_EPSILON && bFixSizeToLineWidth)
   {
      for(size_t i = 0u; i < mLineWidths.size(); i++)
      {
         if(mLineWidths[i] > mLineWidth)
         {
            mLineWidths[i] = mLineWidth;
         }
      }

      if(mTextWidth > mLineWidth)
      {
         mFontResizeFactor = mLineWidth / mTextWidth;
         sPen.mFontSize *= mFontResizeFactor;
      }
   }

   float fPosX;
   float fPosY;

   float fExtraLineWidth = 0.0f;

   if(bJustifyText && mLineWidth > GE_EPSILON && mLineJustifySpaces[0] > 0u)
   {
      fExtraLineWidth = mLineWidth - mLineWidths[0];
      mTextWidth = mLineWidth;
   }

   switch(mAlignment)
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
      fPosX = -((mLineWidths[0] + fExtraLineWidth) * 0.5f);
      break;
   case Alignment::TopRight:
   case Alignment::MiddleRight:
   case Alignment::BottomRight:
      fPosX = -(mLineWidths[0] + fExtraLineWidth);
      break;
   default:
      break;
   }

   const uint32_t fontCharSetIndex = mFont->getCharacterSetIndex(mFontStyle);
   const float fFontOffsetY =
      mFont->getLineHeight(fontCharSetIndex) * mFontSize * kFontSizeScale * mFontResizeFactor;
   const float fHalfFontOffsetY = fFontOffsetY * 0.5f;
   const float fLineHeight = fFontOffsetY + mVerticalSpacing;

   switch(mAlignment)
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
      fPosY = (mLineFeedIndices.size() - 1) * fLineHeight * 0.5f;
      break;
   case Alignment::BottomLeft:
   case Alignment::BottomRight:
   case Alignment::BottomCenter:
      fPosY = ((float)mLineFeedIndices.size() - 0.5f) * fLineHeight;
      break;
   default:
      break;
   }

   uint32_t iCurrentCharIndex = 0u;
   uint32_t iCurrentLineIndex = 0u;

   sPen.mFontStyle = mFontStyle;
   sPen.mColor = cColor;
   sPen.mFontSize = mFontSize * mFontResizeFactor;
   sPen.mYOffset = 0.0f;
   sPen.mCharIndex = 0u;

   mVertexData.clear();
   mIndices.clear();
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

      if(sPen.mCharIndex == mLineFeedIndices[iCurrentLineIndex])
      {
         iCurrentLineIndex++;

         fPosY -= fLineHeight;

         fExtraLineWidth = 0.0f;

         if(bJustifyText && mLineWidth > GE_EPSILON && mLineJustifySpaces[iCurrentLineIndex] > 0u)
         {
            fExtraLineWidth = mLineWidth - mLineWidths[iCurrentLineIndex];
         }

         switch(mAlignment)
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
            fPosX = -((mLineWidths[iCurrentLineIndex] + fExtraLineWidth) * 0.5f);
            break;
         case Alignment::TopRight:
         case Alignment::MiddleRight:
         case Alignment::BottomRight:
            fPosX = -(mLineWidths[iCurrentLineIndex] + fExtraLineWidth);
            break;
         default:
            break;
         }

         continue;
      }

      float fAdvanceX = measureCharacter(sPen);

      if(mText[sPen.mCharIndex] != ' ')
      {
         const uint16_t iGlyphIndex = getGlyphIndex((size_t)sPen.mCharIndex);
         const uint32_t penCharSetIndex = mFont->getCharacterSetIndex(sPen.mFontStyle);
         const Glyph& sGlyph = mFont->getGlyph(penCharSetIndex, iGlyphIndex);
         const float fCharacterSize = sPen.mFontSize * kFontSizeScale;

         fPosX += getKerning(sPen);

         const float glyphWidth = sGlyph.Width * fCharacterSize;
         const float glyphHeight = sGlyph.Height * fCharacterSize;

         const float glyphOffsetX = sGlyph.OffsetX * fCharacterSize;
         const float glyphOffsetY = sGlyph.OffsetY * fCharacterSize;

         mVertexData.push_back(fPosX + glyphOffsetX);
         mVertexData.push_back(fPosY + glyphOffsetY - glyphHeight);
         mVertexData.push_back(0.0f);
         mVertexData.push_back(sPen.mColor.Red);
         mVertexData.push_back(sPen.mColor.Green);
         mVertexData.push_back(sPen.mColor.Blue);
         mVertexData.push_back(sPen.mColor.Alpha);
         mVertexData.push_back(sGlyph.UV.U0); mVertexData.push_back(sGlyph.UV.V1);

         mVertexData.push_back(fPosX + glyphOffsetX + glyphWidth);
         mVertexData.push_back(fPosY + glyphOffsetY - glyphHeight);
         mVertexData.push_back(0.0f);
         mVertexData.push_back(sPen.mColor.Red);
         mVertexData.push_back(sPen.mColor.Green);
         mVertexData.push_back(sPen.mColor.Blue);
         mVertexData.push_back(sPen.mColor.Alpha);
         mVertexData.push_back(sGlyph.UV.U1); mVertexData.push_back(sGlyph.UV.V1);

         mVertexData.push_back(fPosX + glyphOffsetX);
         mVertexData.push_back(fPosY + glyphOffsetY);
         mVertexData.push_back(0.0f);
         mVertexData.push_back(sPen.mColor.Red);
         mVertexData.push_back(sPen.mColor.Green);
         mVertexData.push_back(sPen.mColor.Blue);
         mVertexData.push_back(sPen.mColor.Alpha);
         mVertexData.push_back(sGlyph.UV.U0); mVertexData.push_back(sGlyph.UV.V0);

         mVertexData.push_back(fPosX + glyphOffsetX + glyphWidth);
         mVertexData.push_back(fPosY + glyphOffsetY);
         mVertexData.push_back(0.0f);
         mVertexData.push_back(sPen.mColor.Red);
         mVertexData.push_back(sPen.mColor.Green);
         mVertexData.push_back(sPen.mColor.Blue);
         mVertexData.push_back(sPen.mColor.Alpha);
         mVertexData.push_back(sGlyph.UV.U1); mVertexData.push_back(sGlyph.UV.V0);

         mIndices.push_back(sGeometryData.NumVertices);
         mIndices.push_back(sGeometryData.NumVertices + 1u);
         mIndices.push_back(sGeometryData.NumVertices + 2u);
         mIndices.push_back(sGeometryData.NumVertices + 3u);
         mIndices.push_back(sGeometryData.NumVertices + 2u);
         mIndices.push_back(sGeometryData.NumVertices + 1u);

         sGeometryData.NumVertices += 4u;
      }

      iCurrentCharIndex++;

      if(iCurrentCharIndex == mCharacterCountLimit)
      {
         break;
      }

      if(bJustifyText && mLineWidth > GE_EPSILON && mLineJustifySpaces[iCurrentLineIndex] > 0u)
      {
         fAdvanceX += (mLineWidth - mLineWidths[iCurrentLineIndex]) / mLineJustifySpaces[iCurrentLineIndex];
      }

      fPosX += fAdvanceX;
   }

   sGeometryData.NumIndices = (uint32_t)mIndices.size();

   if(sGeometryData.NumIndices > 0u)
   {
      sGeometryData.VertexData = &mVertexData[0];
      sGeometryData.Indices = &mIndices[0];
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
   GEAssert(pPen.mCharIndex < mText.length());

   const uint16_t glyphIndex = getGlyphIndex(pPen.mCharIndex);
   const uint32_t charSetIndex = mFont->getCharacterSetIndex(pPen.mFontStyle);
   const Glyph& glyph = mFont->getGlyph(charSetIndex, glyphIndex);
   const float characterSize = pPen.mFontSize * kFontSizeScale;

   return (glyph.AdvanceX * characterSize) + mHorizontalSpacing;
}

float ComponentLabel::getKerning(const Pen& pPen)
{
   GEAssert(pPen.mCharIndex < mText.length());

   unsigned char cChar = mText[pPen.mCharIndex];
   float kerning = 0.0f;

   if(cChar != ' ' && pPen.mCharIndex > 0u)
   {
      const uint16_t previousGlyphIndex = getGlyphIndex(pPen.mCharIndex - 1u);
      const uint16_t currentGlyphIndex = getGlyphIndex(pPen.mCharIndex);
      const uint32_t charSetIndex = mFont->getCharacterSetIndex(pPen.mFontStyle);
      kerning = mFont->getKerning(charSetIndex, previousGlyphIndex, currentGlyphIndex);

      const float characterSize = pPen.mFontSize * kFontSizeScale;
      kerning *= characterSize;
   }

   return kerning;
}

Font* ComponentLabel::getFont() const
{
   return mFont;
}

const ObjectName& ComponentLabel::getFontName() const
{
   return mFont ? mFont->getName() : ObjectName::Empty;
}

const ObjectName& ComponentLabel::getFontCharacterSet() const
{
   return mFontStyle;
}

float ComponentLabel::getHorizontalSpacing() const
{
   return mHorizontalSpacing;
}

float ComponentLabel::getVerticalSpacing() const
{
   return mVerticalSpacing;
}

float ComponentLabel::getLineWidth() const
{
   return mLineWidth;
}

float ComponentLabel::getTextWidth() const
{
   return mTextWidth;
}

void ComponentLabel::setFont(Font* TextFont)
{
   mFont = TextFont;
}

void ComponentLabel::setFontName(const Core::ObjectName& pFontName)
{
   mFont = RenderSystem::getInstance()->getFont(pFontName);

#if defined (GE_EDITOR_SUPPORT)
   const ObjectName propertyName = ObjectName("FontCharacterSet");
   Property* cProperty = const_cast<Property*>(getProperty(propertyName));
   cProperty->DataPtr = mFont ? (void*)mFont->getCharacterSetRegistry() : nullptr;

   EventArgs sArgs;
   sArgs.Data = cOwner;
   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
#endif

   if(!mFont)
   {
      Log::log(LogType::Warning, "No font found in '%s'. The entity will not be rendered.", cOwner->getFullName().getString());
      hide();
      return;
   }

   if(!mText.empty())
   {
      generateText();
   }
}

void ComponentLabel::setFontCharacterSet(const ObjectName& pCharSetName)
{
   if(pCharSetName == getFontCharacterSet())
   {
      return;
   }

   mFontStyle = pCharSetName;

   if(mFont)
   {
      generateText();
   }
}

void ComponentLabel::setHorizontalSpacing(float pHorizontalSpacing)
{
   mHorizontalSpacing = pHorizontalSpacing;

   if(!mText.empty())
   {
      generateText();
   }
}

void ComponentLabel::setVerticalSpacing(float pVerticalSpacing)
{
   mVerticalSpacing = pVerticalSpacing;

   if(!mText.empty())
   {
      generateText();
   }
}

void ComponentLabel::setLineWidth(float pLineWidth)
{
   mLineWidth = pLineWidth;

   if(!mText.empty())
   {
      generateText();
   }
}
