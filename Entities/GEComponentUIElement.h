
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentUIElement.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentTransform.h"

namespace GE { namespace Entities
{
   class ComponentUIElement : public Component
   {
   private:
      ComponentTransform* cTransform;

      Alignment eAnchor;
      Vector2 vOffset;
      float fAlpha;

      void updateTransformPosition();

   public:
      static ComponentType getType() { return ComponentType::UIElement; }

      ComponentUIElement(Entity* Owner);
      virtual ~ComponentUIElement();

      Alignment getAnchor() const;
      const Vector2& getOffset() const;
      float getAlpha() const;
      float getAlphaInHierarchy() const;

      void setAnchor(Alignment Anchor);
      void setOffset(const Vector2& Offset);
      void setAlpha(float Alpha);

      GEPropertyEnum(Alignment, Anchor)
      GEProperty(Vector2, Offset)
      GEProperty(Float, Alpha)
   };
}}
