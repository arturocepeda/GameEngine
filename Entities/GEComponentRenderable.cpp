
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
#include "Core/GEEvents.h"
#include "Rendering/GERenderSystem.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;
using namespace GE::Content;

//
//  ComponentRenderable
//
const ushort ComponentRenderable::QuadIndices[6] = { 0, 1, 2, 3, 2, 1 };
const ObjectName ComponentRenderable::ClassName = ObjectName("Renderable");

ComponentRenderable::ComponentRenderable(Entity* Owner)
   : Component(Owner)
   , eGeometryType(GeometryType::Dynamic)
   , eRenderingMode(RenderingMode::_2D)
   , iRenderPriority(128)
   , mRenderPass(RenderPass::None)
   , cColor(1.0f, 1.0f, 1.0f)
   , iInternalFlags((uint8_t)InternalFlags::Visible)
{
   mClassNames.push_back(ClassName);

   cTransform = cOwner->getComponent<ComponentTransform>();
   GEAssert(cTransform);

   GERegisterProperty(Bool, Visible);
   GERegisterPropertyEnum(GeometryType, GeometryType);
   GERegisterPropertyEnum(RenderingMode, RenderingMode);
   GERegisterProperty(Byte, RenderPriority);
   GERegisterProperty(Color, Color);
   GERegisterPropertyEnumReadonly(RenderPass, RenderPass);
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
   cOwner->triggerEvent(Events::RenderableColorChanged);
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
