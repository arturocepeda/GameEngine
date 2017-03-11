
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEParser.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"

namespace GE { namespace Core
{
   class Parser
   {
   public:
      static int parseInt(const char* StrValue);
      static uint parseUInt(const char* StrValue);
      static float parseFloat(const char* StrValue);
      static bool parseBool(const char* StrValue);
      static Vector2 parseVector2(const char* StrValue);
      static Vector3 parseVector3(const char* StrValue);
      static Color parseColor(const char* StrValue);

      static void writeInt(int Value, char* OutStr);
      static void writeUInt(uint Value, char* OutStr);
      static void writeFloat(float Value, char* OutStr);
      static void writeBool(bool Value, char* OutStr);
      static void writeVector2(const Vector2& Value, char* OutStr);
      static void writeVector3(const Vector3& Value, char* OutStr);
      static void writeColor(const Color& Value, char* OutStr);
   };
}}
