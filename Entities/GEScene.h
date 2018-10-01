
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
#include "GEComponentType.h"
#include "Externals/pugixml/pugixml.hpp"

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

      static Scene cPermanentScene;
      static Scene cDebuggingScene;

      GESTLVector(Entity*) vEntities;
      GESTLMap(uint, Entity*) mRegistry;
      GESTLVector(Component*) vComponents[(uint)ComponentType::Count];

      GESTLVector(Entity*) vEntitiesToRemove;

      SceneBackgroundMode eBackgroundMode;
      Core::ObjectName cBackgroundMaterialName;
      Entity* cBackgroundEntity;

      float fShadowsMaxDistance;

      GEMutex mSceneMutex;

      void registerEntity(Entity* cEntity);
      void removeEntity(Entity* cEntity);
      void removeEntityRecursively(Entity* cEntity);

      Entity* addEntity(const pugi::xml_node& xmlEntity, Entity* cParent);
      void setupEntity(const pugi::xml_node& xmlEntity, Entity* cEntity);
      void addMesh(const pugi::xml_node& xmlMesh, Entity* cParent);

      Entity* addEntity(std::istream& Stream, Entity* cParent);
      void setupEntity(std::istream& Stream, Entity* cEntity);

   public:
      Scene(const Core::ObjectName& Name);
      ~Scene();

      static void setActiveScene(Scene* S);
      static Scene* getActiveScene();

      static Scene* getPermanentScene();
      static Scene* getDebuggingScene();

      Entity* addEntity(const Core::ObjectName& Name, Entity* cParent = 0);
      Entity* getEntity(const Core::ObjectName& FullName);
      bool removeEntity(const Core::ObjectName& FullName);

      bool renameEntity(Entity* cEntity, const Core::ObjectName& NewName);
      Entity* cloneEntity(Entity* cEntity, const Core::ObjectName& CloneName, Entity* cCloneParent);
      void setEntityParent(Entity* cEntity, Entity* cNewParent);

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

      void registerComponent(ComponentType eType, Component* cComponent);
      void removeComponent(ComponentType eType, Component* cComponent);

      template<typename T>
      const GESTLVector(Component*)& getComponents()
      {
         return vComponents[(uint)T::getType()];
      }

      Entity* addPrefab(const char* PrefabName, const Core::ObjectName& EntityName, Entity* cParent = 0);
      void setupEntityFromPrefab(Entity* cEntity, const char* PrefabName);

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

      void queueUpdateJobs();
      void update();
      void queueForRendering();

      void load(const char* FileName);
      void loadModel(Entity* cEntity, const char* FileName);
   };
}}
