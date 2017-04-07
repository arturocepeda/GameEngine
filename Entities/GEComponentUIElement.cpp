
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
   , fAlpha(1.0f)
{
   cClassName = ObjectName("UIElement");

   cTransform = cOwner->getComponent<ComponentTransform>();
   GEAssert(cTransform);

   GERegisterPropertyMinMax(ComponentUIElement, Float, Alpha, 0.0f, 1.0f);
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

void ComponentUIElement::setAlpha(float Alpha)
{
   fAlpha = Alpha;
}


//
//  ComponentUI2DElement
//
ComponentUI2DElement::ComponentUI2DElement(Entity* Owner)
   : ComponentUIElement(Owner)
   , eAnchor(Alignment::MiddleCenter)
   , vOffset(Vector2::Zero)
{
   cClassName = ObjectName("UI2DElement");

   GERegisterPropertyEnum(ComponentUI2DElement, Alignment, Anchor);
   GERegisterProperty(ComponentUI2DElement, Vector2, Offset);
}

ComponentUI2DElement::~ComponentUI2DElement()
{
}

void ComponentUI2DElement::updateTransformPosition()
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
