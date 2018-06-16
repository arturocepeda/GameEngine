
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

ComponentRenderable::ComponentRenderable(Entity* Owner, RenderableType RType)
   : Component(Owner)
   , iRenderableType(RType)
   , eGeometryType(GeometryType::Dynamic)
   , eRenderingMode(RenderingMode::_2D)
   , iRenderPriority(128)
   , cColor(1.0f, 1.0f, 1.0f)
   , iInternalFlags((uint8_t)InternalFlags::Visible)
{
   cClassName = ObjectName("Renderable");

   cTransform = cOwner->getComponent<ComponentTransform>();
   GEAssert(cTransform);

   GERegisterProperty(Bool, Visible);
   GERegisterPropertyEnum(GeometryType, GeometryType);
   GERegisterPropertyEnum(RenderingMode, RenderingMode);
   GERegisterPropertyMinMax(Byte, RenderPriority, 0, 255);
   GERegisterProperty(Color, Color);
   GERegisterPropertyArray(MaterialPass);
}

ComponentRenderable::~ComponentRenderable()
{
   GEReleasePropertyArray(MaterialPass);
}

void ComponentRenderable::show()
{
   GESetFlag(iInternalFlags, InternalFlags::Visible);
}

void ComponentRenderable::hide()
{
   GEResetFlag(iInternalFlags, InternalFlags::Visible);
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

uint8_t ComponentRenderable::getRenderPriority() const
{
   return iRenderPriority;
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
   return GEHasFlag(iInternalFlags, InternalFlags::Visible);
}

uint8_t ComponentRenderable::getInternalFlags() const
{
   return iInternalFlags;
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

void ComponentRenderable::setRenderPriority(uint8_t Priority)
{
   iRenderPriority = Priority;
}

void ComponentRenderable::setColor(const Color& C)
{
   cColor = C;
}

void ComponentRenderable::setVisible(bool Visible)
{
   if(Visible)
      show();
   else
      hide();
}

void ComponentRenderable::setInternalFlags(uint8_t Flags)
{
   iInternalFlags = Flags;
}
