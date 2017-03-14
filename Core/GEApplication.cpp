
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEApplication.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEApplication.h"
#include "GEAllocator.h"
#include "GETaskManager.h"

#include "Input/GEInputSystem.h"

#include "Content/GEContentManager.h"
#include "Content/GELocalizedString.h"

#include "Entities/GEComponentTransform.h"
#include "Entities/GEComponentCamera.h"
#include "Entities/GEComponentLight.h"
#include "Entities/GEComponentUIElement.h"
#include "Entities/GEComponentSprite.h"
#include "Entities/GEComponentLabel.h"
#include "Entities/GEComponentMesh.h"
#include "Entities/GEComponentParticleSystem.h"
#include "Entities/GEComponentSkeleton.h"
#include "Entities/GEComponentCollider.h"
#include "Entities/GEComponentGenericData.h"
#include "Entities/GEComponentScript.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Input;
using namespace GE::Content;
using namespace GE::Entities;

const char* Application::Name = 0;
const char* Application::ID = 0;

const char* Application::VersionString = 0;
uint Application::VersionNumber = 0;

const char* Application::ExecutablePath = 0;
GESTLVector(const char*) Application::Arguments;

ApplicationContentType Application::ContentType = ApplicationContentType::Xml;

void Application::startUp()
{
   Allocator::init();

   InputSystem* cInputSystem = Allocator::alloc<InputSystem>();
   GEInvokeCtor(InputSystem, cInputSystem);

   ObjectManagers* cObjectManagers = Allocator::alloc<ObjectManagers>();
   GEInvokeCtor(ObjectManagers, cObjectManagers);

   ContentManager* cContentManager = Allocator::alloc<ContentManager>();
   GEInvokeCtor(ContentManager, cContentManager);

   LocalizedStringsManager* cLocaStringsManager = Allocator::alloc<LocalizedStringsManager>();
   GEInvokeCtor(LocalizedStringsManager, cLocaStringsManager);

   registerComponentFactories();
}

void Application::shutDown()
{
   GEInvokeDtor(LocalizedStringsManager, LocalizedStringsManager::getInstance());
   Allocator::free(LocalizedStringsManager::getInstance());

   GEInvokeDtor(ContentManager, ContentManager::getInstance());
   Allocator::free(ContentManager::getInstance());

   GEInvokeDtor(ObjectManagers, ObjectManagers::getInstance());
   Allocator::free(ObjectManagers::getInstance());

   GEInvokeDtor(InputSystem, InputSystem::getInstance());
   Allocator::free(InputSystem::getInstance());

   Allocator::release();
}

void Application::registerComponentFactories()
{
   Entity::registerComponentFactory<ComponentTransform>("Transform", ComponentType::Transform);
   Entity::registerComponentFactory<ComponentCamera>("Camera", ComponentType::Camera);
   Entity::registerComponentFactory<ComponentLight>("Light", ComponentType::Light);
   Entity::registerComponentFactory<ComponentUIElement>("UIElement", ComponentType::UIElement);
   Entity::registerComponentFactory<ComponentSprite>("Sprite", ComponentType::Renderable);
   Entity::registerComponentFactory<ComponentLabel>("Label", ComponentType::Renderable);
   Entity::registerComponentFactory<ComponentMesh>("Mesh", ComponentType::Renderable);
   Entity::registerComponentFactory<ComponentParticleSystem>("ParticleSystem", ComponentType::Renderable);
   Entity::registerComponentFactory<ComponentSkeleton>("Skeleton", ComponentType::Skeleton);
   Entity::registerComponentFactory<ComponentColliderSphere>("ColliderSphere", ComponentType::Collider);
   Entity::registerComponentFactory<ComponentColliderMesh>("ColliderMesh", ComponentType::Collider);
   Entity::registerComponentFactory<ComponentGenericData>("GenericData", ComponentType::GenericData);
   Entity::registerComponentFactory<ComponentScript>("Script", ComponentType::Script);
}
