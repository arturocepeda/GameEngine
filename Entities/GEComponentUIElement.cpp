
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

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;

//
//  ComponentUIElement
//
ComponentUIElement::ComponentUIElement(Entity* Owner)
   : Component(Owner)
   , eAnchor(Alignment::MiddleCenter)
   , vOffset(Vector2::Zero)
   , fAlpha(1.0f)
{
   cClassName = ObjectName("UIElement");

   cTransform = cOwner->getComponent<ComponentTransform>();
   GEAssert(cTransform);

   GERegisterPropertyEnum(ComponentUIElement, Alignment, Anchor);
   GERegisterProperty(ComponentUIElement, Vector2, Offset);
   GERegisterProperty(ComponentUIElement, Float, Alpha);
}

ComponentUIElement::~ComponentUIElement()
{
}

void ComponentUIElement::updateTransformPosition()
{
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

Alignment ComponentUIElement::getAnchor() const
{
   return eAnchor;
}

const Vector2& ComponentUIElement::getOffset() const
{
   return vOffset;
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

void ComponentUIElement::setAnchor(Alignment Anchor)
{
   eAnchor = Anchor;
   updateTransformPosition();
}

void ComponentUIElement::setOffset(const Vector2& Offset)
{
   vOffset = Offset;
   updateTransformPosition();
}

void ComponentUIElement::setAlpha(float Alpha)
{
   fAlpha = Alpha;
}
