
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda PŽrez
//  Game Engine
//
//  macOS
//
//  --- main.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "main.h"
#include "config.h"

#include "Core/GEApplication.h"
#include "Core/GEDevice.h"
#include "Core/GETimer.h"
#include "Core/GETime.h"
#include "Core/GEStateManager.h"
#include "Core/GETaskManager.h"
#include "Core/GEAllocator.h"
#include "Core/GEDistributionPlatform.h"

#include "Rendering/OpenGL/GEOpenGLES20.h"
#include "Rendering/OpenGL/GERenderSystemES20.h"
#include "Input/GEInputSystem.h"

#include "Externals/glfw/include/GLFW/glfw3.h"

#include <CoreGraphics/CoreGraphics.h>

using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Input;

int iFullscreenWidth = 0;
int iFullscreenHeight = 0;
bool bMousePositionSet = false;

Timer cTimer;
double dTimeInterval = 0.0;
double dTimeDelta = 0.0;
double dTimeBefore = 0.0;
double dTimeNow = 0.0;

// engine objects
RenderSystem* cRender = 0;          // rendering system
AudioSystem* cAudio = 0;            // audio system
AppSettings gSettings;              // settings

// mouse
int iMouseX = 0;
int iMouseY = 0;
Scaler* cPixelToScreenX = 0;
Scaler* cPixelToScreenY = 0;
bool bMouseLeftButton = false;
GE::Vector2 vMouseLastPosition(0.0f, 0.0f);

int main(int argc, char* argv[])
{
   Application::Name = GE_APP_NAME;
   Application::ID = GE_APP_ID;
   Application::VersionString = GE_VERSION_STRING;
   Application::VersionNumber = GE_VERSION_NUMBER;
   Application::ExecutablePath = (const char*)argv[0];

   for(int i = 1; i < argc; i++)
   {
      Application::Arguments.push_back(argv[i]);
   }
        
#if defined (GE_BINARY_CONTENT)
   Application::ContentType = ApplicationContentType::Bin;
#endif
    
   // set the working directory for the application bundle
   char workingDirectory[1024];
   strcpy(workingDirectory, Application::ExecutablePath);
    
   char bundleContentsDir[64];
   sprintf(bundleContentsDir, "/%s.app/Contents", Application::Name);
    
   char* bundlePtr = strstr(workingDirectory, bundleContentsDir);
    
   if(bundlePtr)
   {
      bundlePtr += strlen(bundleContentsDir);
      *bundlePtr = '\0';
      strcat(workingDirectory, "/Resources");
      chdir(workingDirectory);
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
   CGRect cgBounds = CGDisplayBounds(kCGDirectMainDisplay);
   iFullscreenWidth = (int)cgBounds.size.width;
   iFullscreenHeight = (int)cgBounds.size.height;

   if(gSettings.getFullscreen())
   {
      Device::ScreenWidth = iFullscreenWidth;
      Device::ScreenHeight = iFullscreenHeight;
   }
   else
   {
      Device::ScreenWidth = (int)gSettings.getWindowSizeX();
      Device::ScreenHeight = (int)gSettings.getWindowSizeY();
   }
   
   Device::AspectRatio = (float)Device::ScreenHeight / (float)Device::ScreenWidth;

   cPixelToScreenX = Allocator::alloc<Scaler>();
   GEInvokeCtor(Scaler, cPixelToScreenX)(0.0f, (float)Device::ScreenWidth, -1.0f, 1.0f);
   cPixelToScreenY = Allocator::alloc<Scaler>();
   GEInvokeCtor(Scaler, cPixelToScreenY)(0.0f, (float)Device::ScreenHeight, (float)Device::getAspectRatio(), (float)-Device::getAspectRatio());

   const int windowWidth = Device::ScreenWidth;
   const int windowHeight = Device::ScreenHeight;

   // create the main window
   glfwInit();

   GLFWmonitor* monitor = nullptr;

   if(gSettings.getFullscreen())
   {
      monitor = glfwGetPrimaryMonitor();
      CGDisplayHideCursor(kCGDirectMainDisplay);
   }
   
   const int windowPositionX = (iFullscreenWidth / 2) - (windowWidth / 2);
   const int windowPositionY = (iFullscreenHeight / 2) - (windowHeight / 2);
   
   GLFWwindow* window = glfwCreateWindow(Device::ScreenWidth, Device::ScreenHeight, GE_APP_NAME, monitor, nullptr);
   glfwSetWindowPos(window, windowPositionX, windowPositionY);
   glfwMakeContextCurrent(window);

   int framebufferWidth;
   int framebufferHeight;
   glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
   glViewport(0, 0, framebufferWidth, framebufferHeight);

   bool vsync = gSettings.getVSync();
   glfwSwapInterval((int)vsync);
   
   // initialize rendering and sound systems
   cRender = Allocator::alloc<RenderSystemES20>();
   GEInvokeCtor(RenderSystemES20, cRender)();
   
   cAudio = Allocator::alloc<AudioSystem>();
   GEInvokeCtor(AudioSystem, cAudio);
   cAudio->init();
   cAudio->setListenerPosition(GE::Vector3::Zero);

   // timer
   cTimer.start();
   Time::reset();

   dTimeInterval = 1000000.0 / gSettings.getTargetFPS();
   dTimeDelta = 0.0;
   dTimeBefore = 0.0;
    
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
   return GE::Vector2(cPixelToScreenX->y((float)iMouseX), cPixelToScreenY->y((float)iMouseY));
}

void keyboard(GLFWwindow*, int pKey, int pScancode, int pAction, int pMods)
{
   (void)pScancode; (void)pMods;

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
   InputSystem::getInstance()->inputKeyText((uint16_t)pCodePoint);
}

void mouseButton(GLFWwindow*, int pButton, int pAction, int pMods)
{
   (void)pMods;

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
   iMouseX = (int)pX;
   iMouseY = (int)pY;

   GE::Vector2 vMouseCurrentPosition = getMouseScreenPosition();
   InputSystem::getInstance()->inputMouse(vMouseCurrentPosition);

   if(bMouseLeftButton)
   {
      InputSystem::getInstance()->inputTouchMove(0, vMouseLastPosition, vMouseCurrentPosition);

      if(!gSettings.getFullscreen() &&
         (iMouseX < 0 || iMouseY < 0 || iMouseX >= Device::ScreenWidth || iMouseY >= Device::ScreenHeight))
      {
         mouseButton(nullptr, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
      }
   }

   vMouseLastPosition = vMouseCurrentPosition;
}

void mouseWheel(GLFWwindow*, double pXOffset, double pYOffset)
{
   (void)pXOffset;
   InputSystem::getInstance()->inputMouseWheel((int)pYOffset);
}
