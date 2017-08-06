
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentRenderable.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentTransform.h"
#include "Rendering/GERenderingObjects.h"
#include "Rendering/GEMaterial.h"
#include "Content/GEGeometryData.h"

namespace GE { namespace Entities
{
   using namespace Rendering;


   enum class RenderableType
   {
      Mesh,
      Sprite,
      Label,
      ParticleSystem,

      Count
   };


   class ComponentRenderable : public Component
   {
   protected:
      static ushort QuadIndices[6];

      RenderableType iRenderableType;
      GeometryType eGeometryType;
      RenderingMode eRenderingMode;
      ComponentTransform* cTransform;
      Color cColor;
    
      Content::GeometryData sGeometryData;

      bool bVisible;

      ComponentRenderable(Entity* Owner, RenderableType RType);
      ~ComponentRenderable();

   public:
      static ComponentType getType() { return ComponentType::Renderable; }

      void show();
      void hide();
    
      RenderableType getRenderableType() const;
      GeometryType getGeometryType() const;
      RenderingMode getRenderingMode() const;
      ComponentTransform* getTransform() const;

      const Color& getColor() const;
      bool getVisible() const;

      const Content::GeometryData& getGeometryData() const;

      void setGeometryType(GeometryType Type);
      void setRenderingMode(RenderingMode Mode);
      void setColor(const Color& C);
      void setVisible(bool Visible);

      GEProperty(Bool, Visible)
      GEPropertyEnum(GeometryType, GeometryType)
      GEPropertyEnum(RenderingMode, RenderingMode)
      GEProperty(Color, Color)
      GEPropertyArray(MaterialPass, MaterialPass)
   };
}}
