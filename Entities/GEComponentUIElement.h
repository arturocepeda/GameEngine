
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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
      bool mInputEnabled;
      uint8_t mInputTabOrder;

      ComponentUIElement(Entity* Owner);

   public:
      static const Core::ObjectName ClassName;

      static ComponentType getType() { return ComponentType::UIElement; }

      virtual ~ComponentUIElement();

      float getAlpha() const;
      float getAlphaInHierarchy() const;

      void setAlpha(float Alpha);

      GEDefaultGetter(bool, InputEnabled, m);
      GEDefaultSetter(bool, InputEnabled, m);

      GEDefaultGetter(uint8_t, InputTabOrder, m);
      GEDefaultSetter(uint8_t, InputTabOrder, m);
   };


   GESerializableEnum(OffsetMode)
   {
      Absolute,
      Relative,

      Count
   };


   class ComponentUI2DElement : public ComponentUIElement
   {
   private:
      Alignment eAnchor;
      Vector2 vOffset;
      OffsetMode mOffsetMode;

      void updateTransformPosition();

   public:
      static const Core::ObjectName ClassName;

      ComponentUI2DElement(Entity* Owner);
      virtual ~ComponentUI2DElement();

      Alignment getAnchor() const;
      const Vector2& getOffset() const;
      OffsetMode getOffsetMode() const;

      void setAnchor(Alignment Anchor);
      void setOffset(const Vector2& Offset);
      void setOffsetMode(OffsetMode pMode);
   };


   class ComponentUI3DElement : public ComponentUIElement
   {
   private:
      uint8_t iCanvasIndex;

   public:
      static const Core::ObjectName ClassName;

      ComponentUI3DElement(Entity* Owner);
      virtual ~ComponentUI3DElement();

      uint8_t getCanvasIndex() const;
      void setCanvasIndex(uint8_t Index);
   };


   GESerializableEnum(CanvasSettingsBitMask)
   {
      RenderAfter2DElements  = 1 << 0,

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
