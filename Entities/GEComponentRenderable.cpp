
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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

void ComponentRenderable::loadFromStream(std::istream& Stream)
{
   uint iMaterialPassCount = (uint)Value::fromStream(ValueType::Byte, Stream).getAsByte();

   for(uint i = 0; i < iMaterialPassCount; i++)
   {
      MaterialPass* cMaterialPass = addMaterialPass();
      cMaterialPass->loadFromStream(Stream);
   }

   Serializable::loadFromStream(Stream);
}

void ComponentRenderable::xmlToStream(const pugi::xml_node& XmlNode, std::ostream& Stream)
{
   pugi::xml_object_range<pugi::xml_named_node_iterator> xmlMaterialPasses = XmlNode.children("MaterialPass");
   GE::byte iMaterialPassCount = 0;
    
   for(const pugi::xml_node& xmlMaterialPass : xmlMaterialPasses)
   {
      iMaterialPassCount++;
   }

   Value(iMaterialPassCount).writeToStream(Stream);

   for(const pugi::xml_node& xmlMaterialPass : xmlMaterialPasses)
   {
      MaterialPass cMaterialPass;
      cMaterialPass.loadFromXml(xmlMaterialPass);
      cMaterialPass.xmlToStream(xmlMaterialPass, Stream);
   }

   Serializable::xmlToStream(XmlNode, Stream);
}
