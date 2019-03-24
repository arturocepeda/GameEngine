
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


//
//  FontCharacterSet
//
const ObjectName FontCharacterSetClassName = ObjectName("FontCharacterSet");

FontCharacterSet::FontCharacterSet()
   : SerializableArrayElement(FontCharacterSetClassName)
   , Object(0u)
   , mUOffset(0.0f)
   , mVOffset(0.0f)
   , mUScale(1.0f)
   , mVScale(1.0f)
{
   GERegisterProperty(ObjectName, Name);
   GERegisterProperty(Float, UOffset);
   GERegisterProperty(Float, VOffset);
   GERegisterProperty(Float, UScale);
   GERegisterProperty(Float, VScale);
}

FontCharacterSet::~FontCharacterSet()
{
}


//
//  Font
//
const ObjectName Font::TypeName = ObjectName("Font");

Font::Font(const ObjectName& Name, const ObjectName& GroupName)
   : Resource(Name, GroupName, TypeName)
   , pRenderDevice(0)
   , cTexture(0)
   , fOffsetYMin(0.0f)
   , fOffsetYMax(0.0f) 
{
   GERegisterPropertyArray(FontCharacterSet);
}

Font::~Font()
{
   if(cTexture)
   {
      releaseFontTexture();
   }

   GEReleasePropertyArray(FontCharacterSet);
}

void Font::load(void* pRenderDevice)
{
   this->pRenderDevice = pRenderDevice;

   if(getFontCharacterSetCount() == 0)
   {
      addFontCharacterSet();
   }

   char sSubDir[256];
   sprintf(sSubDir, "Fonts/%s", cGroupName.getString());

   const uint32_t charSetsCount = getFontCharacterSetCount();

   mGlyphs.resize(charSetsCount);
   mKernings.resize(charSetsCount);

   for(uint32_t i = 0; i < charSetsCount; i++)
   {
      FontCharacterSet* charSet = getFontCharacterSet(i);
      mCharSets.add(charSet);

      char fileNameBuffer[64];
      const char* fileName = cName.getString();

      if(!charSet->getName().isEmpty())
      {
         sprintf(fileNameBuffer, "%s_%s", cName.getString(), charSet->getName().getString());
         fileName = fileNameBuffer;
      }

      ContentData cFontData;
      Device::readContentFile(ContentType::FontData, sSubDir, fileName, "fnt", &cFontData);
      pugi::xml_document xml;
      xml.load_buffer(cFontData.getData(), cFontData.getDataSize());
      loadFontData(i, xml.child("font"));
   }

   ImageData cImageData;
   Device::readContentFile(ContentType::FontTexture, sSubDir, cName.getString(), "png", &cImageData);
   createFontTexture(cImageData);
   cImageData.unload();
}

void Font::load(std::istream& pStream, void* pRenderDevice)
{
   this->pRenderDevice = pRenderDevice;

   mGlyphs.resize(1);
   mKernings.resize(1);

   loadFontData(0, pStream);

   ImageData cImageData;
   uint iTextureDataSize = Value::fromStream(ValueType::UInt, pStream).getAsUInt();
   cImageData.load(iTextureDataSize, pStream);
   createFontTexture(cImageData);
   cImageData.unload();
}

void Font::loadFontData(uint32_t pCharSetIndex, const pugi::xml_node& pXmlFontData)
{
   FontCharacterSet* charSet = getFontCharacterSet(pCharSetIndex);

   const pugi::xml_node& xmlCommon = pXmlFontData.child("common");

   const float textureWidth = Parser::parseFloat(xmlCommon.attribute("scaleW").value());
   const float textureHeight = Parser::parseFloat(xmlCommon.attribute("scaleH").value());

   const float base = Parser::parseFloat(xmlCommon.attribute("base").value());
   const float lineHeight = Parser::parseFloat(xmlCommon.attribute("lineHeight").value());
   charSet->setLineHeight(lineHeight);

   const pugi::xml_node& xmlChars = pXmlFontData.child("chars");

   for(const pugi::xml_node& xmlChar : xmlChars.children("char"))
   {
      byte iCharId = static_cast<byte>(Parser::parseUInt(xmlChar.attribute("id").value()));

      Glyph sGlyph;
      sGlyph.Width = Parser::parseFloat(xmlChar.attribute("width").value());
      sGlyph.Height = Parser::parseFloat(xmlChar.attribute("height").value());
      sGlyph.OffsetX = Parser::parseFloat(xmlChar.attribute("xoffset").value());
      sGlyph.OffsetY = base - Parser::parseFloat(xmlChar.attribute("yoffset").value());
      sGlyph.AdvanceX = Parser::parseFloat(xmlChar.attribute("xadvance").value());

      sGlyph.UV.U0 = Parser::parseFloat(xmlChar.attribute("x").value()) / textureWidth;
      sGlyph.UV.U1 = sGlyph.UV.U0 + (sGlyph.Width / textureWidth);
      sGlyph.UV.V0 = Parser::parseFloat(xmlChar.attribute("y").value()) / textureHeight;
      sGlyph.UV.V1 = sGlyph.UV.V0 + (sGlyph.Height / textureHeight);

      sGlyph.UV.U0 = charSet->getUOffset() + (sGlyph.UV.U0 * charSet->getUScale());
      sGlyph.UV.U1 = charSet->getUOffset() + (sGlyph.UV.U1 * charSet->getUScale());
      sGlyph.UV.V0 = charSet->getVOffset() + (sGlyph.UV.V0 * charSet->getVScale());
      sGlyph.UV.V1 = charSet->getVOffset() + (sGlyph.UV.V1 * charSet->getVScale());

      mGlyphs[pCharSetIndex][iCharId] = sGlyph;
   }

   const pugi::xml_node& xmlKernings = pXmlFontData.child("kernings");

   for(const pugi::xml_node& xmlKerning : xmlKernings.children("kerning"))
   {
       byte iKerningFirstCharId = (byte)Parser::parseUInt(xmlKerning.attribute("first").value());
       byte iKerningSecondCharId = (byte)Parser::parseUInt(xmlKerning.attribute("second").value());
       int iKerningAmount = Parser::parseInt(xmlKerning.attribute("amount").value());

       KerningsMap::iterator it = mKernings[pCharSetIndex].find(iKerningFirstCharId);

       if(it == mKernings[pCharSetIndex].end())
       {
           mKernings[pCharSetIndex][iKerningFirstCharId] = CharKerningsMap();
           it = mKernings[pCharSetIndex].find(iKerningFirstCharId);
       }

       CharKerningsMap& mCharKernings = it->second;
       mCharKernings[iKerningSecondCharId] = iKerningAmount;
   }
}

void Font::loadFontData(uint32_t pCharSetIndex, std::istream& pStream)
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

      mGlyphs[pCharSetIndex][iCharId] = sGlyph;
   }

   uint iKerningsCount = (uint)Value::fromStream(ValueType::Short, pStream).getAsShort();

   for(uint i = 0; i < iKerningsCount; i++)
   {
      byte iKerningFirstCharId = Value::fromStream(ValueType::Byte, pStream).getAsByte();
      byte iKerningSecondCharId = Value::fromStream(ValueType::Byte, pStream).getAsByte();
      int iKerningAmount = (int)Value::fromStream(ValueType::Short, pStream).getAsShort();

      KerningsMap::iterator it = mKernings[pCharSetIndex].find(iKerningFirstCharId);

      if(it == mKernings[pCharSetIndex].end())
      {
         mKernings[pCharSetIndex][iKerningFirstCharId] = CharKerningsMap();
         it = mKernings[pCharSetIndex].find(iKerningFirstCharId);
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

const ObjectRegistry* Font::getCharacterSetRegistry() const
{
   return mCharSets.getObjectRegistry();
}

uint32_t Font::getCharacterSetIndex(const ObjectName& pCharacterSetName) const
{
   for(uint32_t i = 0; i < getFontCharacterSetCount(); i++)
   {
      const FontCharacterSet* charSet = getFontCharacterSetConst(i);

      if(charSet->getName() == pCharacterSetName)
      {
         return i;
      }
   }

   return 0;
}

float Font::getLineHeight(uint32_t pCharSetIndex)
{
   GEAssert(pCharSetIndex < (uint32_t)mCharSets.count());
   return getFontCharacterSet(pCharSetIndex)->getLineHeight();
}

const Glyph& Font::getGlyph(uint32_t pCharSetIndex, GE::byte pCharacter)
{
   GEAssert(pCharSetIndex < (uint32_t)mGlyphs.size());
   return mGlyphs[pCharSetIndex][pCharacter];
}

float Font::getKerning(uint32_t pCharSetIndex, GE::byte pChar1, GE::byte pChar2) const
{
   GEAssert(pCharSetIndex < (uint32_t)mCharSets.count());

   KerningsMap::const_iterator itChar1Kernings = mKernings[pCharSetIndex].find(pChar1);

   if(itChar1Kernings != mKernings[pCharSetIndex].end())
   {
      const CharKerningsMap& sChar1Kernings = itChar1Kernings->second;
      CharKerningsMap::const_iterator itChar2Kerning = sChar1Kernings.find(pChar2);

      if(itChar2Kerning != sChar1Kernings.end())
      {
         return (float)itChar2Kerning->second;
      }
   }

   return 0.0f;
}
