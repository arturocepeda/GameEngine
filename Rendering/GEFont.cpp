
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEFont.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEFont.h"
#include "Core/GEDevice.h"
#include "Core/GEParser.h"
#include "Core/GEValue.h"
#include "pugixml/pugixml.hpp"

using namespace GE;
using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Content;

const ObjectName Font::TypeName = ObjectName("Font");

Font::Font(const ObjectName& Name, const ObjectName& GroupName, void* RenderDevice)
   : Resource(Name, GroupName, TypeName)
   , pRenderDevice(RenderDevice)
   , cTexture(0)
{
   char sSubDir[256];
   sprintf(sSubDir, "Fonts/%s", GroupName.getString());

   mGlyphs.resize(1);
   mKernings.resize(1);

   ContentData cFontData;
   Device::readContentFile(ContentType::FontData, sSubDir, Name.getString(), "fnt", &cFontData);
   pugi::xml_document xml;
   xml.load_buffer(cFontData.getData(), cFontData.getDataSize());
   loadFontData(0, xml.child("font"));

   ImageData cImageData;
   Device::readContentFile(ContentType::FontTexture, sSubDir, Name.getString(), "png", &cImageData);
   createFontTexture(cImageData);
   cImageData.unload();
}

Font::Font(const ObjectName& Name, const ObjectName& GroupName, std::istream& Stream, void* RenderDevice)
   : Resource(Name, GroupName, TypeName)
   , pRenderDevice(RenderDevice)
   , cTexture(0)
   , fOffsetYMin(0.0f)
   , fOffsetYMax(0.0f)
{
   mGlyphs.resize(1);
   mKernings.resize(1);

   loadFontData(0, Stream);

   ImageData cImageData;
   uint iTextureDataSize = Value::fromStream(ValueType::UInt, Stream).getAsUInt();
   cImageData.load(iTextureDataSize, Stream);
   createFontTexture(cImageData);
   cImageData.unload();
}

Font::~Font()
{
   releaseFontTexture();
}

void Font::loadFontData(uint32_t pCharSet, const pugi::xml_node& pXmlFontData)
{
   const pugi::xml_node& xmlCommon = pXmlFontData.child("common");

   float fTextureWidth = Parser::parseFloat(xmlCommon.attribute("scaleW").value());
   float fTextureHeight = Parser::parseFloat(xmlCommon.attribute("scaleH").value());

   fOffsetYMin = 0.0f;
   fOffsetYMax = 0.0f;

   const pugi::xml_node& xmlChars = pXmlFontData.child("chars");

   for(const pugi::xml_node& xmlChar : xmlChars.children("char"))
   {
      byte iCharId = static_cast<byte>(Parser::parseUInt(xmlChar.attribute("id").value()));

      Glyph sGlyph;
      sGlyph.Width = Parser::parseFloat(xmlChar.attribute("width").value());
      sGlyph.Height = Parser::parseFloat(xmlChar.attribute("height").value());
      sGlyph.OffsetX = Parser::parseFloat(xmlChar.attribute("xoffset").value());
      sGlyph.OffsetY = Parser::parseFloat(xmlChar.attribute("yoffset").value());
      sGlyph.AdvanceX = Parser::parseFloat(xmlChar.attribute("xadvance").value());

      if(sGlyph.OffsetY < fOffsetYMin)
         fOffsetYMin = sGlyph.OffsetY;
      else if(sGlyph.OffsetY > fOffsetYMax)
         fOffsetYMax = sGlyph.OffsetY;

      sGlyph.UV.U0 = Parser::parseFloat(xmlChar.attribute("x").value()) / fTextureWidth;
      sGlyph.UV.U1 = sGlyph.UV.U0 + (Parser::parseFloat(xmlChar.attribute("width").value()) / fTextureWidth);
      sGlyph.UV.V0 = Parser::parseFloat(xmlChar.attribute("y").value()) / fTextureHeight;
      sGlyph.UV.V1 = sGlyph.UV.V0 + (Parser::parseFloat(xmlChar.attribute("height").value()) / fTextureHeight);

      mGlyphs[pCharSet][iCharId] = sGlyph;
   }

   const pugi::xml_node& xmlKernings = pXmlFontData.child("kernings");

   for(const pugi::xml_node& xmlKerning : xmlKernings.children("kerning"))
   {
       byte iKerningFirstCharId = (byte)Parser::parseUInt(xmlKerning.attribute("first").value());
       byte iKerningSecondCharId = (byte)Parser::parseUInt(xmlKerning.attribute("second").value());
       int iKerningAmount = Parser::parseInt(xmlKerning.attribute("amount").value());

       KerningsMap::iterator it = mKernings[pCharSet].find(iKerningFirstCharId);

       if(it == mKernings[pCharSet].end())
       {
           mKernings[pCharSet][iKerningFirstCharId] = CharKerningsMap();
           it = mKernings[pCharSet].find(iKerningFirstCharId);
       }

       CharKerningsMap& mCharKernings = it->second;
       mCharKernings[iKerningSecondCharId] = iKerningAmount;
   }
}

void Font::loadFontData(uint32_t pCharSet, std::istream& pStream)
{
   float fTextureWidth = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();
   float fTextureHeight = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();

   fOffsetYMin = 0.0f;
   fOffsetYMax = 0.0f;

   uint iCharsCount = (uint)Value::fromStream(ValueType::Short, pStream).getAsShort();

   for(uint i = 0; i < iCharsCount; i++)
   {
      byte iCharId = Value::fromStream(ValueType::Byte, pStream).getAsByte();

      float x = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();
      float y = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();

      Glyph sGlyph;
      sGlyph.Width = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();
      sGlyph.Height = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();
      sGlyph.OffsetX = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();
      sGlyph.OffsetY = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();
      sGlyph.AdvanceX = (float)Value::fromStream(ValueType::Short, pStream).getAsShort();

      if(sGlyph.OffsetY < fOffsetYMin)
         fOffsetYMin = sGlyph.OffsetY;
      else if(sGlyph.OffsetY > fOffsetYMax)
         fOffsetYMax = sGlyph.OffsetY;

      sGlyph.UV.U0 = x / fTextureWidth;
      sGlyph.UV.U1 = sGlyph.UV.U0 + (sGlyph.Width / fTextureWidth);
      sGlyph.UV.V0 = y / fTextureHeight;
      sGlyph.UV.V1 = sGlyph.UV.V0 + (sGlyph.Height / fTextureHeight);

      mGlyphs[pCharSet][iCharId] = sGlyph;
   }

   uint iKerningsCount = (uint)Value::fromStream(ValueType::Short, pStream).getAsShort();

   for(uint i = 0; i < iKerningsCount; i++)
   {
      byte iKerningFirstCharId = Value::fromStream(ValueType::Byte, pStream).getAsByte();
      byte iKerningSecondCharId = Value::fromStream(ValueType::Byte, pStream).getAsByte();
      int iKerningAmount = (int)Value::fromStream(ValueType::Short, pStream).getAsShort();

      KerningsMap::iterator it = mKernings[pCharSet].find(iKerningFirstCharId);

      if(it == mKernings[pCharSet].end())
      {
         mKernings[pCharSet][iKerningFirstCharId] = CharKerningsMap();
         it = mKernings[pCharSet].find(iKerningFirstCharId);
      }

      CharKerningsMap& mCharKernings = it->second;
      mCharKernings[iKerningSecondCharId] = iKerningAmount;
   }
}

const Texture* Font::getTexture()
{
   return cTexture;
}

const void* Font::getHandler()
{
   return pRenderDevice;
}

const Glyph& Font::getGlyph(uint32_t pCharSet, GE::byte pCharacter)
{
   pCharSet = pCharSet < (uint32_t)mGlyphs.size() ? pCharSet : 0;
   return mGlyphs[pCharSet][pCharacter];
}

float Font::getKerning(uint32_t pCharSet, GE::byte pChar1, GE::byte pChar2) const
{
   pCharSet = pCharSet < (uint32_t)mKernings.size() ? pCharSet : 0;

   KerningsMap::const_iterator itChar1Kernings = mKernings[pCharSet].find(pChar1);

   if(itChar1Kernings != mKernings[pCharSet].end())
   {
      const CharKerningsMap& sChar1Kernings = itChar1Kernings->second;
      CharKerningsMap::const_iterator itChar2Kerning = sChar1Kernings.find(pChar2);

      if(itChar2Kerning != sChar1Kernings.end())
         return (float)itChar2Kerning->second;
   }

   return 0.0f;
}

float Font::getOffsetYMin() const
{
   return fOffsetYMin;
}

float Font::getOffsetYMax() const
{
   return fOffsetYMax;
}
