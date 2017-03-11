
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

   GERegisterPropertyEnum(ComponentRenderable, GeometryType, GeometryType);
   GERegisterPropertyEnum(ComponentRenderable, RenderingMode, RenderingMode);
   GERegisterProperty(ComponentRenderable, Color, Color);
   GERegisterProperty(ComponentRenderable, Bool, Visible);
}

ComponentRenderable::~ComponentRenderable()
{
   clearMaterialPasses();
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

MaterialPass* ComponentRenderable::addMaterialPass()
{
   MaterialPass* cMaterialPass = Allocator::alloc<MaterialPass>();
   GEInvokeCtor(MaterialPass, cMaterialPass)();

   vMaterialPassList.push_back(cMaterialPass);

   return cMaterialPass;
}

uint ComponentRenderable::getMaterialPassCount() const
{
   return (uint)vMaterialPassList.size();
}

MaterialPass* ComponentRenderable::getMaterialPass(uint Index)
{
   GEAssert(Index < vMaterialPassList.size());
   return vMaterialPassList[Index];
}

void ComponentRenderable::removeMaterialPass(uint Index)
{
   GEAssert(Index < vMaterialPassList.size());
   MaterialPass* cMaterialPass = vMaterialPassList[Index];
   GEInvokeDtor(MaterialPass, cMaterialPass);
   Allocator::free(cMaterialPass);
   vMaterialPassList.erase(vMaterialPassList.begin() + Index);
}

void ComponentRenderable::clearMaterialPasses()
{
   for(uint i = 0; i < vMaterialPassList.size(); i++)
   {
      MaterialPass* cMaterialPass = vMaterialPassList[i];
      GEInvokeDtor(MaterialPass, cMaterialPass);
      Allocator::free(cMaterialPass);
   }

   vMaterialPassList.clear();
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

void ComponentRenderable::copy(Component* cSource)
{
   ComponentRenderable* cRenderableSource = static_cast<ComponentRenderable*>(cSource);

   for(uint i = 0; i < cRenderableSource->getMaterialPassCount(); i++)
   {
      MaterialPass* cSourceMaterialPass = cRenderableSource->getMaterialPass(i);
      MaterialPass* cTargetMaterialPass = addMaterialPass();

      for(uint j = 0; j < cSourceMaterialPass->getPropertiesCount(); j++)
      {
         Value cSourceMaterialPassPropertyValue = cSourceMaterialPass->getProperty(j).Getter();
         cTargetMaterialPass->getProperty(j).Setter(cSourceMaterialPassPropertyValue);
      }
   }

   Component::copy(cSource);
}

void ComponentRenderable::loadFromXml(const pugi::xml_node& XmlNode)
{
   for(const pugi::xml_node& xmlMaterialPass : XmlNode.children("MaterialPass"))
   {
      MaterialPass* cMaterialPass = addMaterialPass();
      cMaterialPass->loadFromXml(xmlMaterialPass);
   }

   Serializable::loadFromXml(XmlNode);
}

void ComponentRenderable::saveToXml(pugi::xml_node& XmlNode) const
{
   for(uint i = 0; i < vMaterialPassList.size(); i++)
   {
      pugi::xml_node xmlMaterialPass = XmlNode.append_child("MaterialPass");
      vMaterialPassList[i]->saveToXml(xmlMaterialPass);
   }

   Serializable::saveToXml(XmlNode);
}
