
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEEntity.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEEntity.h"
#include "GEComponent.h"
#include "Core/GEAllocator.h"
#include "Core/GETime.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;

//
//  Entity
//
ComponentFactoryList Entity::vComponentFactories;

Entity::Entity(const ObjectName& Name, Entity* Parent, Scene* Owner)
   : EventHandlingObject(Name)
   , Serializable("Entity")
   , cParent(Parent)
   , cOwner(Owner)
   , bActive(true)
   , bInitialized(false)
   , mClock(0)
   , iInternalFlags(0)
{
   GEAssert(Owner);
   updateFullName();
   memset(vComponents, 0, sizeof(Component*) * (uint)ComponentType::Count);

   mClock = cParent ? cParent->getClock() : Time::getDefaultClock();

   GERegisterProperty(Bool, Active);
   GERegisterProperty(ObjectName, ClockName);
   GERegisterProperty(ObjectName, PrefabName);
}

Entity::~Entity()
{
   for(uint i = 0; i < (uint)ComponentType::Count; i++)
   {
      if(vComponents[i])
      {
         GEInvokeDtor(Component, vComponents[i]);
         Allocator::free(vComponents[i]);
      }
   }
}

Component* Entity::addComponent(const Core::ObjectName& TypeName)
{
   ComponentAbstractFactory* cFactory = 0;

   for(uint i = 0; i < vComponentFactories.size(); i++)
   {
      if(vComponentFactories[i].ComponentName == TypeName)
      {
         cFactory = vComponentFactories[i].Factory;
         break;
      }
   }

   GEAssert(cFactory);

   ComponentType eComponentType = cFactory->getComponentType();
   GEAssert(!getComponent(eComponentType));

   Component* cComponent = cFactory->create(this);
   vComponents[(uint)eComponentType] = cComponent;

   if(bInitialized)
      cOwner->registerComponent(eComponentType, cComponent);

   return cComponent;
}

Component* Entity::getComponent(const Core::ObjectName& TypeName)
{
   ComponentAbstractFactory* cFactory = 0;

   for(uint i = 0; i < vComponentFactories.size(); i++)
   {
      if(vComponentFactories[i].ComponentName == TypeName)
      {
         cFactory = vComponentFactories[i].Factory;
         break;
      }
   }

   GEAssert(cFactory);

   ComponentType eComponentType = cFactory->getComponentType();

   return vComponents[(uint)eComponentType];
}

Component* Entity::getOrAddComponent(const Core::ObjectName& TypeName)
{
   Component* cComponent = getComponent(TypeName);

   if(!cComponent)
      cComponent = addComponent(TypeName);

   return cComponent;
}

void Entity::removeComponent(const Core::ObjectName& ComponentName)
{
   Component* cComponent = 0;
   ComponentType eComponentType = ComponentType::Count;

   for(uint i = 0; i < (uint)ComponentType::Count; i++)
   {
      if(vComponents[i] && vComponents[i]->getClassName() == ComponentName)
      {
         cComponent = vComponents[i];
         eComponentType = (ComponentType)i;
         vComponents[i] = 0;
         break;
      }
   }

   GEAssert(cComponent);

   cOwner->removeComponent(eComponentType, cComponent);
   GEInvokeDtor(Component, cComponent);
   Allocator::free(cComponent);
}

const ObjectName& Entity::getFullName() const
{
   return cFullName;
}

Component* Entity::getComponent(ComponentType Type) const
{
   return vComponents[(uint)Type];
}

void Entity::addChild(Entity* Child)
{
   vChildren.push_back(Child);
   Child->cParent = this;
}

void Entity::init()
{
   GEAssert(!bInitialized);

   for(uint i = 0; i < (uint)ComponentType::Count; i++)
   {
      if(vComponents[i])
         cOwner->registerComponent((ComponentType)i, vComponents[i]);
   }

   for(uint i = 0; i < vChildren.size(); i++)
      vChildren[i]->init();

   bInitialized = true;
}

uint Entity::getChildrenCount() const
{
   return (uint)vChildren.size();
}

Entity* Entity::getChildByIndex(uint Index) const
{
   GEAssert(Index < vChildren.size());
   return vChildren[Index];
}

Entity* Entity::getChildByName(const Core::ObjectName& Name) const
{
   for(uint i = 0; i < vChildren.size(); ++i)
   {
      if(vChildren[i]->getName() == Name)
         return vChildren[i];
   }

   return 0;
}

Entity* Entity::getParent() const
{
   return cParent;
}

void Entity::updateFullName()
{
   if(cParent)
   {
      char sBuffer[512];
      sprintf(sBuffer, "%s/%s", cParent->cFullName.getString(), cName.getString());
      cFullName = ObjectName(sBuffer);
   }
   else
   {
      cFullName = cName;
   }
}

Scene* Entity::getOwner() const
{
   return cOwner;
}

void Entity::setName(const Core::ObjectName& Name)
{
   if(Name == cName)
      return;

   cOwner->renameEntity(this, Name);
}

bool Entity::isActive() const
{
   return bActive;
}

bool Entity::isActiveInHierarchy() const
{
   return bActive && (!cParent || cParent->isActiveInHierarchy());
}

void Entity::setActive(bool Active)
{
   bActive = Active;
}

bool Entity::getActive() const
{
   return bActive;
}

const ObjectName& Entity::getClockName() const
{
   return mClock ? mClock->getName() : ObjectName::Empty;
}

void Entity::setClockName(const ObjectName& pClockName)
{
   Clock* cClock = Time::getClock(pClockName);

   if(!cClock)
   {
      cClock = Time::getDefaultClock();
   }

   setClock(cClock);
}

void Entity::setClock(Core::Clock* pClock)
{
   mClock = pClock;

   for(size_t i = 0; i < vChildren.size(); i++)
   {
      vChildren[i]->setClock(pClock);
   }
}

const ObjectName& Entity::getPrefabName() const
{
   return cPrefabName;
}

void Entity::setPrefabName(const ObjectName& Name)
{
   cPrefabName = Name;
}

uint8_t Entity::getInternalFlags() const
{
   return iInternalFlags;
}

void Entity::setInternalFlags(uint8_t Flags)
{
   iInternalFlags = Flags;
}

void Entity::mergeXmlDescription(pugi::xml_node& pXmlBase, const pugi::xml_node& pXmlDerived)
{
   Serializable::mergeXmlDescription(pXmlBase, pXmlDerived);

   for(const pugi::xml_node& xmlDerivedNode : pXmlDerived.children())
   {
      // Component
      if(strcmp(xmlDerivedNode.name(), "Component") == 0)
      {
         const char* componentType = xmlDerivedNode.attribute("type").value();
         pugi::xml_node xmlBaseComponent = pXmlBase.find_child_by_attribute("Component", "type", componentType);

         if(xmlBaseComponent.empty())
         {
            pXmlBase.append_copy(xmlDerivedNode);
         }
         else
         {
            Serializable::mergeXmlDescription(xmlBaseComponent, xmlDerivedNode);
         }
      }
      // Child entity
      else if(strcmp(xmlDerivedNode.name(), "Entity") == 0)
      {
         const char* childName = xmlDerivedNode.attribute("name").value();
         pugi::xml_node xmlBaseChild = pXmlBase.find_child_by_attribute("Entity", "name", childName);

         if(xmlBaseChild.empty())
         {
            pXmlBase.append_copy(xmlDerivedNode);
         }
         else
         {
            mergeXmlDescription(xmlBaseChild, xmlDerivedNode);
         }
      }
   }
}
