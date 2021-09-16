
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
#include "GEDevice.h"
#include "GEPlatform.h"
#include "GETime.h"
#include "GESettings.h"
#include "GELog.h"
#include "GEDistributionPlatform.h"

#include "Input/GEInputSystem.h"

#include "Content/GEResourcesManager.h"
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
#include "Entities/GEComponentDataContainer.h"
#include "Entities/GEComponentAudio.h"
#include "Entities/GEComponentScript.h"

#include "Scripting/GEScriptingEnvironment.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Input;
using namespace GE::Content;
using namespace GE::Entities;
using namespace GE::Audio;
using namespace GE::Scripting;


class LogFileWriter : public LogListener
{
private:
   std::ofstream mLogFile;

public:
   LogFileWriter()
   {
      mLogFile = Device::writeUserFile(".", "logs", "txt");
   }
   ~LogFileWriter()
   {
      mLogFile.close();
   }

   virtual void onLog(LogType pType, const char* pMessage) override
   {
      char timeBuffer[32];
      const time_t timeStamp = time(nullptr);
      tm* brokenDownTimeStamp = localtime(&timeStamp);
      sprintf(timeBuffer, "[%02d:%02d:%02d] ",
         brokenDownTimeStamp->tm_hour,
         brokenDownTimeStamp->tm_min,
         brokenDownTimeStamp->tm_sec);

      if(pType == LogType::Info)
      {
         mLogFile.write("[INF] ", 6u);
      }
      else if(pType == LogType::Warning)
      {
         mLogFile.write("[WAR] ", 6u);
      }
      else
      {
         mLogFile.write("[ERR] ", 6u);
      }

      mLogFile.write(timeBuffer, 11u);
      mLogFile.write(pMessage, strlen(pMessage));
      mLogFile.write("\n", 1u);

      mLogFile.flush();
   }
};

LogFileWriter* gLogFileWriter = nullptr;


const char* Application::Name = nullptr;
const char* Application::ID = nullptr;

const char* Application::VersionString = nullptr;
uint Application::VersionNumber = 0u;

const char* Application::ExecutablePath = nullptr;
GESTLVector(const char*) Application::Arguments;

#if defined (GE_RENDERING_API_DIRECTX)
const ApplicationRenderingAPI Application::RenderingAPI = ApplicationRenderingAPI::DirectX;
#else
const ApplicationRenderingAPI Application::RenderingAPI = ApplicationRenderingAPI::OpenGL;
#endif

ApplicationContentType Application::ContentType = ApplicationContentType::Xml;

Scripting::Environment* Application::smScriptingEnvironments[Application::ScriptingEnvironmentsCount];

void Application::startUp(void (*pInitAppModuleFunction)())
{
   Allocator::init();
   Device::init();

   Settings::getInstance()->load();

   if(Settings::getInstance()->getDumpLogs())
   {
      gLogFileWriter = Allocator::alloc<LogFileWriter>();
      GEInvokeCtor(LogFileWriter, gLogFileWriter);
      Log::addListener(gLogFileWriter);
   }

   Device::ContentHashPath = ContentType == ApplicationContentType::Bin;

   const char* language = Settings::getInstance()->getLanguage();

   if(strcmp(language, "Default") == 0)
   {
      Device::Language = DistributionPlatform::getInstance()->getLanguage();

      if(Device::Language == SystemLanguage::Count)
      {
         Device::Language = Device::requestOSLanguage();
      }
   }
   else
   {
      for(uint32_t i = 0u; i < (uint32_t)SystemLanguage::Count; i++)
      {
         if(strcmp(language, strSystemLanguage[i]) == 0)
         {
            Device::Language = (SystemLanguage)i;
            break;
         }
      }
   }

   InputSystem* cInputSystem = Allocator::alloc<InputSystem>();
   GEInvokeCtor(InputSystem, cInputSystem);
   cInputSystem->init();

   SerializableResourcesManager* cSerializableResourcesManager = Allocator::alloc<SerializableResourcesManager>();
   GEInvokeCtor(SerializableResourcesManager, cSerializableResourcesManager);

   ResourcesManager* cResourcesManager = Allocator::alloc<ResourcesManager>();
   GEInvokeCtor(ResourcesManager, cResourcesManager);

   LocalizedStringsManager* cLocaStringsManager = Allocator::alloc<LocalizedStringsManager>();
   GEInvokeCtor(LocalizedStringsManager, cLocaStringsManager);

   Time::init();

   if(pInitAppModuleFunction)
   {
      pInitAppModuleFunction();
   }

   Environment::initStaticData();

   for(uint32_t i = 0; i < ScriptingEnvironmentsCount; i++)
   {
      smScriptingEnvironments[i] = Allocator::alloc<Environment>();
      GEInvokeCtor(Environment, smScriptingEnvironments[i]);
      smScriptingEnvironments[i]->load();
   }

   Scene::loadPrefabData();
   Scene::initStaticScenes();

   registerComponentFactories();
}

void Application::tick()
{
   InputSystem::getInstance()->update();

   for(uint32_t i = 0; i < ScriptingEnvironmentsCount; i++)
   {
      smScriptingEnvironments[i]->collectGarbageStep();
   }

   Device::update();
   DistributionPlatform::getInstance()->update();
}

void Application::shutDown()
{
   Scene::releaseStaticScenes();
   Scene::unloadPrefabData();

   for(uint32_t i = 0; i < ScriptingEnvironmentsCount; i++)
   {
      GEInvokeDtor(Environment, smScriptingEnvironments[i]);
      Allocator::free(smScriptingEnvironments[i]);
      smScriptingEnvironments[i] = 0;
   }

   Scripting::Environment::releaseStaticData();

   Time::release();

   GEInvokeDtor(LocalizedStringsManager, LocalizedStringsManager::getInstance());
   Allocator::free(LocalizedStringsManager::getInstance());

   GEInvokeDtor(SerializableResourcesManager, SerializableResourcesManager::getInstance());
   Allocator::free(SerializableResourcesManager::getInstance());

   GEInvokeDtor(ResourcesManager, ResourcesManager::getInstance());
   Allocator::free(ResourcesManager::getInstance());

   InputSystem::getInstance()->shutdown();
   GEInvokeDtor(InputSystem, InputSystem::getInstance());
   Allocator::free(InputSystem::getInstance());

   GEInvokeDtor(AudioSystem, AudioSystem::getInstance());
   Allocator::free(AudioSystem::getInstance());

   GEInvokeDtor(RenderSystem, RenderSystem::getInstance());
   Allocator::free(RenderSystem::getInstance());

   GEInvokeDtor(TaskManager, TaskManager::getInstance());
   Allocator::free(TaskManager::getInstance());

   if(gLogFileWriter)
   {
      GEInvokeDtor(LogFileWriter, gLogFileWriter);
      Allocator::free(gLogFileWriter);
      gLogFileWriter = nullptr;
   }

   Device::release();
   Allocator::release();
}

Scripting::Environment* Application::getScriptingEnvironment(uint32_t pIndex)
{
   GEAssert(pIndex < ScriptingEnvironmentsCount);
   return smScriptingEnvironments[pIndex];
}

void Application::registerComponentFactories()
{
   Entity::registerComponentFactory<ComponentTransform>("Transform", ComponentType::Transform);
   Entity::registerComponentFactory<ComponentCamera>("Camera", ComponentType::Camera);
   Entity::registerComponentFactory<ComponentLight>("Light", ComponentType::Light);
   Entity::registerComponentFactory<ComponentUI2DElement>("UI2DElement", ComponentType::UIElement);
   Entity::registerComponentFactory<ComponentUI3DElement>("UI3DElement", ComponentType::UIElement);
   Entity::registerComponentFactory<ComponentUI3DCanvas>("UI3DCanvas", ComponentType::UIElement);
   Entity::registerComponentFactory<ComponentSprite>("Sprite", ComponentType::Renderable);
   Entity::registerComponentFactory<ComponentLabel>("Label", ComponentType::Renderable);
   Entity::registerComponentFactory<ComponentMesh>("Mesh", ComponentType::Renderable);
   Entity::registerComponentFactory<ComponentParticleSystem>("ParticleSystem", ComponentType::Renderable);
   Entity::registerComponentFactory<ComponentSkeleton>("Skeleton", ComponentType::Skeleton);
   Entity::registerComponentFactory<ComponentColliderSphere>("ColliderSphere", ComponentType::Collider);
   Entity::registerComponentFactory<ComponentColliderMesh>("ColliderMesh", ComponentType::Collider);
   Entity::registerComponentFactory<ComponentDataContainer>("DataContainer", ComponentType::DataContainer);
   Entity::registerComponentFactory<ComponentAudioListener>("AudioListener", ComponentType::Audio);
   Entity::registerComponentFactory<ComponentAudioSource2D>("AudioSource2D", ComponentType::Audio);
   Entity::registerComponentFactory<ComponentAudioSource3D>("AudioSource3D", ComponentType::Audio);
   Entity::registerComponentFactory<ComponentScript>("Script", ComponentType::Script);
}
