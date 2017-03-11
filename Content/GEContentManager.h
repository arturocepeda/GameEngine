
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEContentManager.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEManagedContent.h"
#include "Core/GESingleton.h"
#include "Core/GEObjectManager.h"

#include "GEMesh.h"
#include "GESkeleton.h"
#include "GEAnimation.h"

namespace GE { namespace Content
{
   class ContentManager : public Core::Singleton<ContentManager>
   {
   private:
      GESTLMap(ManagedContentType, void*) mManagersRegistry;

      Core::ObjectManager<Mesh> mMeshes;
      Core::ObjectManager<Skeleton> mSkeletons;
      Core::ObjectManager<AnimationSet> mAnimationSets;

      void loadBuiltInMeshes();

   public:
      ContentManager();
      ~ContentManager();

      template<typename T>
      T* load(const char* FileName)
      {
         T* cContentInstance = Core::Allocator::alloc<T>();
         GEInvokeCtor(T, cContentInstance)(FileName);

         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mManagersRegistry[T::ContentType]);
         cObjectManager->add(cContentInstance);

         return cContentInstance;
      }

      template<typename T>
      bool unload(const char* FileName)
      {
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mManagersRegistry[T::ContentType]);
         GEAssert(cObjectManager->get(Core::ObjectName(FileName)));
         return cObjectManager->remove(Core::ObjectName(FileName));
      }

      template<typename T>
      void add(T* cContentInstance)
      {
         GEAssert(cContentInstance);
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mManagersRegistry[T::ContentType]);
         cObjectManager->add(cContentInstance);
      }

      template<typename T>
      T* get(const Core::ObjectName& Name)
      {
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mManagersRegistry[T::ContentType]);
         return cObjectManager->get(Name);
      }

      template<typename T>
      bool remove(const Core::ObjectName& Name)
      {
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mManagersRegistry[T::ContentType]);
         return cObjectManager->remove(Name);
      }

      template<typename T>
      void clear()
      {
         Core::ObjectManager<T>* cObjectManager = static_cast<Core::ObjectManager<T>*>(mManagersRegistry[T::ContentType]);
         cObjectManager->clear();
      }
   };
}}
