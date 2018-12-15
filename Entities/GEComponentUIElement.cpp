
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
const ObjectName ComponentUIElement::ClassName = ObjectName("UIElement");

ComponentUIElement::ComponentUIElement(Entity* Owner)
   : Component(Owner)
   , fAlpha(1.0f)
{
   mClassNames.push_back(ClassName);

   GEAssert(cOwner->getComponent<ComponentTransform>());

   GERegisterProperty(Float, Alpha);
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
const ObjectName ComponentUI2DElement::ClassName = ObjectName("UI2DElement");

ComponentUI2DElement::ComponentUI2DElement(Entity* Owner)
   : ComponentUIElement(Owner)
   , eAnchor(Alignment::None)
   , vOffset(Vector2::Zero)
   , fScaledYOffset(0.0f)
{
   mClassNames.push_back(ClassName);

#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::connectStaticEventCallback(Events::RenderingSurfaceChanged, this, [this](const EventArgs* args) -> bool
   {
      updateTransformPosition();
      return false;
   });
#endif

   GERegisterPropertyEnum(Alignment, Anchor);
   GERegisterProperty(Vector2, Offset);
   GERegisterProperty(Float, ScaledYOffset);
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

   vNewPosition.Y += fScaledYOffset * Device::getAspectRatio();

   cOwner->getComponent<ComponentTransform>()->setPosition(vNewPosition);
}

Alignment ComponentUI2DElement::getAnchor() const
{
   return eAnchor;
}

const Vector2& ComponentUI2DElement::getOffset() const
{
   return vOffset;
}

float ComponentUI2DElement::getScaledYOffset() const
{
   return fScaledYOffset;
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

void ComponentUI2DElement::setScaledYOffset(float Offset)
{
   fScaledYOffset = Offset;
   updateTransformPosition();
}


//
//  ComponentUI3DElement
//
const ObjectName ComponentUI3DElement::ClassName = ObjectName("UI3DElement");

ComponentUI3DElement::ComponentUI3DElement(Entity* Owner)
   : ComponentUIElement(Owner)
   , iCanvasIndex(0)
{
   mClassNames.push_back(ClassName);

   Entity* cParent = cOwner->getParent();

   if(cParent)
   {
      ComponentUIElement* cUIElement = cParent->getComponent<ComponentUIElement>();

      if(cUIElement && cUIElement->is(ComponentUI3DElement::ClassName))
      {
         iCanvasIndex = static_cast<ComponentUI3DElement*>(cUIElement)->getCanvasIndex();
      }
   }

   GERegisterProperty(Byte, CanvasIndex);
}

ComponentUI3DElement::~ComponentUI3DElement()
{
}

uint8_t ComponentUI3DElement::getCanvasIndex() const
{
   return iCanvasIndex;
}

void ComponentUI3DElement::setCanvasIndex(uint8_t Index)
{
   iCanvasIndex = Index;

   for(uint i = 0; i < cOwner->getChildrenCount(); i++)
   {
      ComponentUIElement* cUIElement = cOwner->getChildByIndex(i)->getComponent<ComponentUIElement>();

      if(cUIElement && cUIElement->is(ComponentUI3DElement::ClassName))
      {
         static_cast<ComponentUI3DElement*>(cUIElement)->setCanvasIndex(Index);
      }
   }
}


//
//  ComponentUI3DCanvas
//
const ObjectName ComponentUI3DCanvas::ClassName = ObjectName("UI3DCanvas");

ComponentUI3DCanvas::ComponentUI3DCanvas(Entity* Owner)
   : ComponentUI3DElement(Owner)
   , eSettings(0)
{
   mClassNames.push_back(ClassName);

   GERegisterPropertyBitMask(CanvasSettingsBitMask, Settings);
}

ComponentUI3DCanvas::~ComponentUI3DCanvas()
{
}
