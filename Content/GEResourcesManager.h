
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
#include "Core/GEApplication.h"
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

      void registerSimpleResourceTypes();
      void registerSerializableResourceTypes();

      void loadBuiltInMeshes();

   public:
      ResourcesManager();
      ~ResourcesManager();

      template<typename T>
      void registerObjectManager(const Core::ObjectName& Name, const Core::ObjectManager<T>* Mgr)
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

#if defined (GE_EDITOR_SUPPORT)
      typedef GESTLMap(uint32_t, GESTLString) ResourceGroupSourceFileRegistry;
      typedef GESTLMap(uint32_t, ResourceGroupSourceFileRegistry) SourceFileRegistry;
      SourceFileRegistry mSourceFileRegistry;
#endif

   public:
      SerializableResourcesManager();
      ~SerializableResourcesManager();

      template<typename T>
      void registerSerializableResourceType(const Core::ObjectManager<T>* Manager)
      {
         SerializableResourceManagerObjects sManagerObjects;
         sManagerObjects.Registry = const_cast<Core::ObjectRegistry*>(Manager->getObjectRegistry());
         sManagerObjects.Factory = Core::Allocator::alloc<SerializableResourceFactory<T>>();

         GEInvokeCtor(SerializableResourceFactory<T>, sManagerObjects.Factory)(T::TypeName);
         vEntries.push_back(sManagerObjects);

         ResourcesManager::getInstance()->registerObjectManager<T>(T::TypeName, Manager);
      }

      uint getEntriesCount() const;

      SerializableResourceManagerObjects* getEntry(uint Index);
      SerializableResourceManagerObjects* getEntry(const Core::ObjectName& ResourceTypeName);

      template<typename T>
      void add(T* pEntry)
      {
         SerializableResourceManagerObjects* cObjects = getEntry(T::TypeName);
         (*cObjects->Registry)[pEntry->getName().getID()] = pEntry;
      }

      template<typename T>
      T* get(const Core::ObjectName& Name)
      {
         SerializableResourceManagerObjects* cObjects = getEntry(T::TypeName);
         Core::ObjectRegistry::const_iterator it = cObjects->Registry->find(Name.getID());

         if(it != cObjects->Registry->end())
         {
            return static_cast<T*>(it->second);
         }

         return 0;
      }

      template<typename T>
      void load(const Core::ObjectName& GroupName)
      {
         if(Core::Application::ContentType == Core::ApplicationContentType::Xml)
         {
            char extension[32];
            sprintf(extension, "%s.xml", T::Extension);

            ContentData cContentData;
            Core::Device::readContentFile(ContentType::GenericTextData,
               T::SubDir, GroupName.getString(), extension, &cContentData);

   #if defined (GE_EDITOR_SUPPORT)
            registerSourceFile(T::TypeName, GroupName, T::SubDir, extension);
   #endif

            char sRootNode[32];
            sprintf(sRootNode, "%sList", T::TypeName.getString());

            pugi::xml_document xml;
            xml.load_buffer(cContentData.getData(), cContentData.getDataSize());
            pugi::xml_node xmlEntries = xml.child(sRootNode);

            SerializableResourceManagerObjects* cObjects =
               SerializableResourcesManager::getInstance()->getEntry(T::TypeName.getString());

            for(const pugi::xml_node& xmlEntry : xmlEntries.children(T::TypeName.getString()))
            {
               T* cInstance = Core::Allocator::alloc<T>();
               GEInvokeCtor(T, cInstance)(xmlEntry.attribute("name").value(), GroupName);
               cInstance->loadFromXml(xmlEntry);
               (*cObjects->Registry)[cInstance->getName().getID()] = cInstance;
            }
         }
         else
         {
            char extension[32];
            sprintf(extension, "%s.ge", T::Extension);

#if defined (GE_EDITOR_SUPPORT)
            //TODO: remove this when the data packaging for builds is fully finished
            if(!Core::Device::contentFileExists(T::SubDir, GroupName.getString(), extension))
            {
               Core::Application::ContentType = Core::ApplicationContentType::Xml;
               load<T>(GroupName);
               Core::Application::ContentType = Core::ApplicationContentType::Bin;
               return;
            }
#endif

            ContentData contentData;
            Core::Device::readContentFile(ContentType::GenericBinaryData,
               T::SubDir, GroupName.getString(), extension, &contentData);
            ContentDataMemoryBuffer memoryBuffer(contentData);
            std::istream stream(&memoryBuffer);

            SerializableResourceManagerObjects* objects =
               SerializableResourcesManager::getInstance()->getEntry(T::TypeName.getString());

            const uint32_t entriesCount = (uint32_t)Value::fromStream(ValueType::Byte, stream).getAsByte();

            for(uint32_t i = 0u; i < entriesCount; i++)
            {
               const Core::ObjectName entryName = Value::fromStream(ValueType::ObjectName, stream).getAsObjectName();

               T* entry = Core::Allocator::alloc<T>();
               GEInvokeCtor(T, entry)(entryName, GroupName);
               entry->loadFromStream(stream);
               (*objects->Registry)[entry->getName().getID()] = entry;
            }
         }
      }

      template<typename T>
      void unload(const Core::ObjectName& GroupName)
      {
         if(Core::Application::ContentType == Core::ApplicationContentType::Xml)
         {
            char extension[32];
            sprintf(extension, "%s.xml", T::Extension);

            ContentData cContentData;
            Core::Device::readContentFile(ContentType::GenericTextData,
               T::SubDir, GroupName.getString(), extension, &cContentData);

            char sRootNode[32];
            sprintf(sRootNode, "%sList", T::TypeName.getString());

            pugi::xml_document xml;
            xml.load_buffer(cContentData.getData(), cContentData.getDataSize());
            pugi::xml_node xmlEntries = xml.child(sRootNode);

            SerializableResourceManagerObjects* cObjects =
               SerializableResourcesManager::getInstance()->getEntry(T::TypeName.getString());

            for(const pugi::xml_node& xmlEntry : xmlEntries.children(T::TypeName.getString()))
            {
               const Core::ObjectName entryName = Core::ObjectName(xmlEntry.attribute("name").value());
               Core::ObjectRegistry::const_iterator it = cObjects->Registry->find(entryName.getID());

               if(it != cObjects->Registry->end())
               {
                  T* entry = static_cast<T*>(it->second);
                  GEInvokeDtor(T, entry);
                  Core::Allocator::free(entry);
                  cObjects->Registry->erase(entryName.getID());
               }
            }
         }
         else
         {
            char extension[32];
            sprintf(extension, "%s.ge", T::Extension);

            ContentData contentData;
            Core::Device::readContentFile(ContentType::GenericBinaryData,
               T::SubDir, GroupName.getString(), extension, &contentData);
            ContentDataMemoryBuffer memoryBuffer(contentData);
            std::istream stream(&memoryBuffer);

            SerializableResourceManagerObjects* objects =
               SerializableResourcesManager::getInstance()->getEntry(T::TypeName.getString());

            const uint32_t entriesCount = (uint32_t)Value::fromStream(ValueType::Byte, stream).getAsByte();

            for(uint32_t i = 0u; i < entriesCount; i++)
            {
               const Core::ObjectName entryName = Value::fromStream(ValueType::ObjectName, stream).getAsObjectName();
               Core::ObjectRegistry::const_iterator it = objects->Registry->find(entryName.getID());
               GEAssert(it != objects->Registry->end());

               T* entry = static_cast<T*>(it->second);
               entry->loadFromStream(stream);
               GEInvokeDtor(T, entry);
               Core::Allocator::free(entry);
               objects->Registry->erase(entryName.getID());
            }
         }
      }

#if defined (GE_EDITOR_SUPPORT)
      void registerSourceFile(const Core::ObjectName& pResourceType, const Core::ObjectName& pGroupName,
         const char* pSubDir, const char* pExtension);
      GESTLString* retrieveSourceFile(const Core::ObjectName& pResourceType, const Core::ObjectName& pGroupName);
#endif
   };
}}
