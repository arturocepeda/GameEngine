
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEFont.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Rendering/GETexture.h"
#include "Content/GEImageData.h"
#include "Content/GEResource.h"

#include <map>

namespace GE { namespace Rendering
{
   struct Glyph
   {
      float Width;
      float Height;
      float OffsetX;
      float OffsetY;
      float AdvanceX;

      TextureCoordinates UV;
   };


   class FontCharacterSet : public Core::SerializableArrayElement, public Core::Object
   {
   private:
      float mUOffset;
      float mVOffset;
      float mUScale;
      float mVScale;

   public:
      FontCharacterSet();
      ~FontCharacterSet();

      void setName(const Core::ObjectName& pName) { cName = pName; }

      GEDefaultGetter(float, UOffset)
      GEDefaultGetter(float, VOffset)
      GEDefaultGetter(float, UScale)
      GEDefaultGetter(float, VScale)

      GEDefaultSetter(float, UOffset)
      GEDefaultSetter(float, VOffset)
      GEDefaultSetter(float, UScale)
      GEDefaultSetter(float, VScale)
   };


   class Font : public Content::Resource
   {
   private:
      Texture* cTexture;
      void* pRenderDevice;

      Core::ObjectManager<FontCharacterSet> mCharSets;

      typedef GESTLMap(byte, Glyph) GlyphsMap;
      GESTLVector(GlyphsMap) mGlyphs;

      typedef GESTLMap(byte, int) CharKerningsMap;
      typedef GESTLMap(byte, CharKerningsMap) KerningsMap;
      GESTLVector(KerningsMap) mKernings;

      float fOffsetYMin;
      float fOffsetYMax;

      void loadFontData(uint32_t pCharSet, const pugi::xml_node& pXmlFontData);
      void loadFontData(uint32_t pCharSet, std::istream& pStream);

      void createFontTexture(Content::ImageData& cImageData);
      void releaseFontTexture();

   public:
      static const Core::ObjectName TypeName;

      Font(const Core::ObjectName& pName, const Core::ObjectName& pGroupName);
      ~Font();

      void load(void* pRenderDevice);
      void load(std::istream& pStream, void* pRenderDevice);

      const Texture* getTexture();
      const void* getHandler();

      const Core::ObjectRegistry* getCharacterSetRegistry() const;
      uint32_t getCharacterSetIndex(const Core::ObjectName& pCharacterSetName) const;

      const Glyph& getGlyph(uint32_t pCharSetIndex, byte pCharacter);
      float getKerning(uint32_t pCharSetIndex, byte pChar1, byte pChar2) const;

      float getOffsetYMin() const;
      float getOffsetYMax() const;

      GEPropertyArray(FontCharacterSet, FontCharacterSet)
   };
}}
