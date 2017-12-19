
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
#include "Core/GEDevice.h"
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
   , fFontSize(12.0f)
   , iAlignment(Alignment::MiddleCenter)
   , fHorizontalSpacing(0.0f)
   , fVerticalSpacing(0.0f)
   , fLineWidth(0.0f)
   , fCharacterSize(0.0f)
{
   cClassName = ObjectName("Label");

   sGeometryData.VertexStride = 20;

   GERegisterPropertyResource(ObjectName, FontName, Font);
   GERegisterProperty(Float, FontSize);
   GERegisterPropertyEnum(Alignment, Alignment);
   GERegisterProperty(Float, HorizontalSpacing);
   GERegisterProperty(Float, VerticalSpacing);
   GERegisterProperty(Float, LineWidth);
   GERegisterProperty(String, Text);
   GERegisterPropertyResource(ObjectName, StringID, LocalizedString);
}

ComponentLabel::~ComponentLabel()
{
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

   const uint iTextLength = (uint)sText.length();

   fCharacterSize = (fFontSize * FontSizeScale);

   vLineWidths.clear();
   vLineFeedIndices.clear();

   float fCurrentLineWidth = 0.0f;

   int iLastSpaceIndex = -1;
   float fLineWidthAtLastSpace = 0.0f;
   float fLastSpaceCharWidth = 0.0f;

   for(uint i = 0; i < iTextLength; i++)
   {
      unsigned char cChar = sText[i];

      if(cChar == LineFeedChar)
      {
         vLineWidths.push_back(fCurrentLineWidth);
         vLineFeedIndices.push_back(i);

         fCurrentLineWidth = 0.0f;
         iLastSpaceIndex = -1;

         continue;
      }

      float fCharWidth = measureCharacter(i) + getKerning(i);

      if(cChar == ' ')
      {
         iLastSpaceIndex = (int)i;
         fLineWidthAtLastSpace = fCurrentLineWidth;
         fLastSpaceCharWidth = fCharWidth;
      }

      fCurrentLineWidth += fCharWidth;

      if(fLineWidth > GE_EPSILON && fCurrentLineWidth > fLineWidth && iLastSpaceIndex >= 0)
      {
         vLineWidths.push_back(fLineWidthAtLastSpace);
         vLineFeedIndices.push_back((uint)iLastSpaceIndex);

         fCurrentLineWidth -= fLineWidthAtLastSpace + fLastSpaceCharWidth;
         iLastSpaceIndex = -1;
      }
   }

   vLineWidths.push_back(fCurrentLineWidth);
   vLineFeedIndices.push_back(iTextLength);

   float fPosX;
   float fPosY;

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
      fPosX = -(vLineWidths[0] * 0.5f);
      break;
   case Alignment::TopRight:
   case Alignment::MiddleRight:
   case Alignment::BottomRight:
      fPosX = -vLineWidths[0];
      break;
   default:
      break;
   }

   const float fFontOffsetY = (cFont->getOffsetYMin() + cFont->getOffsetYMax()) * 0.5f * fCharacterSize;
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

   vVertexData.clear();
   vIndices.clear();
   sGeometryData.NumVertices = 0;

   for(uint i = 0; i < iTextLength; i++)
   {
      if(i == vLineFeedIndices[iCurrentLineIndex])
      {
         iCurrentLineIndex++;

         fPosY -= fLineHeight;

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
            fPosX = -(vLineWidths[iCurrentLineIndex] * 0.5f);
            break;
         case Alignment::TopRight:
         case Alignment::MiddleRight:
         case Alignment::BottomRight:
            fPosX = -vLineWidths[iCurrentLineIndex];
            break;
         default:
            break;
         }

         continue;
      }

      unsigned char cCurrentChar = sText[i];
      const Glyph& sGlyph = cFont->getGlyph(cCurrentChar);

      float fAdvanceX = measureCharacter(i);

      if(cCurrentChar != ' ')
      {
         fPosX += getKerning(i);

         float fGlyphWidth = sGlyph.Width * fCharacterSize;
         float fGlyphHeight = sGlyph.Height * fCharacterSize;

         float fHalfGlyphWidth = fGlyphWidth * 0.5f;
         float fHalfGlyphHeight = fGlyphHeight * 0.5f;
         float fHalfGlyphOffsetX = (sGlyph.OffsetX * fCharacterSize) * 0.5f;
         float fHalfGlyphOffsetY = (sGlyph.OffsetY * fCharacterSize) * 0.5f;
         float fHalfAdvanceX = fAdvanceX * 0.5f;

         vVertexData.push_back(fPosX - fHalfGlyphWidth + fHalfGlyphOffsetX + fHalfAdvanceX);
         vVertexData.push_back(fPosY - fHalfGlyphHeight - fHalfGlyphOffsetY);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sGlyph.UV.U0); vVertexData.push_back(sGlyph.UV.V1);

         vVertexData.push_back(fPosX + fHalfGlyphWidth + fHalfGlyphOffsetX + fHalfAdvanceX);
         vVertexData.push_back(fPosY - fHalfGlyphHeight - fHalfGlyphOffsetY);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sGlyph.UV.U1); vVertexData.push_back(sGlyph.UV.V1);

         vVertexData.push_back(fPosX - fHalfGlyphWidth + fHalfGlyphOffsetX + fHalfAdvanceX);
         vVertexData.push_back(fPosY + fHalfGlyphHeight - fHalfGlyphOffsetY);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sGlyph.UV.U0); vVertexData.push_back(sGlyph.UV.V0);

         vVertexData.push_back(fPosX + fHalfGlyphWidth + fHalfGlyphOffsetX + fHalfAdvanceX);
         vVertexData.push_back(fPosY + fHalfGlyphHeight - fHalfGlyphOffsetY);
         vVertexData.push_back(0.0f);
         vVertexData.push_back(sGlyph.UV.U1); vVertexData.push_back(sGlyph.UV.V0);

         vIndices.push_back(sGeometryData.NumVertices);
         vIndices.push_back(sGeometryData.NumVertices + 1);
         vIndices.push_back(sGeometryData.NumVertices + 2);
         vIndices.push_back(sGeometryData.NumVertices + 3);
         vIndices.push_back(sGeometryData.NumVertices + 2);
         vIndices.push_back(sGeometryData.NumVertices + 1);

         sGeometryData.NumVertices += 4;
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

float ComponentLabel::measureCharacter(uint iCharIndex)
{
   GEAssert(iCharIndex < sText.length());

   unsigned char cChar = sText[iCharIndex];
   const Glyph& sGlyph = cFont->getGlyph(cChar);

   return (sGlyph.AdvanceX * fCharacterSize) + fHorizontalSpacing;
}

float ComponentLabel::getKerning(uint iCharIndex)
{
   GEAssert(iCharIndex < sText.length());

   unsigned char cChar = sText[iCharIndex];
   float fKerning = 0.0f;

   if(cChar != ' ')
   {
      fKerning = iCharIndex > 0 ? cFont->getKerning(sText[iCharIndex - 1], cChar) : 0.0f;
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

void ComponentLabel::setFont(Font* TextFont)
{
   cFont = TextFont;
}

void ComponentLabel::setFontName(const Core::ObjectName& FontName)
{
   cFont = RenderSystem::getInstance()->getFont(FontName);

   if(!cFont)
   {
      Device::log("No font found in '%s'. The entity will not be rendered.", cOwner->getFullName().getString().c_str());
      hide();
      return;
   }

   if(!sText.empty())
      generateVertexData();
}

void ComponentLabel::setFontSize(float FontSize)
{
   fFontSize = FontSize;

   if(!sText.empty())
      generateVertexData();
}

void ComponentLabel::setAlignment(Alignment Align)
{
   iAlignment = Align;

   if(!sText.empty())
      generateVertexData();
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

   processVariables();
   generateVertexData();
}

void ComponentLabel::setStringID(const ObjectName& StringID)
{
   cStringID = StringID;
   LocalizedString* cLocaString = 0;
    
   if(!StringID.isEmpty())
   {
      cLocaString = LocalizedStringsManager::getInstance()->get(StringID);
   }

   if(cLocaString)
   {
      setText(cLocaString->getString().c_str());
   }
}

void ComponentLabel::setHorizontalSpacing(float HorizontalSpacing)
{
   fHorizontalSpacing = HorizontalSpacing;

   if(!sText.empty())
      generateVertexData();
}

void ComponentLabel::setVerticalSpacing(float VerticalSpacing)
{
   fVerticalSpacing = VerticalSpacing;

   if(!sText.empty())
      generateVertexData();
}

void ComponentLabel::setLineWidth(float LineWidth)
{
   fLineWidth = LineWidth;

   if(!sText.empty())
      generateVertexData();
}
