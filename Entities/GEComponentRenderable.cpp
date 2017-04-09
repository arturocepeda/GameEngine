
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentRenderable.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentRenderable.h"
#include "GEEntity.h"
#include "Core/GEAllocator.h"
#include "Rendering/GERenderSystem.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;
using namespace GE::Content;

//
//  ComponentRenderable
//
ushort ComponentRenderable::QuadIndices[6] = { 0, 1, 2, 3, 2, 1 };

ComponentRenderable::ComponentRenderable(Entity* Owner, RenderableType RType, GeometryType GType)
   : Component(Owner)
   , iRenderableType(RType)
   , eGeometryType(GType)
   , eRenderingMode(RenderingMode::_2D)
   , cColor(1.0f, 1.0f, 1.0f)
   , bVisible(true)
{
   cClassName = ObjectName("Renderable");

   cTransform = cOwner->getComponent<ComponentTransform>();
   GEAssert(cTransform);

   GERegisterProperty(ComponentRenderable, Bool, Visible);
   GERegisterPropertyEnum(ComponentRenderable, GeometryType, GeometryType);
   GERegisterPropertyEnum(ComponentRenderable, RenderingMode, RenderingMode);
   GERegisterProperty(ComponentRenderable, Color, Color);
   GERegisterPropertyArray(ComponentRenderable, MaterialPass);
}

ComponentRenderable::~ComponentRenderable()
{
   GEReleasePropertyArray(ComponentRenderable, MaterialPass);
}

void ComponentRenderable::show()
{
   bVisible = true;
}

void ComponentRenderable::hide()
{
   bVisible = false;
}

RenderableType ComponentRenderable::getRenderableType() const
{
   return iRenderableType;
}

GeometryType ComponentRenderable::getGeometryType() const
{
   return eGeometryType;
}

RenderingMode ComponentRenderable::getRenderingMode() const
{
   return eRenderingMode;
}

ComponentTransform* ComponentRenderable::getTransform() const
{
   return cTransform;
}

const Color& ComponentRenderable::getColor() const
{
   return cColor;
}

bool ComponentRenderable::getVisible() const
{
   return bVisible;
}

const GeometryData& ComponentRenderable::getGeometryData() const
{
   return sGeometryData;
}

void ComponentRenderable::setGeometryType(GeometryType Type)
{
   eGeometryType = Type;
}

void ComponentRenderable::setRenderingMode(RenderingMode Mode)
{
   eRenderingMode = Mode;
}

void ComponentRenderable::setColor(const Color& C)
{
   cColor = C;
}

void ComponentRenderable::setVisible(bool Visible)
{
   bVisible = Visible;
}
