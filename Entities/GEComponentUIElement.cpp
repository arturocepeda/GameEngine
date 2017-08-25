
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentUIElement.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentUIElement.h"
#include "GEEntity.h"
#include "Core/GEDevice.h"
#include "Core/GEEvents.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;

//
//  ComponentUIElement
//
ComponentUIElement::ComponentUIElement(Entity* Owner)
   : Component(Owner)
   , fAlpha(1.0f)
   , iRenderPriority(128)
{
   cClassName = ObjectName("UIElement");

   cTransform = cOwner->getComponent<ComponentTransform>();
   GEAssert(cTransform);

   GERegisterPropertyMinMax(Float, Alpha, 0.0f, 1.0f);
   GERegisterPropertyMinMax(Byte, RenderPriority, 0, 255);
}

ComponentUIElement::~ComponentUIElement()
{
}

float ComponentUIElement::getAlpha() const
{
   return fAlpha;
}

float ComponentUIElement::getAlphaInHierarchy() const
{
   float fAlphaInHierarchy = fAlpha;   
   Entity* cParent = cOwner->getParent();

   while(cParent)
   {
      ComponentUIElement* cUIElement = cParent->getComponent<ComponentUIElement>();

      if(cUIElement)
         fAlphaInHierarchy *= cUIElement->getAlpha();

      cParent = cParent->getParent();
   }

   return fAlphaInHierarchy;
}

uint8_t ComponentUIElement::getRenderPriority() const
{
   return iRenderPriority;
}

void ComponentUIElement::setAlpha(float Alpha)
{
   fAlpha = Alpha;
}

void ComponentUIElement::setRenderPriority(uint8_t Value)
{
   iRenderPriority = Value;
}

void ComponentUIElement::changeRenderPriority(int8_t Delta)
{
   iRenderPriority += Delta;

   for(uint i = 0; i < cOwner->getChildrenCount(); i++)
   {
      ComponentUIElement* cChildUIElement = cOwner->getChildByIndex(i)->getComponent<ComponentUIElement>();

      if(cChildUIElement)
      {
         cChildUIElement->changeRenderPriority(Delta);
      }
   }
}


//
//  ComponentUI2DElement
//
ComponentUI2DElement::ComponentUI2DElement(Entity* Owner)
   : ComponentUIElement(Owner)
   , eAnchor(Alignment::None)
   , vOffset(Vector2::Zero)
{
   cClassName = ObjectName("UI2DElement");

#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::connectStaticEventCallback(Events::RenderingSurfaceChanged, this, [this](const EventArgs* args) -> bool
   {
      updateTransformPosition();
      return false;
   });
#endif

   GERegisterPropertyEnum(Alignment, Anchor);
   GERegisterProperty(Vector2, Offset);
}

ComponentUI2DElement::~ComponentUI2DElement()
{
#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::disconnectStaticEventCallback(Events::RenderingSurfaceChanged, this);
#endif
}

void ComponentUI2DElement::updateTransformPosition()
{
   if(eAnchor == Alignment::None)
      return;

   Vector3 vNewPosition = Vector3(vOffset.X, vOffset.Y, 0.0f);

   switch(eAnchor)
   {
   case Alignment::TopLeft:
      vNewPosition.X += -1.0f;
      vNewPosition.Y += Device::getAspectRatio();
      break;
   case Alignment::TopCenter:
      vNewPosition.Y += Device::getAspectRatio();
      break;
   case Alignment::TopRight:
      vNewPosition.X += 1.0f;
      vNewPosition.Y += Device::getAspectRatio();
      break;
   case Alignment::MiddleLeft:
      vNewPosition.X += -1.0f;
      break;
   case Alignment::MiddleCenter:
      break;
   case Alignment::MiddleRight:
      vNewPosition.X += 1.0f;
      break;
   case Alignment::BottomLeft:
      vNewPosition.X += -1.0f;
      vNewPosition.Y += -Device::getAspectRatio();
      break;
   case Alignment::BottomCenter:
      vNewPosition.Y += -Device::getAspectRatio();
      break;
   case Alignment::BottomRight:
      vNewPosition.X += 1.0f;
      vNewPosition.Y += -Device::getAspectRatio();
      break;
   default:
      break;
   }

   cTransform->setPosition(vNewPosition);
}

Alignment ComponentUI2DElement::getAnchor() const
{
   return eAnchor;
}

const Vector2& ComponentUI2DElement::getOffset() const
{
   return vOffset;
}

void ComponentUI2DElement::setAnchor(Alignment Anchor)
{
   eAnchor = Anchor;
   updateTransformPosition();
}

void ComponentUI2DElement::setOffset(const Vector2& Offset)
{
   vOffset = Offset;
   updateTransformPosition();
}


//
//  ComponentUI3DElement
//
ComponentUI3DElement::ComponentUI3DElement(Entity* Owner)
   : ComponentUIElement(Owner)
{
   cClassName = ObjectName("UI3DElement");
}

ComponentUI3DElement::~ComponentUI3DElement()
{
}
