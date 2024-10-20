
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Core
//
//  --- GEUtils.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEConstants.h"
#include "Types/GETypes.h"
#include "Types/GESTLTypes.h"

#include "Externals/pugixml/pugixml.hpp"

namespace GE { namespace Core
{
   class Serializable;


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
      NonCopyable& operator=(const NonCopyable&);
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
   //  SerializableIO
   //
   class SerializableIO
   {
   public:
      static bool saveToXmlFile(Serializable* Obj, const char* Directory, const char* FileName);
      static bool saveToBinaryFile(Serializable* Obj, const char* Directory, const char* FileName);

      static bool loadFromXmlFile(Serializable* Obj, const char* Directory, const char* FileName);
      static bool loadFromBinaryFile(Serializable* Obj, const char* Directory, const char* FileName);
   };


   //
   //  FNV-1a Hash
   //
   uint32_t hash(const char* pString);
   bool isHash(const char* pString);
   void toHashPath(char* pPath);


   //
   //  UTF-8
   //
   void utf8ToUnicode(const char* pSequence, int* pOutUnicode, int* pOutExtraChars);
   void utf8AppendText(const char* pText, GESTLString* pString, GESTLString* pStringExtension);
}}
