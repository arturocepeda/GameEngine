
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Entities
//
//  --- GEScene.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEScene.h"
#include "Entities/GEEntity.h"
#include "Entities/GEComponentRenderable.h"
#include "Entities/GEComponentUIElement.h"
#include "Entities/GEComponentParticleSystem.h"
#include "Entities/GEComponentSkeleton.h"
#include "Entities/GEComponentCollider.h"
#include "Entities/GEComponentAudio.h"
#include "Entities/GEComponentScript.h"
#include "Rendering/GERenderSystem.h"
#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Core/GETaskManager.h"
#include "Core/GEGeometry.h"
#include "Core/GEProfiler.h"
#include "Core/GEPlatform.h"
#include "Core/GEApplication.h"
#include "Core/GEEvents.h"

#include <algorithm>

#define GE_SCENE_JOBIFIED_UPDATE
//#define GE_SCRIPT_INSTANCE_JOBIFIED_UPDATE

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Rendering;

const ObjectName Name = ObjectName("Name");
const ObjectName BackgroundEntityName = ObjectName("Background");

//
//  Scene
//
Scene* Scene::cActiveScene = nullptr;
Scene* Scene::cPermanentScene = nullptr;
Scene* Scene::cDebuggingScene = nullptr;

GESTLMap(uint32_t, ContentData) Scene::smPrefabData;

Scene::Scene(const ObjectName& Name)
   : EventHandlingObject(Name)
   , Serializable("Scene")
   , mRemovingEntities(false)
   , eBackgroundMode(SceneBackgroundMode::SolidColor)
   , cBackgroundEntity(0)
   , fShadowsMaxDistance(20.0f)
{
   GEMutexInit(mSceneMutex);

   GERegisterPropertyEnum(SceneBackgroundMode, BackgroundMode);
   GERegisterProperty(String, BackgroundMaterialName);
   GERegisterProperty(Color, AmbientLightColor);
   GERegisterProperty(Float, ShadowsMaxDistance);
}

Scene::~Scene()
{
   if(cActiveScene == this)
      setActiveScene(0);

   for(GESTLVector(Entity*)::iterator it = vEntities.begin(); it != vEntities.end(); it++)
   {
      GEInvokeDtor(Entity, (*it));
      Allocator::free(*it);
   }

   GEMutexDestroy(mSceneMutex);
}

void Scene::loadPrefabData()
{
   if(Application::ContentType == ApplicationContentType::Bin)
   {
      FileNamesList prefabNames;
      Device::getContentFileNames("Prefabs", "prefab.ge", &prefabNames);

      for(size_t i = 0u; i < prefabNames.size(); i++)
      {
         uint32_t prefabNameHash = 0u;

         if(Device::ContentHashPath)
         {
            prefabNameHash = (uint32_t)strtoul(prefabNames[i].c_str(), nullptr, 16);
         }
         else
         {
            prefabNameHash = hash(prefabNames[i].c_str());
         }

         smPrefabData[prefabNameHash] = ContentData();
         ContentData& contentData = smPrefabData.find(prefabNameHash)->second;
         Device::readContentFile(
            ContentType::GenericBinaryData, "Prefabs", prefabNames[i].c_str(), "prefab.ge", &contentData);
      }
   }
}

void Scene::unloadPrefabData()
{
   for(GESTLMap(uint32_t, ContentData)::iterator it = smPrefabData.begin(); it != smPrefabData.end(); it++)
   {
      it->second.unload();
   }

   smPrefabData.clear();
}

void Scene::initStaticScenes()
{
   cPermanentScene = Allocator::alloc<Scene>();
   GEInvokeCtor(Scene, cPermanentScene)("Permanent");

   cDebuggingScene = Allocator::alloc<Scene>();
   GEInvokeCtor(Scene, cDebuggingScene)("Debugging");
}

void Scene::releaseStaticScenes()
{
   GEInvokeDtor(Scene, cDebuggingScene);
   Allocator::free(cDebuggingScene);
   cDebuggingScene = 0;

   GEInvokeDtor(Scene, cPermanentScene);
   Allocator::free(cPermanentScene);
   cPermanentScene = 0;
}

void Scene::saveEntityContents(std::ostream& pStream, Entity* pEntity)
{
   uint8_t componentsCount = 0u;

   for(uint32_t i = 0u; i < (uint32_t)ComponentType::Count; i++)
   {
      Component* component = pEntity->getComponent((ComponentType)i);

      if(component)
      {
         componentsCount++;
      }
   }

   const Value componentsCountValue(componentsCount);
   componentsCountValue.writeToStream(pStream);

   for(uint32_t i = 0u; i < (uint32_t)ComponentType::Count; i++)
   {
      Component* component = pEntity->getComponent((ComponentType)i);

      if(component)
      {
         const Value componentClassNameValue(component->getClassName().getID());
         componentClassNameValue.writeToStream(pStream);
         component->saveToStream(pStream);
      }
   }

   const Value childrenCountValue((uint8_t)pEntity->getChildrenCount());
   childrenCountValue.writeToStream(pStream);

   for(uint32_t i = 0u; i < pEntity->getChildrenCount(); i++)
   {
      saveEntity(pStream, pEntity->getChildByIndex(i));
   }
}

void Scene::registerEntity(Entity* cEntity)
{
   GEMutexLock(mSceneMutex);

   Entity* cParent = cEntity->getParent();

   if(cParent)
   {
      cParent->addChild(cEntity);
   }

   vEntities.push_back(cEntity);
   mRegistry[cEntity->getFullName().getID()] = cEntity;

   GEMutexUnlock(mSceneMutex);
}

void Scene::removeEntity(Entity* cEntity)
{
   // remove the attachment to the parent
   Entity* cParent = cEntity->getParent();

   if(cParent)
   {
      cParent->vChildren.erase(std::find(cParent->vChildren.begin(), cParent->vChildren.end(), cEntity));
   }

   // remove the entity and all its children
   removeEntityRecursively(cEntity);
}

void Scene::removeEntityRecursively(Entity* cEntity)
{
   // notify the removal
   EventArgs sEventArgs;
   sEventArgs.Sender = this;
   sEventArgs.Data = cEntity;
   triggerEvent(Events::EntityRemoved, &sEventArgs);

   // remove the entity from the registry
   GESTLMap(uint, Entity*)::iterator it = mRegistry.find(cEntity->getFullName().getID());
   mRegistry.erase(it);

   // remove the entity from the list of entities
   vEntities.erase(std::find(vEntities.begin(), vEntities.end(), cEntity));

   // remove the components from the lists of component
   if(cEntity->bInitialized)
   {
      for(uint i = 0; i < (uint)ComponentType::Count; i++)
      {
         Component* cComponent = cEntity->getComponent((ComponentType)i);

         if(cComponent)
         {
            vComponents[i].erase(std::find(vComponents[i].begin(), vComponents[i].end(), cComponent));
         }
      }
   }

   // remove the children
   for(uint i = 0; i < cEntity->getChildrenCount(); i++)
   {
      removeEntityRecursively(cEntity->getChildByIndex(i));
   }

   // delete the entity
   GEInvokeDtor(Entity, cEntity);
   Allocator::free(cEntity);
}

void Scene::setActiveScene(Scene* S)
{
   cActiveScene = S;
   triggerEventStatic(Events::ActiveSceneSet);
   RenderSystem::getInstance()->clearGeometryRenderInfoEntries();
}

Scene* Scene::getActiveScene()
{
   return cActiveScene;
}

Scene* Scene::getPermanentScene()
{
   return cPermanentScene;
}

Scene* Scene::getDebuggingScene()
{
   return cDebuggingScene;
}

void Scene::saveEntity(std::ostream& pStream, Entity* pEntity)
{
   const Value entityNameValue(pEntity->getName());
   entityNameValue.writeToStream(pStream);

   pEntity->saveToStream(pStream);

   saveEntityContents(pStream, pEntity);
}

Entity* Scene::addEntity(const ObjectName& Name, Entity* cParent)
{
   Entity* cEntity = Allocator::alloc<Entity>();
   GEInvokeCtor(Entity, cEntity)(Name, cParent, this);

   GEAssert(!getEntity(cEntity->getFullName()));
   registerEntity(cEntity);

   EventArgs sEventArgs;
   sEventArgs.Sender = this;
   sEventArgs.Data = cEntity;
   triggerEvent(Events::EntityAdded, &sEventArgs);

   return cEntity;
}

Entity* Scene::getEntity(const ObjectName& FullName)
{
   GEMutexLock(mSceneMutex);

   Entity* cEntity = 0;
   GESTLMap(uint, Entity*)::iterator it = mRegistry.find(FullName.getID());

   if(it != mRegistry.end())
      cEntity = it->second;

   GEMutexUnlock(mSceneMutex);

   return cEntity;
}

bool Scene::removeEntity(const ObjectName& FullName)
{
   GEMutexLock(mSceneMutex);

   GESTLMap(uint, Entity*)::iterator it = mRegistry.find(FullName.getID());

   if(it == mRegistry.end())
   {
      GEMutexUnlock(mSceneMutex);
      return false;
   }

   mEntitiesToRemove.push_back(it->second);

   GEMutexUnlock(mSceneMutex);

   return true;
}

bool Scene::removeEntityImmediately(const Core::ObjectName& FullName)
{
   GEMutexLock(mSceneMutex);

   GESTLMap(uint, Entity*)::iterator it = mRegistry.find(FullName.getID());

   if(it == mRegistry.end())
   {
      GEMutexUnlock(mSceneMutex);
      return false;
   }

   removeEntity(it->second);

   GEMutexUnlock(mSceneMutex);

   return true;
}

bool Scene::renameEntity(Entity* cEntity, const Core::ObjectName& NewName)
{
   ObjectName cOriginalName = cEntity->getName();
   ObjectName cOriginalFullName = cEntity->getFullName();

   GEMutexLock(mSceneMutex);

   cEntity->cName = NewName;
   cEntity->updateFullName();
   
   bool bNewNameIsUnique = mRegistry.find(cEntity->getFullName().getID()) == mRegistry.end();
   
   if(bNewNameIsUnique)
   {
      GESTLMap(uint, Entity*)::const_iterator it = mRegistry.find(cOriginalFullName.getID());
      mRegistry.erase(it);
      mRegistry[cEntity->cFullName.getID()] = cEntity;

      EventArgs sEventArgs;
      sEventArgs.Sender = this;
      sEventArgs.Data = cEntity;
      triggerEvent(Events::EntityRenamed, &sEventArgs);

      GEMutexUnlock(mSceneMutex);

      for(uint i = 0; i < cEntity->getChildrenCount(); i++)
      {
         Entity* cChild = cEntity->getChildByIndex(i);
         renameEntity(cChild, cChild->getName());
      }
   }
   else
   {
      cEntity->cName = cOriginalName;
      cEntity->cFullName = cOriginalFullName;

      GEMutexUnlock(mSceneMutex);
   }

   return bNewNameIsUnique;
}

Entity* Scene::cloneEntity(Entity* cEntity, const Core::ObjectName& CloneName, Entity* cCloneParent)
{
   Entity* cCloneEntity = addEntity(CloneName, cCloneParent);

   if(!cEntity->getPrefabName().isEmpty())
      cCloneEntity->setPrefabName(cEntity->getPrefabName());

   for(uint i = 0; i < cEntity->getPropertiesCount(); i++)
   {
      const Property& sSourceProperty = cEntity->getProperty(i);

      if(!sSourceProperty.Setter || sSourceProperty.Name == Name)
         continue;

      const Property& sTargetProperty = cCloneEntity->getProperty(i);
      Value cSourcePropertyValue = sSourceProperty.Getter();
      sTargetProperty.Setter(cSourcePropertyValue);
   }

   for(uint i = 0; i < (uint)ComponentType::Count; i++)
   {
      Component* cComponent = cEntity->getComponent((ComponentType)i);

      if(cComponent)
      {
         Component* cCloneComponent = cCloneEntity->addComponent(cComponent->getClassName());
         cCloneComponent->copy(cComponent);
      }
   }

   for(uint i = 0; i < cEntity->getChildrenCount(); i++)
   {
      Entity* cEntityChild = cEntity->getChildByIndex(i);

      if(!GEHasFlag(cEntityChild->getInternalFlags(), Entity::InternalFlags::Generated))
      {
         cloneEntity(cEntity->getChildByIndex(i), cEntityChild->getName(), cCloneEntity);
      }
   }

   return cCloneEntity;
}

void Scene::setEntityParent(Entity* cEntity, Entity* cNewParent)
{
   GEMutexLock(mSceneMutex);

   // remove the current entry in the registry
   GESTLMap(uint, Entity*)::const_iterator it = mRegistry.find(cEntity->getFullName().getID());
   GEAssert(it != mRegistry.end());
   mRegistry.erase(it);

   // remove this entity from the parent's children list
   if(cEntity->cParent)
   {
      GESTLVector(Entity*)::iterator it = cEntity->cParent->vChildren.begin();

      for(; it != cEntity->cParent->vChildren.end(); it++)
      {
         if(*it == cEntity)
         {
            cEntity->cParent->vChildren.erase(it);
            break;
         }
      }
   }

   // set the new parent
   cEntity->cParent = cNewParent;
   cEntity->updateFullName();

   if(cNewParent)
   {
      cNewParent->vChildren.push_back(cEntity);
   }

   mRegistry[cEntity->getFullName().getID()] = cEntity;

   // update the local transform matrix
   ComponentTransform* cTransform = cEntity->getComponent<ComponentTransform>();

   if(cNewParent)
   {
      Matrix4 mNewParentInverseWorldMatrix = cNewParent->getComponent<ComponentTransform>()->mGlobalWorldMatrix;
      Matrix4Invert(&mNewParentInverseWorldMatrix);
      Matrix4Multiply(mNewParentInverseWorldMatrix, cTransform->mGlobalWorldMatrix, &cTransform->mLocalWorldMatrix);
   }
   else
   {
      cTransform->mLocalWorldMatrix = cTransform->mGlobalWorldMatrix;
   }

   Vector3 vLocalPosition;
   Rotation cLocalRotation;
   Vector3 vLocalScale;
   Geometry::extractTRSFromMatrix(cTransform->mLocalWorldMatrix, &vLocalPosition, &cLocalRotation, &vLocalScale);

   cTransform->setPosition(vLocalPosition);
   cTransform->setRotation(cLocalRotation);
   cTransform->setScale(vLocalScale);

   GEMutexUnlock(mSceneMutex);

   EventArgs sEventArgs;
   sEventArgs.Sender = this;
   sEventArgs.Data = cEntity;
   triggerEvent(Events::EntityParentChanged, &sEventArgs);
}

void Scene::setupEntity(std::istream& Stream, Entity* cEntity)
{
   cEntity->loadFromStream(Stream);

   uint iComponentsCount = (uint)Value::fromStream(ValueType::Byte, Stream).getAsByte();

   for(uint i = 0; i < iComponentsCount; i++)
   {
      ObjectName cComponentTypeName = ObjectName(Value::fromStream(ValueType::UInt, Stream).getAsUInt());
      Component* cComponent = cEntity->getOrAddComponent(cComponentTypeName);
      cComponent->loadFromStream(Stream);
   }

   uint iChildrenCount = (uint)Value::fromStream(ValueType::Byte, Stream).getAsByte();

   for(uint i = 0; i < iChildrenCount; i++)
   {
      addEntity(Stream, cEntity);
   }
}

uint Scene::getEntitiesCount() const
{
   return (uint)vEntities.size();
}

Entity* Scene::getEntityByIndex(uint Index) const
{
   GEAssert(Index < vEntities.size());
   return vEntities[Index];
}

void Scene::registerComponent(ComponentType pType, Component* pComponent)
{
   GEAssert(pType < ComponentType::Count);
   GEAssert(pComponent);
   GEMutexLock(mSceneMutex);
   vComponents[(uint32_t)pType].push_back(pComponent);
   GEMutexUnlock(mSceneMutex);
}

void Scene::removeComponent(ComponentType pType, Component* pComponent)
{
   GEAssert(pType < ComponentType::Count);
   GEAssert(pComponent);
   GEMutexLock(mSceneMutex);

   GESTLVector(Component*)& vComponentList = vComponents[(uint32_t)pType];

   for(uint32_t i = 0u; i < vComponentList.size(); i++)
   {
      if(vComponentList[i] == pComponent)
      {
         vComponentList.erase(vComponentList.begin() + i);
         break;
      }
   }

   GEMutexUnlock(mSceneMutex);
}

void Scene::bringComponentToFront(ComponentType pType, Component* pComponent)
{
   GEAssert(pType < ComponentType::Count);
   GEAssert(pComponent);
   GEMutexLock(mSceneMutex);

   GESTLVector(Component*)& vComponentList = vComponents[(uint)pType];

   for(uint32_t i = 0u; i < vComponentList.size(); i++)
   {
      if(vComponentList[i] == pComponent)
      {
         vComponentList.erase(vComponentList.begin() + i);
         vComponentList.insert(vComponentList.begin(), pComponent);
         break;
      }
   }

   GEMutexUnlock(mSceneMutex);
}

void Scene::sendComponentToBack(ComponentType pType, Component* pComponent)
{
   GEAssert(pType < ComponentType::Count);
   GEAssert(pComponent);
   GEMutexLock(mSceneMutex);

   GESTLVector(Component*)& vComponentList = vComponents[(uint)pType];

   for(uint32_t i = 0u; i < vComponentList.size(); i++)
   {
      if(vComponentList[i] == pComponent)
      {
         vComponentList.erase(vComponentList.begin() + i);
         vComponentList.push_back(pComponent);
         break;
      }
   }

   GEMutexUnlock(mSceneMutex);
}

const GESTLVector(Component*)& Scene::getComponents(ComponentType pType)
{
   GEAssert(pType < ComponentType::Count);
   return vComponents[(uint)pType];
}

Entity* Scene::addPrefab(const char* PrefabName, const ObjectName& EntityName, Entity* cParent)
{
   Entity* cEntity = addEntity(EntityName, cParent);
   setupEntityFromPrefab(cEntity, PrefabName);
   cEntity->init();

   return cEntity;
}

void Scene::setupEntityFromPrefab(Entity* pEntity, const char* pPrefabName, bool pIncludeRootTransform)
{
   Vector3 cachedPosition;
   Rotation cachedRotation;
   Vector3 cachedScale;

   if(!pIncludeRootTransform)
   {
      cachedPosition = pEntity->getComponent<ComponentTransform>()->getPosition();
      cachedRotation = pEntity->getComponent<ComponentTransform>()->getRotation();
      cachedScale = pEntity->getComponent<ComponentTransform>()->getScale();
   }

   char sFilename[64];
   sprintf(sFilename, "%s.prefab", pPrefabName);

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      ContentData content;
      Device::readContentFile(ContentType::GenericTextData, "Prefabs", sFilename, "xml", &content);
      pugi::xml_document xml;
      xml.load_buffer(content.getData(), content.getDataSize());
      pugi::xml_node xmlRoot = xml.child("Prefab");

      pugi::xml_attribute xmlPrefabBase = xmlRoot.attribute("base");

      while(!xmlPrefabBase.empty())
      {
         pugi::xml_document xmlDerived;
         xmlDerived.append_copy(xmlRoot);
         pugi::xml_node xmlDerivedRoot = xmlDerived.child("Prefab");

         const char* prefabBaseName = xmlPrefabBase.value();
         sprintf(sFilename, "%s.prefab", prefabBaseName);

         xml.remove_child(xmlRoot);

         Device::readContentFile(ContentType::GenericTextData, "Prefabs", sFilename, "xml", &content);
         xml.load_buffer(content.getData(), content.getDataSize());
         xmlRoot = xml.child("Prefab");

         Entity::mergeXmlDescription(xmlRoot, xmlDerivedRoot);

         xmlPrefabBase = xmlRoot.attribute("base");
      }

      setupEntity(xmlRoot, pEntity);
   }
   else
   {
      const uint32_t prefabNameHash = hash(pPrefabName);
      const ContentData& content = smPrefabData.find(prefabNameHash)->second;

      ContentDataMemoryBuffer memoryBuffer(content);
      std::istream stream(&memoryBuffer);
      Value::fromStream(ValueType::ObjectName, stream);

      setupEntity(stream, pEntity);
   }

   if(!pIncludeRootTransform)
   {
      pEntity->getComponent<ComponentTransform>()->setPosition(cachedPosition);
      pEntity->getComponent<ComponentTransform>()->setRotation(cachedRotation);
      pEntity->getComponent<ComponentTransform>()->setScale(cachedScale);
   }

   pEntity->setPrefabName(ObjectName(pPrefabName));
}

SceneBackgroundMode Scene::getBackgroundMode() const
{
   return eBackgroundMode;
}

const char* Scene::getBackgroundMaterialName() const
{
   return cBackgroundMaterialName.getString();
}

Color Scene::getAmbientLightColor() const
{
   return RenderSystem::getInstance()
      ? RenderSystem::getInstance()->getAmbientLightColor()
      : Color();
}

float Scene::getShadowsMaxDistance() const
{
   return fShadowsMaxDistance;
}

void Scene::setBackgroundMode(SceneBackgroundMode BackgroundMode)
{
   eBackgroundMode = BackgroundMode;
   setupBackground();
}

void Scene::setBackgroundMaterialName(const char* MaterialName)
{
   cBackgroundMaterialName = ObjectName(MaterialName);
   setupBackground();
}

void Scene::setAmbientLightColor(const Color& AmbientLightColor)
{
   RenderSystem::getInstance()->setAmbientLightColor(AmbientLightColor);
}

void Scene::setShadowsMaxDistance(float Distance)
{
   fShadowsMaxDistance = Distance;
}

void Scene::setupBackground()
{
   switch(eBackgroundMode)
   {
   case SceneBackgroundMode::SolidColor:
      {
         if(cBackgroundEntity)
         {
            removeEntity(cBackgroundEntity->getFullName());
            cBackgroundEntity = 0;
         }
      }
      break;

   case SceneBackgroundMode::SkyBox:
      {
         if(cBackgroundMaterialName.getID() != 0)
            setupSkyBox(cBackgroundMaterialName.getString());
      }
      break;

   default:
      break;
   }
}

void Scene::setupSkyBox(const char* MaterialName)
{
   if(cBackgroundEntity)
      removeEntity(cBackgroundEntity->getFullName());

   const ObjectName SkyBoxPlaneUpEntityName = ObjectName("Background/PlaneUp");
   const ObjectName SkyBoxPlaneDownEntityName = ObjectName("Background/PlaneDown");
   const ObjectName SkyBoxPlaneLeftEntityName = ObjectName("Background/PlaneLeft");
   const ObjectName SkyBoxPlaneRightEntityName = ObjectName("Background/PlaneRight");
   const ObjectName SkyBoxPlaneFrontEntityName = ObjectName("Background/PlaneFront");
   const ObjectName SkyBoxPlaneBackEntityName = ObjectName("Background/PlaneBack");

   cBackgroundEntity = addEntity(BackgroundEntityName);
   cBackgroundEntity->setInternalFlags((uint8_t)Entity::InternalFlags::Generated);
   cBackgroundEntity->addComponent<ComponentTransform>();

   ComponentCamera* cCamera = RenderSystem::getInstance()->getActiveCamera();
   const float PlaneDistance = (cCamera ? cCamera->getFarZ() : ComponentCamera::DefaultFarZ) * 0.5f;
   const Vector3 PlaneScale = Vector3(PlaneDistance * 2.0f, 0.0f, PlaneDistance * 2.0f);
   ObjectName cQuadName = ObjectName("Quad");

   Entity* cSkyBoxPlaneUp = addEntity(SkyBoxPlaneUpEntityName, cBackgroundEntity);
   ComponentTransform* cCubeMapPlaneTransform = cSkyBoxPlaneUp->addComponent<ComponentTransform>();
   cCubeMapPlaneTransform->setPosition(Vector3(0.0f, PlaneDistance, 0.0f));
   cCubeMapPlaneTransform->setOrientation(Vector3(180.0f, 0.0f, 0.0f));
   cCubeMapPlaneTransform->setScale(PlaneScale);
   ComponentMesh* cCubeMapPlane = cSkyBoxPlaneUp->addComponent<ComponentMesh>();
   cCubeMapPlane->setMeshName(cQuadName);

   Entity* cSkyBoxPlaneDown = addEntity(SkyBoxPlaneDownEntityName, cBackgroundEntity);
   cCubeMapPlaneTransform = cSkyBoxPlaneDown->addComponent<ComponentTransform>();
   cCubeMapPlaneTransform->setPosition(Vector3(0.0f, -PlaneDistance, 0.0f));
   cCubeMapPlaneTransform->setOrientation(Vector3(0.0f, 0.0f, 0.0f));
   cCubeMapPlaneTransform->setScale(PlaneScale);
   cCubeMapPlane = cSkyBoxPlaneDown->addComponent<ComponentMesh>();
   cCubeMapPlane->setMeshName(cQuadName);

   Entity* cSkyBoxPlaneLeft = addEntity(SkyBoxPlaneLeftEntityName, cBackgroundEntity);
   cCubeMapPlaneTransform = cSkyBoxPlaneLeft->addComponent<ComponentTransform>();
   cCubeMapPlaneTransform->setPosition(Vector3(-PlaneDistance, 0.0f, 0.0f));
   cCubeMapPlaneTransform->setOrientation(Vector3(0.0f, -90.0f, 90.0f));
   cCubeMapPlaneTransform->setScale(PlaneScale);
   cCubeMapPlane = cSkyBoxPlaneLeft->addComponent<ComponentMesh>();
   cCubeMapPlane->setMeshName(cQuadName);

   Entity* cSkyBoxPlaneRight = addEntity(SkyBoxPlaneRightEntityName, cBackgroundEntity);
   cCubeMapPlaneTransform = cSkyBoxPlaneRight->addComponent<ComponentTransform>();
   cCubeMapPlaneTransform->setPosition(Vector3(PlaneDistance, 0.0f, 0.0f));
   cCubeMapPlaneTransform->setOrientation(Vector3(0.0f, 90.0f, -90.0f));
   cCubeMapPlaneTransform->setScale(PlaneScale);
   cCubeMapPlane = cSkyBoxPlaneRight->addComponent<ComponentMesh>();
   cCubeMapPlane->setMeshName(cQuadName);

   Entity* cSkyBoxPlaneFront = addEntity(SkyBoxPlaneFrontEntityName, cBackgroundEntity);
   cCubeMapPlaneTransform = cSkyBoxPlaneFront->addComponent<ComponentTransform>();
   cCubeMapPlaneTransform->setPosition(Vector3(0.0f, 0.0f, PlaneDistance));
   cCubeMapPlaneTransform->setOrientation(Vector3(90.0f, 0.0f, 180.0f));
   cCubeMapPlaneTransform->setScale(PlaneScale);
   cCubeMapPlane = cSkyBoxPlaneFront->addComponent<ComponentMesh>();
   cCubeMapPlane->setMeshName(cQuadName);

   Entity* cSkyBoxPlaneBack = addEntity(SkyBoxPlaneBackEntityName, cBackgroundEntity);
   cCubeMapPlaneTransform = cSkyBoxPlaneBack->addComponent<ComponentTransform>();
   cCubeMapPlaneTransform->setPosition(Vector3(0.0f, 0.0f, -PlaneDistance));
   cCubeMapPlaneTransform->setOrientation(Vector3(-90.0f, 0.0f, 0.0f));
   cCubeMapPlaneTransform->setScale(PlaneScale);
   cCubeMapPlane = cSkyBoxPlaneBack->addComponent<ComponentMesh>();
   cCubeMapPlane->setMeshName(cQuadName);

   char sMaterialNameBuffer[64];
   sprintf(sMaterialNameBuffer, "%s_u", MaterialName);
   cSkyBoxPlaneUp->getComponent<ComponentMesh>()->addMaterialPass()->setMaterialName(sMaterialNameBuffer);
   sprintf(sMaterialNameBuffer, "%s_d", MaterialName);
   cSkyBoxPlaneDown->getComponent<ComponentMesh>()->addMaterialPass()->setMaterialName(sMaterialNameBuffer);
   sprintf(sMaterialNameBuffer, "%s_l", MaterialName);
   cSkyBoxPlaneLeft->getComponent<ComponentMesh>()->addMaterialPass()->setMaterialName(sMaterialNameBuffer);
   sprintf(sMaterialNameBuffer, "%s_r", MaterialName);
   cSkyBoxPlaneRight->getComponent<ComponentMesh>()->addMaterialPass()->setMaterialName(sMaterialNameBuffer);
   sprintf(sMaterialNameBuffer, "%s_f", MaterialName);
   cSkyBoxPlaneFront->getComponent<ComponentMesh>()->addMaterialPass()->setMaterialName(sMaterialNameBuffer);
   sprintf(sMaterialNameBuffer, "%s_b", MaterialName);
   cSkyBoxPlaneBack->getComponent<ComponentMesh>()->addMaterialPass()->setMaterialName(sMaterialNameBuffer);

   cBackgroundEntity->init();
}

void Scene::queueUpdateJobs()
{
   GEProfilerMarker("Scene::queueUpdateJobs()");

   GEMutexLock(mSceneMutex);

   if(!mEntitiesToRemove.empty())
   {
      mRemovingEntities = true;

      for(uint i = 0; i < mEntitiesToRemove.size(); i++)
         removeEntity(mEntitiesToRemove[i]);

      mEntitiesToRemove.clear();

      mRemovingEntities = false;
   }

   GEMutexUnlock(mSceneMutex);

   // skeletons
   GESTLVector(Component*)& vSkeletons = vComponents[(uint)ComponentType::Skeleton];

   for(uint i = 0; i < vSkeletons.size(); i++)
   {
      ComponentSkeleton* cSkeleton = static_cast<ComponentSkeleton*>(vSkeletons[i]);

      if(cSkeleton->getOwner()->isActiveInHierarchy())
      {
#if defined (GE_SCENE_JOBIFIED_UPDATE)
         JobDesc sJobDesc("UpdateSkeleton");
         sJobDesc.Task = [cSkeleton] { cSkeleton->update(); };
         TaskManager::getInstance()->queueJob(sJobDesc, JobType::Frame);
#else
         cSkeleton->update();
#endif
      }
   }

   // particle systems
   GESTLVector(Component*)& vRenderables = vComponents[(uint)ComponentType::Renderable];

   for(uint i = 0; i < vRenderables.size(); i++)
   {
      ComponentRenderable* cRenderable = static_cast<ComponentRenderable*>(vRenderables[i]);

      if(cRenderable->getVisible() &&
         cRenderable->getClassName() == ComponentParticleSystem::ClassName &&
         cRenderable->getOwner()->isActiveInHierarchy())
      {
         ComponentParticleSystem* cParticleSystem = static_cast<ComponentParticleSystem*>(cRenderable);
#if defined (GE_SCENE_JOBIFIED_UPDATE)
         JobDesc sJobDesc("UpdateParticleSystem");
         sJobDesc.Task = [cParticleSystem] { cParticleSystem->update(); };
         TaskManager::getInstance()->queueJob(sJobDesc, JobType::Frame);
#else
         cParticleSystem->update();
#endif
      }
   }

   // audio components
   GESTLVector(Component*)& vAudioComponents = vComponents[(uint)ComponentType::Audio];

   for(uint i = 0; i < vAudioComponents.size(); i++)
   {
      ComponentAudio* cAudioComponent = static_cast<ComponentAudio*>(vAudioComponents[i]);

      if(cAudioComponent->getOwner()->isActiveInHierarchy())
      {
#if defined (GE_SCENE_JOBIFIED_UPDATE)
         JobDesc sJobDesc("UpdateAudioComponent");
         sJobDesc.Task = [cAudioComponent] { cAudioComponent->update(); };
         TaskManager::getInstance()->queueJob(sJobDesc, JobType::Frame);
#else
         cAudioComponent->update();
#endif
      }
   }

#if defined (GE_SCENE_JOBIFIED_UPDATE) && defined (GE_SCRIPT_INSTANCE_JOBIFIED_UPDATE)
   // thread-safe script instances
   for(uint32_t jobIndex = 1u; jobIndex < Application::ScriptingEnvironmentsCount; jobIndex++)
   {
      JobDesc sJobDesc("UpdateScriptInstance");
      sJobDesc.Task = [this, jobIndex]
      {
         GESTLVector(Component*)& vScripts = vComponents[(uint)ComponentType::Script];

         for(uint i = 0; i < vScripts.size(); i++)
         {
            ComponentScript* cScript = static_cast<ComponentScript*>(vScripts[i]);

            if(cScript->getJobIndex() == jobIndex && cScript->getOwner()->isActiveInHierarchy())
            {
               for(uint j = 0; j < cScript->getScriptInstanceCount(); j++)
               {
                  ScriptInstance* cScriptInstance = cScript->getScriptInstance(j);

                  if(cScriptInstance->getThreadSafe())
                  {
                     cScriptInstance->update();
                  }
               }
            }
         }
      };

      TaskManager::getInstance()->queueJob(sJobDesc, JobType::Frame);
   }
#endif
}

void Scene::update()
{
   GEProfilerMarker("Scene::update()");

   // background
   if(cBackgroundEntity && RenderSystem::getInstance()->getActiveCamera())
   {
      ComponentCamera* cActiveCamera = RenderSystem::getInstance()->getActiveCamera();
      const Vector3& vActiveCameraPosition = cActiveCamera->getTransform()->getPosition();
      cBackgroundEntity->getComponent<ComponentTransform>()->setPosition(vActiveCameraPosition);
   }

   // lights
   GESTLVector(Component*)& vLights = vComponents[(uint)ComponentType::Light];

   for(uint i = 0; i < vLights.size(); i++)
   {
      RenderSystem::getInstance()->queueForRendering(static_cast<ComponentLight*>(vLights[i]));
   }

   // non thread-safe script instances
   GESTLVector(Component*)& vScripts = vComponents[(uint)ComponentType::Script];

   for(uint i = 0; i < vScripts.size(); i++)
   {
      ComponentScript* cScript = static_cast<ComponentScript*>(vScripts[i]);

      if(cScript->getOwner()->isActiveInHierarchy())
      {
         for(uint j = 0; j < cScript->getScriptInstanceCount(); j++)
         {
            ScriptInstance* cScriptInstance = cScript->getScriptInstance(j);

#if defined (GE_SCENE_JOBIFIED_UPDATE) && defined (GE_SCRIPT_INSTANCE_JOBIFIED_UPDATE)
            if(!cScriptInstance->getThreadSafe())
#endif
            {
               cScriptInstance->update();
            }
         }
      }
   }

   // sprites
   GESTLVector(Component*)& vRenderables = vComponents[(uint)ComponentType::Renderable];

   for(uint i = 0; i < vRenderables.size(); i++)
   {
      ComponentRenderable* cRenderable = static_cast<ComponentRenderable*>(vRenderables[i]);

      if(cRenderable->getClassName() == ComponentSprite::ClassName)
      {
         ComponentSprite* cSprite = static_cast<ComponentSprite*>(cRenderable);
         cSprite->update();
      }
   }

   // cameras
   GESTLVector(Component*)& vCameras = vComponents[(uint)ComponentType::Camera];

   for(uint i = 0; i < vCameras.size(); i++)
   {
      ComponentCamera* cCamera = static_cast<ComponentCamera*>(vCameras[i]);

      if(cCamera->getOwner()->isActiveInHierarchy())
      {
         cCamera->update();
      }
   }
}

void Scene::queueForRendering()
{
   GEProfilerMarker("Scene::queueForRendering()");

   GESTLVector(Component*)& uiElements = vComponents[(uint32_t)ComponentType::UIElement];

   for(size_t i = 0u; i < uiElements.size(); i++)
   {
      if(uiElements[i]->getClassName() == ComponentUI3DCanvas::ClassName)
      {
         ComponentUI3DCanvas* canvas = static_cast<ComponentUI3DCanvas*>(uiElements[i]);

         const uint32_t canvasIndex = (uint32_t)canvas->getCanvasIndex();
         const Vector3& canvasWorldPosition =
            canvas->getOwner()->getComponent<ComponentTransform>()->getWorldPosition();
         const uint16_t canvasSettings = (uint16_t)canvas->getSettings();

         RenderSystem::getInstance()->setup3DUICanvas(canvasIndex, canvasWorldPosition, canvasSettings);
      }
   }

   GESTLVector(Component*)& renderables = vComponents[(uint32_t)ComponentType::Renderable];

   for(size_t i = 0u; i < renderables.size(); i++)
   {
      ComponentRenderable* renderable = static_cast<ComponentRenderable*>(renderables[i]);
      RenderSystem::getInstance()->queueForRendering(renderable, (uint32_t)i);
   }
}

void Scene::load(const char* Name)
{
   ContentData cContent;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Scenes", Name, "scene.xml", &cContent);
      pugi::xml_document xml;
      xml.load_buffer(cContent.getData(), cContent.getDataSize());

      const pugi::xml_node& xmlRoot = xml.child("Scene");
      loadFromXml(xmlRoot);

      for(const pugi::xml_node& xmlEntity : xmlRoot.children("Entity"))
      {
         Entity* cEntity = addEntity(xmlEntity, 0);
         cEntity->init();
      }
   }
   else
   {
      Device::readContentFile(ContentType::GenericBinaryData, "Scenes", Name, "scene.ge", &cContent);
      ContentDataMemoryBuffer sMemoryBuffer(cContent);
      std::istream sStream(&sMemoryBuffer);

      loadFromStream(sStream);

      uint iRootEntitiesCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iRootEntitiesCount; i++)
      {
         Entity* cEntity = addEntity(sStream, 0);
         cEntity->init();
      }
   }
}

Entity* Scene::addEntity(const pugi::xml_node& xmlEntity, Entity* cParent)
{
   const char* sEntityName = xmlEntity.attribute("name").value();
   Entity* cEntity = 0;
   
   if(cParent)
   {
      cEntity = cParent->getChildByName(ObjectName(sEntityName));
   }

   if(!cEntity)
   {
      cEntity = addEntity(sEntityName, cParent);
   }

   setupEntity(xmlEntity, cEntity);

   cEntity->getComponent<ComponentTransform>()->updateWorldMatrix();

   return cEntity;
}

void Scene::setupEntity(const pugi::xml_node& xmlEntity, Entity* cEntity)
{
   cEntity->loadFromXml(xmlEntity);

   for(const pugi::xml_node& xmlNode : xmlEntity.children())
   {
      const char* sNodeType = xmlNode.name();

      // component
      if(strcmp(sNodeType, "Component") == 0)
      {
         const char* sComponentType = xmlNode.attribute("type").value();
         ObjectName cComponentTypeName = ObjectName(sComponentType);

         Component* cComponent = cEntity->getOrAddComponent(cComponentTypeName);
         cComponent->loadFromXml(xmlNode);
      }
      // child entity
      else if(strcmp(sNodeType, "Entity") == 0)
      {
         addEntity(xmlNode, cEntity);
      }
   }
}

void Scene::addMesh(const pugi::xml_node& xmlMesh, Entity* cEntity)
{
   ObjectName sMeshName = ObjectName(xmlMesh.attribute("name").value());
   ComponentMesh* cMesh = cEntity->addComponent<ComponentMesh>();
   cMesh->setMeshName(sMeshName);

   pugi::xml_attribute xmlMeshMaterial = xmlMesh.attribute("material");
   Material* cMaterial = RenderSystem::getInstance()->getMaterial(xmlMeshMaterial.value());
   GEAssert(cMaterial);
   cMesh->addMaterialPass()->setMaterial(cMaterial);

   for(pugi::xml_node_iterator it = xmlMesh.begin(); it != xmlMesh.end(); it++)
   {
      const pugi::xml_node& xmlMeshChild = *it;
      addMesh(xmlMeshChild, cEntity);
   }
}

Entity* Scene::addEntity(std::istream& Stream, Entity* cParent)
{
   ObjectName cEntityName = Value::fromStream(ValueType::ObjectName, Stream).getAsObjectName();
   Entity* cEntity = 0;

   if(cParent)
   {
      cEntity = cParent->getChildByName(cEntityName);
   }

   if(!cEntity)
   {
      cEntity = addEntity(cEntityName, cParent);
   }

   setupEntity(Stream, cEntity);

   cEntity->getComponent<ComponentTransform>()->updateWorldMatrix();

   return cEntity;
}
