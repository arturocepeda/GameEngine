
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
#include "Core/GEDevice.h"
#include "Types/GECurve.h"
#include "Types/GEBezierCurve.h"

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
      Core::ObjectManager<Curve> mCurves;
      Core::ObjectManager<BezierCurve> mBezierCurves;

      GESTLMap(uint, void*) mSimpleResourceManagersRegistry;
      Core::ObjectManager<Mesh> mMeshes;
      Core::ObjectManager<Skeleton> mSkeletons;
      Core::ObjectManager<AnimationSet> mAnimationSets;

      void registerSerializableResourceTypes();

      void loadBuiltInMeshes();

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

         Core::ObjectManager<T>* cObjectManager =
            static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::TypeName.getID()]);
         cObjectManager->add(cContentInstance);

         return cContentInstance;
      }
      
      template<typename T>
      bool unload(const char* FileName)
      {
         Core::ObjectManager<T>* cObjectManager =
            static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::TypeName.getID()]);
         GEAssert(cObjectManager->get(Core::ObjectName(FileName)));
         return cObjectManager->remove(Core::ObjectName(FileName));
      }

      template<typename T>
      void add(T* cContentInstance)
      {
         GEAssert(cContentInstance);
         Core::ObjectManager<T>* cObjectManager =
            static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::TypeName.getID()]);
         cObjectManager->add(cContentInstance);
      }

      template<typename T>
      T* get(const Core::ObjectName& Name)
      {
         Core::ObjectManager<T>* cObjectManager =
            static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::TypeName.getID()]);
         return cObjectManager->get(Name);
      }

      template<typename T>
      bool remove(const Core::ObjectName& Name)
      {
         Core::ObjectManager<T>* cObjectManager =
            static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::TypeName.getID()]);
         return cObjectManager->remove(Name);
      }

      template<typename T>
      void clear()
      {
         Core::ObjectManager<T>* cObjectManager =
            static_cast<Core::ObjectManager<T>*>(mSimpleResourceManagersRegistry[T::TypeName.getID()]);
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
      virtual void destroy(Resource* Ptr) = 0;

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
         T* cResource = Core::Allocator::alloc<T>();
         GEInvokeCtor(T, cResource)(Name, GroupName);
         GEAssert(static_cast<Resource*>(cResource)->getClassName() == cResourceTypeName);
         return static_cast<Resource*>(cResource);
      }

      virtual void destroy(Resource* Ptr) override
      {
         T* cResource = static_cast<T*>(Ptr);
         GEInvokeDtor(T, cResource);
         Core::Allocator::free(cResource);
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
         sManagerObjects.Registry = const_cast<Core::ObjectRegistry*>(Manager->getObjectRegistry());
         sManagerObjects.Factory = Core::Allocator::alloc<SerializableResourceFactory<T>>();
         GEInvokeCtor(SerializableResourceFactory<T>, sManagerObjects.Factory)(ResourceTypeName);
         vEntries.push_back(sManagerObjects);
      }

      uint getEntriesCount() const;

      SerializableResourceManagerObjects* getEntry(uint Index);
      SerializableResourceManagerObjects* getEntry(const Core::ObjectName& ResourceTypeName);

      template<typename T>
      void loadFromXml(const Core::ObjectName& TypeName, const Core::ObjectName& GroupName,
         const char* SubDir, const char* FileName, const char* FileExtension)
      {
         ContentData cContentData;
         Core::Device::readContentFile(ContentType::GenericTextData, SubDir, FileName, FileExtension, &cContentData);

         char sRootNode[32];
         sprintf(sRootNode, "%sList", TypeName.getString());

         pugi::xml_document xml;
         xml.load_buffer(cContentData.getData(), cContentData.getDataSize());
         pugi::xml_node xmlEntries = xml.child(sRootNode);

         SerializableResourceManagerObjects* cObjects =
            SerializableResourcesManager::getInstance()->getEntry(TypeName.getString());

         for(const pugi::xml_node& xmlEntry : xmlEntries.children(TypeName.getString()))
         {
            T* cInstance = Allocator::alloc<T>();
            GEInvokeCtor(T, cInstance)(xmlEntry.attribute("name").value(), GroupName);
            cInstance->loadFromXml(xmlEntry);
            (*cObjects->Registry)[cInstance->getName().getID()] = cInstance;
         }
      }
   };
}}
