
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
   , iClockIndex(0)
   , eSaveBehavior(EntitySaveBehavior::Save)
{
   GEAssert(Owner);
   updateFullName();
   memset(vComponents, 0, sizeof(Component*) * (uint)ComponentType::Count);

   GERegisterProperty(ObjectName, Name);
   GERegisterProperty(Bool, Active);
   GERegisterProperty(UInt, ClockIndex);
   GERegisterPropertyEnum(EntitySaveBehavior, SaveBehavior);
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
      sprintf(sBuffer, "%s/%s", cParent->cFullName.getString().c_str(), cName.getString().c_str());
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
   if(!bActive)
      return false;

   Entity* cCurrentParent = cParent;

   while(cCurrentParent)
   {
      if(!cCurrentParent->isActive())
         return false;

      cCurrentParent = cCurrentParent->getParent();
   }

   return true;
}

void Entity::setActive(bool Active)
{
   bActive = Active;
}

bool Entity::getActive() const
{
   return bActive;
}

uint Entity::getClockIndex() const
{
   return iClockIndex;
}

void Entity::setClockIndex(uint ClockIndex)
{
   GEAssert(ClockIndex < Time::ClocksCount);
   iClockIndex = ClockIndex;
}

EntitySaveBehavior Entity::getSaveBehavior() const
{
   return eSaveBehavior;
}

void Entity::setSaveBehavior(EntitySaveBehavior Behavior)
{
   eSaveBehavior = Behavior;
}

const Core::ObjectName& Entity::getPrefabName() const
{
   return cPrefabName;
}

void Entity::setPrefabName(const Core::ObjectName& Name)
{
   cPrefabName = Name;
}
