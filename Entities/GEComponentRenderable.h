
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
      ParticleSystem
   };


   class ComponentRenderable : public Component
   {
   protected:
      static ushort QuadIndices[6];

      RenderableType iRenderableType;
      GeometryType eGeometryType;
      RenderingMode eRenderingMode;
      ComponentTransform* cTransform;
      GESTLVector(Rendering::MaterialPass*) vMaterialPassList;
      Color cColor;
    
      Content::GeometryData sGeometryData;

      bool bVisible;

      ComponentRenderable(Entity* Owner, RenderableType RType, GeometryType GType);
      ~ComponentRenderable();

   public:
      static ComponentType getType() { return ComponentType::Renderable; }

      void show();
      void hide();
    
      RenderableType getRenderableType() const;
      GeometryType getGeometryType() const;
      RenderingMode getRenderingMode() const;
      ComponentTransform* getTransform() const;

      Rendering::MaterialPass* addMaterialPass();
      uint getMaterialPassCount() const;
      Rendering::MaterialPass* getMaterialPass(uint Index);
      void removeMaterialPass(uint Index);
      void clearMaterialPasses();

      const Color& getColor() const;
      bool getVisible() const;

      const Content::GeometryData& getGeometryData() const;

      void setGeometryType(GeometryType Type);
      void setRenderingMode(RenderingMode Mode);
      void setColor(const Color& C);
      void setVisible(bool Visible);

      virtual void copy(Component* cSource) override;

      virtual void loadFromXml(const pugi::xml_node& XmlNode) override;
      virtual void saveToXml(pugi::xml_node& XmlNode) const override;

      virtual void loadFromStream(std::istream& Stream) override;

      virtual void xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream) override;

      GEPropertyEnum(GeometryType, GeometryType)
      GEPropertyEnum(RenderingMode, RenderingMode)
      GEProperty(Color, Color)
      GEProperty(Bool, Visible)
   };
}}
