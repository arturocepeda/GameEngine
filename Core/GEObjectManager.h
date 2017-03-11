
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEObjectManager.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEObject.h"
#include "GEAllocator.h"
#include "GESingleton.h"

#include <map>
#include <functional>

namespace GE { namespace Core
{
   typedef GESTLMap(uint, Object*) ObjectRegistry;


   template<typename T>
   class ObjectManager
   {
   private:
      ObjectRegistry mRegistry;

   public:
      const ObjectRegistry* getObjectRegistry() const { return &mRegistry; }

      void add(T* Entry)
      {
         GEAssert(!get(Entry->getName()));
         mRegistry[Entry->getName().getID()] = Entry;
      }

      T* get(const ObjectName& Name) const
      {
         ObjectRegistry::const_iterator it = mRegistry.find(Name.getID());

         if(it == mRegistry.end())
            return 0;

         return static_cast<T*>(it->second);
      }

      bool remove(const ObjectName& Name)
      {
         ObjectRegistry::const_iterator it = mRegistry.find(Name.getID());
         
         if(it == mRegistry.end())
            return false;

         GEInvokeDtor(T, static_cast<T*>(it->second));
         Allocator::free(it->second);
         mRegistry.erase(it);

         return true;
      }

      uint count() const
      {
         return mRegistry.size();
      }

      void iterate(std::function<bool(T*)> EntryProcessor)
      {
         ObjectRegistry::const_iterator it = mRegistry.begin();

         for(; it != mRegistry.end(); it++)
         {
            if(!EntryProcessor(static_cast<T*>(it->second)))
               break;
         }
      }

      void iterateConst(std::function<bool(const T*)> EntryProcessor) const
      {
         ObjectRegistry::const_iterator it = mRegistry.begin();

         for(; it != mRegistry.end(); it++)
         {
            if(!EntryProcessor(static_cast<const T*>(it->second)))
               break;
         }
      }

      void clear()
      {
         ObjectRegistry::const_iterator it = mRegistry.begin();

         for(; it != mRegistry.end(); it++)
         {
            T* cEntry = static_cast<T*>(it->second);
            GEInvokeDtor(T, cEntry);
            Allocator::free(cEntry);
         }

         mRegistry.clear();
      }
   };


   class ObjectManagers : public Singleton<ObjectManagers>
   {
   private:
      GESTLMap(uint, const ObjectRegistry*) mObjectManagersRegistry;

   public:
      template<typename T>
      void registerObjectManager(const ObjectName& Name, ObjectManager<T>* Mgr)
      {
         mObjectManagersRegistry[Name.getID()] = Mgr->getObjectRegistry();
      }

      const ObjectRegistry* getObjectRegistry(const ObjectName& Name)
      {
         return mObjectManagersRegistry.find(Name.getID())->second;
      }
   };
}}
