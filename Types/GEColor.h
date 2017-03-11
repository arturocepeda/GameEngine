
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Types
//
//  --- GEColor.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GETypeDefinitions.h"

namespace GE
{
   struct Color
   {
      float Red;
      float Green;
      float Blue;
      float Alpha;

      Color()
         : Red(1.0f)
         , Green(1.0f)
         , Blue(1.0f)
         , Alpha(1.0f)
      {
      }

      Color(byte cR, byte cG, byte cB, byte cA = 0xFF)
      {
         set(cR, cG, cB, cA);
      }

      Color(float fR, float fG, float fB, float fA = 1.0f)
      {
         set(fR, fG, fB, fA);
      }

      void set(byte cR, byte cG, byte cB, byte cA = 0xFF)
      {
         Red = cR / 255.0f;
         Green = cG / 255.0f;
         Blue = cB / 255.0f;
         Alpha = cA / 255.0f;
      }

      void set(float fR, float fG, float fB, float fA = 1.0f)
      {
         Red = fR;
         Green = fG;
         Blue = fB;
         Alpha = fA;
      }

      float getOpacity() const
      {
         return Alpha;
      }

      void setOpacity(float Opacity)
      {
         Alpha = Opacity;
      }

      Color operator+(const Color& Other)
      {
         return Color(Red + Other.Red, Green + Other.Green, Blue + Other.Blue, Alpha + Other.Alpha);
      }

      Color operator-(const Color& Other)
      {
         return Color(Red - Other.Red, Green - Other.Green, Blue - Other.Blue, Alpha - Other.Alpha);
      }

      Color operator*(const Color& Other) const
      {
         return Color(Red * Other.Red, Green * Other.Green, Blue * Other.Blue, Alpha * Other.Alpha);
      }

      Color operator*(const float fValue) const
      {
         return Color(Red * fValue, Green * fValue, Blue * fValue, Alpha * fValue);
      }

      Color& operator+=(const Color& c)
      {
         Red += c.Red; Green += c.Green; Blue += c.Blue; Alpha += c.Alpha;
         return *this;
      }

      static Color lerp(const Color& C1, const Color& C2, float T)
      {
         return Color(
            Core::Math::lerp(C1.Red, C2.Red, T),
            Core::Math::lerp(C1.Green, C2.Green, T),
            Core::Math::lerp(C1.Blue, C2.Blue, T),
            Core::Math::lerp(C1.Alpha, C2.Alpha, T));
      }
   };
}
