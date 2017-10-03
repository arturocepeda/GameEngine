
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEResourcesManager.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEObjectManager.h"
#include "Core/GESingleton.h"

#include "GEResource.h"
#include "GEMesh.h"
#include "GESkeleton.h"
#include "GEAnimation.h"

namespace GE { namespace Content
{
   class ResourcesManager : public Core::Singleton<ResourcesManager>
   {
   private:
      GESTLMap(uint, const Core::ObjectRegistry*) mResourcesRegistry;

      GESTLMap(ResourceType, void*) mSimpleResourceManagersRegistry;
      Core::ObjectManager<Mesh> mMeshes;
      Core::ObjectManager<Skeleton> mSkeletons;
      Core::ObjectManager<AnimationSet> mAnimationSets;

      void loadBuiltInMeshes();
      void loadBuiltInSkeleton();

   public:
      ResourcesManager();
      ~ResourcesManager();

      template<typename T>
      void registerObjectManager(const Core::ObjectName& Name, Core::ObjectManager<T>* Mgr)
      {
         mResourcesRegistry[Name.getID()] = Mgr->getObjectRegistry();
      }

      const Core::ObjectRegistry* getObjectRegistry(const Core::ObjectName& Name)
      {
         return mResourcesRegistry.find(Name.getID())->second;
      }

      template<typename T>
      T* load(const char* FileName)
      {
         T* cContentInstance = Core::Allocator::alloc<T>();
         GEInvokeCtor(T, cContentInstance)(FileName);

         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::Type]);
         cObjectManager->add(cContentInstance);

         return cContentInstance;
      }

      template<typename T>
      bool unload(const char* FileName)
      {
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::Type]);
         GEAssert(cObjectManager->get(Core::ObjectName(FileName)));
         return cObjectManager->remove(Core::ObjectName(FileName));
      }

      template<typename T>
      void add(T* cContentInstance)
      {
         GEAssert(cContentInstance);
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::Type]);
         cObjectManager->add(cContentInstance);
      }

      template<typename T>
      T* get(const Core::ObjectName& Name)
      {
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::Type]);
         return cObjectManager->get(Name);
      }

      template<typename T>
      bool remove(const Core::ObjectName& Name)
      {
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::Type]);
         return cObjectManager->remove(Name);
      }

      template<typename T>
      void clear()
      {
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::Type]);
         cObjectManager->clear();
      }
   };


   class SerializableResourceAbstractFactory
   {
   protected:
      Core::ObjectName cResourceTypeName;

      SerializableResourceAbstractFactory(const Core::ObjectName& ResourceTypeName)
         : cResourceTypeName(ResourceTypeName)
      {
      }

   public:
      virtual Resource* create(const Core::ObjectName& Name, const Core::ObjectName& GroupName) = 0;

      const Core::ObjectName& getResourceTypeName() const { return cResourceTypeName; }
   };


   template<typename T>
   class SerializableResourceFactory : public SerializableResourceAbstractFactory
   {
   public:
      SerializableResourceFactory(const Core::ObjectName& ResourceTypeName)
         : SerializableResourceAbstractFactory(ResourceTypeName)
      {
      }

      virtual Resource* create(const Core::ObjectName& Name, const Core::ObjectName& GroupName) override
      {
         T* cSerializableResource = Core::Allocator::alloc<T>();
         GEInvokeCtor(T, cSerializableResource)(Name, GroupName);
         GEAssert(static_cast<Core::Serializable*>(cSerializableResource)->getClassName() == cResourceTypeName);
         return static_cast<Resource*>(cSerializableResource);
      }
   };


   struct SerializableResourceManagerObjects
   {
      Core::ObjectRegistry* Registry;
      SerializableResourceAbstractFactory* Factory;
   };


   class SerializableResourcesManager : public Core::Singleton<SerializableResourcesManager>
   {
   private:
      typedef GESTLVector(SerializableResourceManagerObjects) SerializableResourceManagerObjectsList;

      SerializableResourceManagerObjectsList vEntries;

   public:
      SerializableResourcesManager();
      ~SerializableResourcesManager();

      template<typename T>
      void registerSerializableResourceType(const Core::ObjectName& ResourceTypeName, const Core::ObjectManager<T>* Manager)
      {
         SerializableResourceManagerObjects sManagerObjects;
         sManagerObjects.Registry = const_cast<ObjectRegistry*>(Manager->getObjectRegistry());
         sManagerObjects.Factory = Core::Allocator::alloc<SerializableResourceFactory<T>>();
         GEInvokeCtor(SerializableResourceFactory<T>, sManagerObjects.Factory)(ResourceTypeName);
         vEntries.push_back(sManagerObjects);
      }

      uint getEntriesCount() const;

      SerializableResourceManagerObjects* getEntry(uint Index);
      SerializableResourceManagerObjects* getEntry(const Core::ObjectName& ResourceTypeName);
   };
}}
