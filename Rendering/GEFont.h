
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


   class Font : public Content::Resource
   {
   private:
      Texture* cTexture;
      void* pRenderDevice;

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

      Font(const Core::ObjectName& Name, const Core::ObjectName& GroupName, void* RenderDevice = 0);
      Font(const Core::ObjectName& Name, const Core::ObjectName& GroupName, std::istream& Stream, void* RenderDevice = 0);
      ~Font();

      const Texture* getTexture();
      const void* getHandler();
      const Glyph& getGlyph(uint32_t pCharSet, byte pCharacter);
      float getKerning(uint32_t pCharSet, byte pChar1, byte pChar2) const;

      float getOffsetYMin() const;
      float getOffsetYMax() const;
   };
}}
