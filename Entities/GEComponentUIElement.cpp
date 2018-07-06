
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
   , eUIElementType(UIElementType::_2D)
{
   cClassName = ObjectName("UIElement");

   GEAssert(cOwner->getComponent<ComponentTransform>());

   GERegisterProperty(Float, Alpha);
}

ComponentUIElement::~ComponentUIElement()
{
}

UIElementType ComponentUIElement::getUIElementType() const
{
   return eUIElementType;
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
   , eAnchor(Alignment::None)
   , vOffset(Vector2::Zero)
{
   cClassName = ObjectName("UI2DElement");
   eUIElementType = UIElementType::_2D;

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
   , iCanvasIndex(0)
{
   cClassName = ObjectName("UI3DElement");
   eUIElementType = UIElementType::_3D;

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

      if(cUIElement && cUIElement->getUIElementType() != UIElementType::_2D)
      {
         static_cast<ComponentUI3DElement*>(cUIElement)->setCanvasIndex(Index);
      }
   }
}


//
//  ComponentUI3DCanvas
//
ComponentUI3DCanvas::ComponentUI3DCanvas(Entity* Owner)
   : ComponentUI3DElement(Owner)
   , eSettings(0)
{
   cClassName = ObjectName("UI3DCanvas");
   eUIElementType = UIElementType::_3DCanvas;

   GERegisterPropertyBitMask(CanvasSettingsBitMask, Settings);
}

ComponentUI3DCanvas::~ComponentUI3DCanvas()
{
}
