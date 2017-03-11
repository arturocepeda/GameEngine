
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEUtils.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include <stdlib.h>
#include <cmath>
#include <string>

#include "GEConstants.h"
#include "Types/GETypes.h"
#include "Types/GESTLTypes.h"
#include "Externals/pugixml/pugixml.hpp"

namespace GE { namespace Core
{
   //
   //  NonCopyable
   //
   class NonCopyable
   {
   protected:
      NonCopyable() {}
      ~NonCopyable() {}

   private:
      NonCopyable(const NonCopyable&);
      NonCopyable& operator = (const NonCopyable&);
   };


   //
   //  Scaler
   //
   class Scaler
   {
   private:
      float m;
      float b;

   public:
      Scaler(float x0, float x1, float y0, float y1);
      ~Scaler();

      float y(float x);
      float x(float y);
   };


   //
   //  XmlStringWriter
   //
   class XmlStringWriter : public pugi::xml_writer
   {
   private:
      char* sBuffer;

      void releaseBuffer();

   public:
      XmlStringWriter();
      ~XmlStringWriter();

      virtual void write(const void* data, size_t size) override;

      const char* getData();
   };


   //
   //  FNV-1a Hash
   //
   uint hash(const GESTLString& Str);
}}
