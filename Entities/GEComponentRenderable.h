
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
   public:
      enum class InternalFlags
      {
         Visible        =  1 << 0,
         DebugGeometry  =  1 << 1,
      };

   protected:
      static ushort QuadIndices[6];

      RenderableType iRenderableType;
      GeometryType eGeometryType;
      RenderingMode eRenderingMode;
      ComponentTransform* cTransform;
      Color cColor;
    
      Content::GeometryData sGeometryData;

      uint8_t iInternalFlags;

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
      uint8_t getInternalFlags() const;

      const Content::GeometryData& getGeometryData() const;

      void setGeometryType(GeometryType Type);
      void setRenderingMode(RenderingMode Mode);
      void setColor(const Color& C);
      void setVisible(bool Visible);
      void setInternalFlags(uint8_t Flags);

      GEPropertyArray(MaterialPass, MaterialPass)
   };
}}
