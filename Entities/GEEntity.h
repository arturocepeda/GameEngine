
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEEntity.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEScene.h"
#include "Core/GEObject.h"
#include "Core/GEUtils.h"
#include "Core/GEAllocator.h"

#include <vector>
#include <cassert>

namespace GE { namespace Entities
{
   class ComponentAbstractFactory
   {
   protected:
      ComponentType eComponentType;

      ComponentAbstractFactory(ComponentType Type) : eComponentType(Type) {}

   public:
      virtual Component* create(Entity* Owner) = 0;

      ComponentType getComponentType() const { return eComponentType; }
   };


   template<typename T>
   class ComponentFactory : public ComponentAbstractFactory
   {
   public:
      ComponentFactory(ComponentType Type) : ComponentAbstractFactory(Type) {}

      virtual Component* create(Entity* Owner) override
      {
         T* cComponent = Core::Allocator::alloc<T>();
         GEInvokeCtor(T, cComponent)(Owner);
         return cComponent;
      }
   };


   struct ComponentFactoryPair
   {
      Core::ObjectName ComponentName;
      ComponentAbstractFactory* Factory;
   };


   typedef GESTLVector(ComponentFactoryPair) ComponentFactoryList;


   class Entity : public Core::EventHandlingObject, public Core::Serializable, private Core::NonCopyable
   {
   private:
      friend class Scene;

      static ComponentFactoryList vComponentFactories;

      Core::ObjectName cFullName;
      Component* vComponents[(uint)ComponentType::Count];
      GESTLVector(Entity*) vChildren;
      Entity* cParent;
      Scene* cOwner;
      bool bActive;
      bool bInitialized;
      uint iClockIndex;
      Core::ObjectName cPrefabName;
      uint8_t iInternalFlags;

      template<typename T>
      void registerComponent(Component* cComponent)
      {
         vComponents[(uint)T::getType()] = cComponent;
      }

      void updateFullName();

   public:
      enum class InternalFlags
      {
         // Generated: the entity must not be saved in scenes or prefabs
         Generated = 1 << 0,
      };

      Entity(const Core::ObjectName& Name, Entity* Parent, Scene* Owner);
      ~Entity();

      Component* addComponent(const Core::ObjectName& TypeName);
      Component* getComponent(const Core::ObjectName& TypeName);
      Component* getOrAddComponent(const Core::ObjectName& TypeName);

      template<typename T>
      T* addComponent()
      {
         GEAssert(!getComponent(T::getType()));

         Component* cComponent = Core::Allocator::alloc<T>();
         GEInvokeCtor(T, cComponent)(this);
         registerComponent<T>(cComponent);

         if(bInitialized)
            cOwner->registerComponent<T>(cComponent);

         return static_cast<T*>(cComponent);
      }

      template<typename T>
      T* getComponent() const
      {
         return static_cast<T*>(vComponents[(uint)T::getType()]);
      }

      template<typename T>
      T* getOrAddComponent()
      {
         T* cComponent = getComponent<T>();
         return cComponent ? cComponent : addComponent<T>();
      }

      template<typename T>
      void getComponents(GESTLVector(T*)* OutComponentsList)
      {
         if(vComponents[(uint)T::getType()])
            OutComponentsList->push_back(static_cast<T*>(vComponents[(uint)T::getType()]));

         for(uint i = 0; i < vChildren.size(); i++)
            vChildren[i]->getComponents<T>(OutComponentsList);
      }

      void removeComponent(const Core::ObjectName& ComponentName);

      template<typename T>
      static void registerComponentFactory(const Core::ObjectName& ComponentName, ComponentType Type)
      {
         ComponentFactoryPair sComponentFactoryPair;
         sComponentFactoryPair.ComponentName = ComponentName;
         sComponentFactoryPair.Factory = new ComponentFactory<T>(T::getType());;

         vComponentFactories.push_back(sComponentFactoryPair);
      }

      static const ComponentFactoryList& getComponentFactoryList()
      {
         return vComponentFactories;
      }

      const Core::ObjectName& getFullName() const;
      Component* getComponent(ComponentType Type) const;

      void addChild(Entity* Child);
      void init();

      Entity* getParent() const;
      uint getChildrenCount() const;
      Entity* getChildByIndex(uint Index) const;
      Entity* getChildByName(const Core::ObjectName& Name) const;
      Scene* getOwner() const;

      void setName(const Core::ObjectName& Name);

      bool isActive() const;
      bool isActiveInHierarchy() const;
      void setActive(bool Active);
      bool getActive() const;

      uint getClockIndex() const;
      void setClockIndex(uint ClockIndex);

      const Core::ObjectName& getPrefabName() const;
      void setPrefabName(const Core::ObjectName& Name);

      uint8_t getInternalFlags() const;
      void setInternalFlags(uint8_t Flags);
   };
}}
