
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


   class Font : public Core::Object
   {
   private:
      Texture* cTexture;
      void* pRenderDevice;

      typedef GESTLMap(byte, Glyph) GlyphsMap;
      GlyphsMap mGlyphs;

      typedef GESTLMap(byte, int) CharKerningsMap;
      typedef GESTLMap(byte, CharKerningsMap) KerningsMap;
      KerningsMap mKernings;

      float fOffsetYMin;
      float fOffsetYMax;

      void loadFontData(const pugi::xml_node& xmlFontData);
      void loadFontData(std::istream& sStream);

      void createFontTexture(Content::ImageData& cImageData);
      void releaseFontTexture();

   public:
      Font(const Core::ObjectName& Name, const char* FileName, void* RenderDevice = 0);
      Font(const Core::ObjectName& Name, std::istream& Stream, void* RenderDevice = 0);
      ~Font();

      const Texture* getTexture();
      const void* getHandler();
      const Glyph& getGlyph(byte Character);
      float getKerning(byte Char1, byte Char2) const;

      float getOffsetYMin() const;
      float getOffsetYMax() const;
   };
}}
