
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEScene.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEObject.h"
#include "Core/GESerializable.h"
#include "Core/GEThreads.h"
#include "Content/GEContentData.h"
#include "GEComponentType.h"
#include "Externals/pugixml/pugixml.hpp"

#include <atomic>

namespace GE { namespace Entities
{
   class Entity;
   class Component;


   GESerializableEnum(SceneBackgroundMode)
   {
      SolidColor,
      SkyBox,

      Count
   };


   class Scene : public Core::EventHandlingObject, public Core::Serializable
   {
   private:
      static Scene* cActiveScene;
      static Scene* cPermanentScene;
      static Scene* cDebuggingScene;

      static GESTLMap(uint32_t, Content::ContentData) smPrefabData;

      GESTLVector(Entity*) vEntities;
      GESTLMap(uint, Entity*) mRegistry;
      GESTLVector(Component*) vComponents[(uint)ComponentType::Count];

      GESTLVector(Entity*) mEntitiesToRemove;
      std::atomic<bool> mRemovingEntities;

      SceneBackgroundMode eBackgroundMode;
      Core::ObjectName cBackgroundMaterialName;
      Entity* cBackgroundEntity;

      float fShadowsMaxDistance;

      GEMutex mSceneMutex;

      static void saveEntityContents(std::ostream& pStream, Entity* pEntity);

      void registerEntity(Entity* cEntity);
      void removeEntity(Entity* cEntity);
      void removeEntityRecursively(Entity* cEntity);

      Entity* addEntity(const pugi::xml_node& xmlEntity, Entity* cParent);
      void setupEntity(const pugi::xml_node& xmlEntity, Entity* cEntity);
      void addMesh(const pugi::xml_node& xmlMesh, Entity* cParent);

      Entity* addEntity(std::istream& Stream, Entity* cParent);

   public:
      Scene(const Core::ObjectName& Name);
      ~Scene();

      static void loadPrefabData();
      static void unloadPrefabData();

      static void initStaticScenes();
      static void releaseStaticScenes();

      static void setActiveScene(Scene* S);
      static Scene* getActiveScene();

      static Scene* getPermanentScene();
      static Scene* getDebuggingScene();

      static void saveEntity(std::ostream& pStream, Entity* pEntity);

      Entity* addEntity(const Core::ObjectName& Name, Entity* cParent = 0);
      Entity* getEntity(const Core::ObjectName& FullName);
      bool removeEntity(const Core::ObjectName& FullName);
      bool removeEntityImmediately(const Core::ObjectName& FullName);

      bool renameEntity(Entity* cEntity, const Core::ObjectName& NewName);
      Entity* cloneEntity(Entity* cEntity, const Core::ObjectName& CloneName, Entity* cCloneParent);
      void setEntityParent(Entity* cEntity, Entity* cNewParent);
      void setupEntity(std::istream& Stream, Entity* cEntity);

      uint getEntitiesCount() const;
      Entity* getEntityByIndex(uint Index) const;

      template<typename T>
      void registerComponent(Component* cComponent)
      {
         GEAssert(cComponent);
         GEMutexLock(mSceneMutex);
         vComponents[(uint)T::getType()].push_back(cComponent);
         GEMutexUnlock(mSceneMutex);
      }

      void registerComponent(ComponentType pType, Component* pComponent);
      void removeComponent(ComponentType pType, Component* pComponent);
      void bringComponentToFront(ComponentType pType, Component* pComponent);
      void sendComponentToBack(ComponentType pType, Component* pComponent);

      template<typename T>
      const GESTLVector(Component*)& getComponents()
      {
         return vComponents[(uint)T::getType()];
      }

      Entity* addPrefab(const char* PrefabName, const Core::ObjectName& EntityName, Entity* cParent = 0);
      void setupEntityFromPrefab(Entity* pEntity, const char* pPrefabName, bool pIncludeRootTransform = true);

      SceneBackgroundMode getBackgroundMode() const;
      const char* getBackgroundMaterialName() const;
      Color getAmbientLightColor() const;
      float getShadowsMaxDistance() const;

      void setBackgroundMode(SceneBackgroundMode BackgroundMode);
      void setBackgroundMaterialName(const char* MaterialName);
      void setAmbientLightColor(const Color& AmbientLightColor);
      void setShadowsMaxDistance(float Distance);

      void setupBackground();
      void setupSkyBox(const char* MaterialName);

      bool isRemovingEntities() const { return mRemovingEntities; }

      void queueUpdateJobs();
      void update();
      void queueForRendering();

      void load(const char* FileName);
   };
}}
