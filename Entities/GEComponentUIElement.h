
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
      float fAlpha;

      ComponentUIElement(Entity* Owner);

   public:
      static const Core::ObjectName ClassName;

      static ComponentType getType() { return ComponentType::UIElement; }

      virtual ~ComponentUIElement();

      float getAlpha() const;
      float getAlphaInHierarchy() const;

      void setAlpha(float Alpha);
   };


   class ComponentUI2DElement : public ComponentUIElement
   {
   private:
      Alignment eAnchor;
      Vector2 vOffset;
      float fScaledYOffset;

      void updateTransformPosition();

   public:
      static const Core::ObjectName ClassName;

      ComponentUI2DElement(Entity* Owner);
      virtual ~ComponentUI2DElement();

      Alignment getAnchor() const;
      const Vector2& getOffset() const;
      float getScaledYOffset() const;

      void setAnchor(Alignment Anchor);
      void setOffset(const Vector2& Offset);
      void setScaledYOffset(float Offset);
   };


   class ComponentUI3DElement : public ComponentUIElement
   {
   private:
      uint8_t iCanvasIndex;

   public:
      static const uint32_t CanvasCount = 64u;
      static const Core::ObjectName ClassName;

      ComponentUI3DElement(Entity* Owner);
      virtual ~ComponentUI3DElement();

      uint8_t getCanvasIndex() const;
      void setCanvasIndex(uint8_t Index);
   };


   GESerializableEnum(CanvasSettingsBitMask)
   {
      RenderAfterTransparentGeometry  = 1 << 0,

      Count = 1
   };


   class ComponentUI3DCanvas : public ComponentUI3DElement
   {
   private:
      uint8_t eSettings;

   public:
      static const Core::ObjectName ClassName;

      ComponentUI3DCanvas(Entity* Owner);
      virtual ~ComponentUI3DCanvas();

      uint8_t getSettings() const { return eSettings; }
      void setSettings(uint8_t Value) { eSettings = Value; }
   };
}}
