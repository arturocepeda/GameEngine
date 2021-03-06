
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Windows (OpenGL)
//
//  --- main.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "main.opengl.h"
#include "config.h"

#include "Core/GEApplication.h"
#include "Core/GEDevice.h"
#include "Core/GETimer.h"
#include "Core/GETime.h"
#include "Core/GEStateManager.h"
#include "Core/GETaskManager.h"
#include "Core/GEAllocator.h"
#include "Core/GELog.h"
#include "Core/GEDistributionPlatform.h"

#include "Content/GEImageData.h"

#include "Rendering/OpenGL/GEOpenGLES20.h"
#include "Rendering/OpenGL/GERenderSystemES20.h"
#include "Audio/GEAudioSystem.h"
#include "Input/GEInputSystem.h"

#include "Externals/glfw/include/GLFW/glfw3.h"
#include "Input/GEGamepad.GLFW.h"

#pragma comment(lib, "GameEngine.OpenGL.lib")
#pragma comment(lib, "AppModule.lib")
#pragma comment(lib, "opengl32.lib")

#if defined (_M_X64)
# pragma comment(lib, "../../../GameEngine/Externals/glew/lib/Release/x64/glew32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/glfw/lib-vc2017/glfw3.lib")
# pragma comment(lib, "../../../GameEngine/Externals/OpenAL/lib/Win64/OpenAL32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libogg/lib/x64/libogg_static.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libvorbis/lib/x64/libvorbis_static.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libvorbis/lib/x64/libvorbisfile_static.lib")
# pragma comment(lib, "../../../GameEngine/Externals/Brofiler/ProfilerCore64.lib")
#else
# pragma comment(lib, "../../../GameEngine/Externals/glew/lib/Release/Win32/glew32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/glfw/lib-vc2017/glfw3.lib")
# pragma comment(lib, "../../../GameEngine/Externals/OpenAL/lib/Win32/OpenAL32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/Brofiler/ProfilerCore32.lib")
#endif

using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Input;
using namespace GE::Content;

bool bMousePositionSet;

Timer cTimer;
double dTimeInterval;
double dTimeDelta;
double dTimeBefore;
double dTimeNow;

// engine objects
RenderSystem* cRender;              // rendering system
AudioSystem* cAudio;                // audio system
AppSettings gSettings;              // settings

// mouse
POINT pMouse;
Scaler* cPixelToScreenX;
Scaler* cPixelToScreenY;
bool bMouseLeftButton = false;
GE::Vector2 vMouseLastPosition(0.0f, 0.0f);

// log listener
class Win32LogListener : public LogListener
{
public:
   virtual void onLog(LogType pLogType, const char* pMsg) override
   {
      if(pLogType == LogType::Error)
      {
         MessageBoxA(nullptr, pMsg, "Error", MB_OK | MB_ICONERROR);
      }
   }
}
gLogListener;

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
   Application::Name = GE_APP_NAME;
   Application::ID = GE_APP_ID;
   Application::VersionString = GE_VERSION_STRING;
   Application::VersionNumber = GE_VERSION_NUMBER;
   Application::ExecutablePath = (const char*)__argv[0];

   for(int i = 1; i < __argc; i++)
   {
      Application::Arguments.push_back(__argv[i]);
   }

#if defined (GE_BINARY_CONTENT)
   Application::ContentType = ApplicationContentType::Bin;
#endif

   if(gSettings.getErrorPopUps())
   {
      Log::addListener(&gLogListener);
   }

   // initialize the distribution platform
   DistributionPlatform distributionPlatform;

   if(!distributionPlatform.init())
   {
      return 1;
   }

   // initialize the application
   StateManager cStateManager;
   Application::startUp(initAppModule);

   // screen size
   SetProcessDPIAware();

   const int systemScreenWidth = GetSystemMetrics(SM_CXSCREEN);
   const int systemScreenHeight = GetSystemMetrics(SM_CYSCREEN);

   if(gSettings.getFullscreen())
   {
      Device::ScreenWidth = (int)gSettings.getFullscreenSizeX();
      Device::ScreenHeight = (int)gSettings.getFullscreenSizeY();

      if(Device::ScreenWidth > 0 && Device::ScreenHeight > 0)
      {
         DEVMODE devMode = { 0 };
         devMode.dmSize = sizeof(DEVMODE);
         devMode.dmPelsWidth = Device::ScreenWidth;
         devMode.dmPelsHeight = Device::ScreenHeight;
         devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
         ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
      }
      else
      {
         Device::ScreenWidth = systemScreenWidth;
         Device::ScreenHeight = systemScreenHeight;
      }

      Device::AspectRatio = (float)systemScreenHeight / (float)systemScreenWidth;
   }
   else
   {
      Device::ScreenWidth = (int)gSettings.getWindowSizeX();
      Device::ScreenHeight = (int)gSettings.getWindowSizeY();
      Device::AspectRatio = (float)Device::ScreenHeight / (float)Device::ScreenWidth;
   }

   cPixelToScreenX = Allocator::alloc<Scaler>();
   GEInvokeCtor(Scaler, cPixelToScreenX)(0.0f, (float)Device::ScreenWidth, -1.0f, 1.0f);
   cPixelToScreenY = Allocator::alloc<Scaler>();
   GEInvokeCtor(Scaler, cPixelToScreenY)(0.0f, (float)Device::ScreenHeight, (float)Device::getAspectRatio(), (float)-Device::getAspectRatio());

   // initialize rendering and sound systems
   glfwInit();

   GLFWmonitor* monitor = nullptr;

   if(gSettings.getFullscreen())
   {
      monitor = glfwGetPrimaryMonitor();
   }

   const int windowPositionX = (systemScreenWidth / 2) - (Device::ScreenWidth / 2);
   const int windowPositionY = (systemScreenHeight / 2) - (Device::ScreenHeight / 2);

   GLFWwindow* window = glfwCreateWindow(Device::ScreenWidth, Device::ScreenHeight, GE_APP_NAME, monitor, nullptr);
   glfwSetWindowPos(window, windowPositionX, windowPositionY);
   glfwMakeContextCurrent(window);

   Device::ContentHashPath = false;

   if(Device::contentFileExists(".", "icon", "png"))
   {
      ImageData imageData;
      Device::readContentFile(ContentType::Texture, ".", "icon", "png", &imageData);

      GLFWimage image;
      image.width = imageData.getWidth();
      image.height = imageData.getHeight();
      image.pixels = (unsigned char*)imageData.getData();

      glfwSetWindowIcon(window, 1, &image);
   }

   Device::ContentHashPath = Application::ContentType == ApplicationContentType::Bin;

   glewInit();

   int framebufferWidth;
   int framebufferHeight;
   glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
   glViewport(0, 0, framebufferWidth, framebufferHeight);

   bool vsync = gSettings.getVSync();
   glfwSwapInterval((int)vsync);

   cRender = Allocator::alloc<RenderSystemES20>();
   GEInvokeCtor(RenderSystemES20, cRender)();
   cAudio = Allocator::alloc<AudioSystem>();
   GEInvokeCtor(AudioSystem, cAudio)();
   cAudio->init();
   cAudio->setListenerPosition(GE::Vector3(0.0f, 0.0f, 0.0f));

#if !defined (_DEBUG)
   // hide the mouse pointer
   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif

   // timer
   cTimer.start();
   Time::reset();

   dTimeInterval = 1000000.0 / gSettings.getTargetFPS();
   dTimeDelta = 0.0;
   dTimeBefore = 0.0;
   dTimeNow;

   // create the task manager
   TaskManager* cTaskManager = Allocator::alloc<TaskManager>();
   GEInvokeCtor(TaskManager, cTaskManager);

   glfwSetKeyCallback(window, keyboard);
   glfwSetCharCallback(window, keyboardText);
   glfwSetMouseButtonCallback(window, mouseButton);
   glfwSetCursorPosCallback(window, mouseMove);
   glfwSetScrollCallback(window, mouseWheel);

   while(!TaskManager::getInstance()->getExitPending() && !glfwWindowShouldClose(window))
   {
      if(gSettings.getVSync() != vsync)
      {
         vsync = gSettings.getVSync();

         dTimeDelta = 0.0;
         dTimeBefore = 0.0;

         glfwSwapInterval((int)vsync);
      }

      dTimeNow = cTimer.getTime();
      dTimeDelta = dTimeNow - dTimeBefore;

      bool renderFrame = false;

      if(vsync)
      {
         renderFrame = true;
      }
      else
      {
         renderFrame = dTimeDelta >= dTimeInterval;
      }

      if(renderFrame)
      {
         dTimeBefore = dTimeNow;

         float fTimeDelta = (float)dTimeDelta * 0.000001f;

         if(fTimeDelta > 1.0f)
         {
            fTimeDelta = 1.0f / gSettings.getTargetFPS();
         }

         Time::setDelta(fTimeDelta);

         checkGamepadState();

         TaskManager::getInstance()->update();
         TaskManager::getInstance()->render();

         glfwSwapBuffers(window);
         glfwPollEvents();
      }
   }

   cStateManager.getActiveState()->deactivate();
   cStateManager.releaseStates();

   Allocator::free(cPixelToScreenX);
   Allocator::free(cPixelToScreenY);

   Application::shutDown();

   distributionPlatform.shutdown();

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}

GE::Vector2 getMouseScreenPosition()
{
   return GE::Vector2(cPixelToScreenX->y((float)pMouse.x), cPixelToScreenY->y((float)pMouse.y));
}

void keyboard(GLFWwindow*, int pKey, int pScancode, int pAction, int pMods)
{
   (void)pScancode; (void)pMods;

   InputSystem::getInstance()->setCurrentInputDevice(InputDevice::Keyboard);

   if(pAction == GLFW_PRESS)
   {
      InputSystem::getInstance()->inputKeyPress((char)pKey);
   }
   else if(pAction == GLFW_RELEASE)
   {
      InputSystem::getInstance()->inputKeyRelease((char)pKey);
   }

   if(pAction == GLFW_PRESS || pAction == GLFW_REPEAT)
   {
      if(pKey == GLFW_KEY_ENTER)
      {
         keyboardText(nullptr, 13u);
      }
      else if(pKey == GLFW_KEY_BACKSPACE)
      {
         keyboardText(nullptr, 8u);
      }
   }
}

void keyboardText(GLFWwindow*, unsigned int pCodePoint)
{
   InputSystem::getInstance()->setCurrentInputDevice(InputDevice::Keyboard);
   InputSystem::getInstance()->inputKeyText((uint16_t)pCodePoint);
}

void mouseButton(GLFWwindow*, int pButton, int pAction, int pMods)
{
   (void)pMods;

   InputSystem::getInstance()->setCurrentInputDevice(InputDevice::Mouse);

   // left button
   if(pButton == GLFW_MOUSE_BUTTON_LEFT)
   {
      if(pAction == GLFW_PRESS)
      {
         bMouseLeftButton = true;
         vMouseLastPosition = getMouseScreenPosition();
         InputSystem::getInstance()->inputMouseLeftButton();
         InputSystem::getInstance()->inputTouchBegin(0, vMouseLastPosition);
      }
      else if(pAction == GLFW_RELEASE)
      {
         bMouseLeftButton = false;
         vMouseLastPosition = getMouseScreenPosition();
         InputSystem::getInstance()->inputTouchEnd(0, vMouseLastPosition);
      }
   }
   // right button
   else if(pButton == GLFW_MOUSE_BUTTON_RIGHT)
   {
      if(pAction == GLFW_PRESS)
      {
         InputSystem::getInstance()->inputMouseRightButton();
      }
   }
}

void mouseMove(GLFWwindow*, double pX, double pY)
{
   pMouse.x = (LONG)pX;
   pMouse.y = (LONG)pY;

   InputSystem::getInstance()->setCurrentInputDevice(InputDevice::Mouse);

   GE::Vector2 vMouseCurrentPosition = getMouseScreenPosition();
   InputSystem::getInstance()->inputMouse(vMouseCurrentPosition);

   if(bMouseLeftButton)
   {
      InputSystem::getInstance()->inputTouchMove(0, vMouseLastPosition, vMouseCurrentPosition);

      if(!gSettings.getFullscreen() &&
         (pMouse.x < 0 || pMouse.y < 0 || pMouse.x >= Device::ScreenWidth || pMouse.y >= Device::ScreenHeight))
      {
         mouseButton(nullptr, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
      }
   }

   vMouseLastPosition = vMouseCurrentPosition;
}

void mouseWheel(GLFWwindow*, double pXOffset, double pYOffset)
{
   (void)pXOffset;
   InputSystem::getInstance()->setCurrentInputDevice(InputDevice::Mouse);
   InputSystem::getInstance()->inputMouseWheel((int)pYOffset);
}
