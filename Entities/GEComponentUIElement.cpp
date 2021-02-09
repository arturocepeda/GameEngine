
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
   , mInputEnabled(false)
   , mInputTabOrder(0u)
{
   mClassNames.push_back(ClassName);

   GEAssert(cOwner->getComponent<ComponentTransform>());

   GERegisterProperty(Float, Alpha);
   GERegisterPropertyReadonly(Float, AlphaInHierarchy);
   GERegisterProperty(Bool, InputEnabled);
   GERegisterProperty(Byte, InputTabOrder);
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
   , mOffsetMode(OffsetMode::Absolute)
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
   GERegisterPropertyEnum(OffsetMode, OffsetMode);
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

   if(mOffsetMode == OffsetMode::Relative)
   {
      vNewPosition.Y *= Device::getAspectRatio();
   }

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

OffsetMode ComponentUI2DElement::getOffsetMode() const
{
   return mOffsetMode;
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

void ComponentUI2DElement::setOffsetMode(OffsetMode pMode)
{
   mOffsetMode = pMode;
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
   , eSettings(0u)
{
   mClassNames.push_back(ClassName);

   GERegisterPropertyBitMask(CanvasSettingsBitMask, Settings);
}

ComponentUI3DCanvas::~ComponentUI3DCanvas()
{
}
