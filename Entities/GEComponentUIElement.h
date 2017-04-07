
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
   protected:
      ComponentTransform* cTransform;
      float fAlpha;

      ComponentUIElement(Entity* Owner);

   public:
      static ComponentType getType() { return ComponentType::UIElement; }

      virtual ~ComponentUIElement();

      float getAlpha() const;
      float getAlphaInHierarchy() const;

      void setAlpha(float Alpha);

      GEProperty(Float, Alpha)
   };


   class ComponentUI2DElement : public ComponentUIElement
   {
   private:
      Alignment eAnchor;
      Vector2 vOffset;

      void updateTransformPosition();

   public:
      ComponentUI2DElement(Entity* Owner);
      virtual ~ComponentUI2DElement();

      Alignment getAnchor() const;
      const Vector2& getOffset() const;

      void setAnchor(Alignment Anchor);
      void setOffset(const Vector2& Offset);

      GEPropertyEnum(Alignment, Anchor)
      GEProperty(Vector2, Offset)
   };


   class ComponentUI3DElement : public ComponentUIElement
   {
   public:
      ComponentUI3DElement(Entity* Owner);
      virtual ~ComponentUI3DElement();
   };
}}
