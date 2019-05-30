
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
   

   GESerializableEnum(SpriteLayer)
   {
      GUI,
      Pre3D,
      PostGUI,

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
      static const ushort QuadIndices[6];

      GeometryType eGeometryType;
      RenderingMode eRenderingMode;
      uint8_t iRenderPriority;
      ComponentTransform* cTransform;
      Color cColor;
    
      Content::GeometryData sGeometryData;

      uint8_t iInternalFlags;

      ComponentRenderable(Entity* Owner);
      ~ComponentRenderable();

   public:
      static const Core::ObjectName ClassName;

      static ComponentType getType() { return ComponentType::Renderable; }

      void show();
      void hide();
    
      GeometryType getGeometryType() const;
      RenderingMode getRenderingMode() const;
      uint8_t getRenderPriority() const;
      ComponentTransform* getTransform() const;

      const Color& getColor() const;
      bool getVisible() const;
      uint8_t getInternalFlags() const;

      const Content::GeometryData& getGeometryData() const;

      void setGeometryType(GeometryType Type);
      void setRenderingMode(RenderingMode Mode);
      void setRenderPriority(uint8_t Priority);
      void setColor(const Color& C);
      void setVisible(bool Visible);
      void setInternalFlags(uint8_t Flags);

      GEPropertyArray(MaterialPass, MaterialPass)
   };
}}
