
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
#include "Core/GEDevice.h"

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
      float mLineHeight;
      float mUOffset;
      float mVOffset;
      float mUScale;
      float mVScale;

   public:
      FontCharacterSet();
      ~FontCharacterSet();

      void setName(const Core::ObjectName& pName) { cName = pName; }

      GEDefaultGetter(float, LineHeight, m)
      GEDefaultGetter(float, UOffset, m)
      GEDefaultGetter(float, VOffset, m)
      GEDefaultGetter(float, UScale, m)
      GEDefaultGetter(float, VScale, m)

      GEDefaultSetter(float, LineHeight, m)
      GEDefaultSetter(float, UOffset, m)
      GEDefaultSetter(float, VOffset, m)
      GEDefaultSetter(float, UScale, m)
      GEDefaultSetter(float, VScale, m)
   };


   class FontReplacement : public Core::SerializableArrayElement
   {
   private:
      Core::SystemLanguage mLanguage;
      Core::ObjectName mFontName;
      float mSizeFactor;
      float mVerticalOffset;

   public:
      FontReplacement();
      ~FontReplacement();

      GEDefaultGetter(Core::SystemLanguage, Language, m)
      GEDefaultGetter(const Core::ObjectName&, FontName, m)
      GEDefaultGetter(float, SizeFactor, m)
      GEDefaultGetter(float, VerticalOffset, m)

      GEDefaultSetter(Core::SystemLanguage, Language, m)
      GEDefaultSetter(const Core::ObjectName&, FontName, m)
      GEDefaultSetter(float, SizeFactor, m)
      GEDefaultSetter(float, VerticalOffset, m)
   };


   class Font : public Content::Resource
   {
   private:
      Texture* cTexture;
      void* pRenderDevice;

      Core::ObjectManager<FontCharacterSet> mCharSets;

      typedef GESTLMap(uint16_t, Glyph) GlyphsMap;
      GESTLVector(GlyphsMap) mGlyphs;

      typedef GESTLMap(uint16_t, int) CharKerningsMap;
      typedef GESTLMap(uint16_t, CharKerningsMap) KerningsMap;
      GESTLVector(KerningsMap) mKernings;

      void loadFontData(uint32_t pCharSet, const pugi::xml_node& pXmlFontData);
      void loadFontData(uint32_t pCharSet, std::istream& pStream);

      void createFontTexture(Content::ImageData* cImageData);
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

      float getLineHeight(uint32_t pCharSetIndex);
      const Glyph& getGlyph(uint32_t pCharSetIndex, uint16_t pCharacter);
      float getKerning(uint32_t pCharSetIndex, uint16_t pChar1, uint16_t pChar2) const;

      GEPropertyArray(FontCharacterSet, FontCharacterSet)
      GEPropertyArray(FontReplacement, FontReplacement)
   };
}}
