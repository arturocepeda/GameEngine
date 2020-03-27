
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
#include "GERenderSystem.h"
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

   if(getFontCharacterSetCount() == 0u)
   {
      addFontCharacterSet();
   }

   char sSubDir[256];
   sprintf(sSubDir, "Fonts/%s", cGroupName.getString());

   const uint32_t charSetsCount = getFontCharacterSetCount();

   mGlyphs.resize(charSetsCount);
   mKernings.resize(charSetsCount);

   for(uint32_t i = 0u; i < charSetsCount; i++)
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

#if defined (GE_PLATFORM_DESKTOP)
   const char* textureFormat = "dds";
#else
   const char* textureFormat = "png";
#endif

   ImageData* cImageData = Allocator::alloc<ImageData>();
   GEInvokeCtor(ImageData, cImageData)();
   Device::readContentFile(ContentType::FontTexture, sSubDir, cName.getString(), textureFormat, cImageData);
   createFontTexture(cImageData);
}

void Font::load(std::istream& pStream, void* pRenderDevice)
{
   this->pRenderDevice = pRenderDevice;

   if(getFontCharacterSetCount() == 0u)
   {
      addFontCharacterSet();
   }

   const uint32_t charSetsCount = getFontCharacterSetCount();

   mGlyphs.resize(charSetsCount);
   mKernings.resize(charSetsCount);

   for(uint32_t i = 0u; i < charSetsCount; i++)
   {
      FontCharacterSet* charSet = getFontCharacterSet(i);
      mCharSets.add(charSet);

      loadFontData(i, pStream);
   }

   ImageData* cImageData = Allocator::alloc<ImageData>();
   GEInvokeCtor(ImageData, cImageData)();
   uint iTextureDataSize = Value::fromStream(ValueType::UInt, pStream).getAsUInt();
   cImageData->load(iTextureDataSize, pStream);
   createFontTexture(cImageData);
}

void Font::createFontTexture(ImageData* cImageData)
{
   cTexture = Allocator::alloc<Texture>();
   GEInvokeCtor(Texture, cTexture)(cName, "FontTextures");
   cTexture->setWidth(cImageData->getWidth());
   cTexture->setHeight(cImageData->getHeight());

   PreloadedTexture preloadedTexture;
   preloadedTexture.Data = cImageData;
   preloadedTexture.Tex = cTexture;

   RenderSystem::getInstance()->loadTexture(&preloadedTexture);
}

void Font::releaseFontTexture()
{
   RenderSystem::getInstance()->unloadTexture(cTexture);

   GEInvokeDtor(Texture, cTexture);
   cTexture = nullptr;
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
      uint16_t iCharId = static_cast<uint16_t>(Parser::parseUInt(xmlChar.attribute("id").value()));

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
       uint16_t iKerningFirstCharId = (uint16_t)Parser::parseUInt(xmlKerning.attribute("first").value());
       uint16_t iKerningSecondCharId = (uint16_t)Parser::parseUInt(xmlKerning.attribute("second").value());
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
   FontCharacterSet* charSet = getFontCharacterSet(pCharSetIndex);

   const float textureWidth = (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
   const float textureHeight = (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();

   const float base = Value::fromStream(ValueType::Float, pStream).getAsFloat();
   const float lineHeight = Value::fromStream(ValueType::Float, pStream).getAsFloat();
   charSet->setLineHeight(lineHeight);

   fOffsetYMin = 0.0f;
   fOffsetYMax = 0.0f;

   uint iCharsCount = (uint)Value::fromStream(ValueType::UShort, pStream).getAsUShort();

   for(uint i = 0; i < iCharsCount; i++)
   {
      const uint16_t iCharId = Value::fromStream(ValueType::UShort, pStream).getAsUShort();

      const float x = (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      const float y = (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();

      Glyph sGlyph;
      sGlyph.Width = (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      sGlyph.Height = (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      sGlyph.OffsetX = (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      sGlyph.OffsetY = base - (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      sGlyph.AdvanceX = (float)Value::fromStream(ValueType::UShort, pStream).getAsUShort();

      sGlyph.UV.U0 = x / textureWidth;
      sGlyph.UV.U1 = sGlyph.UV.U0 + (sGlyph.Width / textureWidth);
      sGlyph.UV.V0 = y / textureHeight;
      sGlyph.UV.V1 = sGlyph.UV.V0 + (sGlyph.Height / textureHeight);

      sGlyph.UV.U0 = charSet->getUOffset() + (sGlyph.UV.U0 * charSet->getUScale());
      sGlyph.UV.U1 = charSet->getUOffset() + (sGlyph.UV.U1 * charSet->getUScale());
      sGlyph.UV.V0 = charSet->getVOffset() + (sGlyph.UV.V0 * charSet->getVScale());
      sGlyph.UV.V1 = charSet->getVOffset() + (sGlyph.UV.V1 * charSet->getVScale());

      mGlyphs[pCharSetIndex][iCharId] = sGlyph;
   }

   uint iKerningsCount = (uint)Value::fromStream(ValueType::UShort, pStream).getAsUShort();

   for(uint i = 0; i < iKerningsCount; i++)
   {
      const uint16_t iKerningFirstCharId = Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      const uint16_t iKerningSecondCharId = Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      const int iKerningAmount = (int)Value::fromStream(ValueType::UShort, pStream).getAsUShort();

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

const Glyph& Font::getGlyph(uint32_t pCharSetIndex, uint16_t pCharacter)
{
   GEAssert(pCharSetIndex < (uint32_t)mGlyphs.size());
   return mGlyphs[pCharSetIndex][pCharacter];
}

float Font::getKerning(uint32_t pCharSetIndex, uint16_t pChar1, uint16_t pChar2) const
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
