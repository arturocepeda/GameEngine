
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

#include <iostream>
#include <vector>

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

#include "Rendering/OpenGL/GEOpenGLES20.h"
#include "Rendering/OpenGL/GERenderSystemES20.h"
#include "Audio/GEAudioSystem.h"
#include "Input/GEInputSystem.h"

#define FREEGLUT_LIB_PRAGMAS 0
#include "Externals/freeglut/include/GL/freeglut.h"

#pragma comment(lib, "GameEngine.OpenGL.lib")
#pragma comment(lib, "AppModule.lib")
#pragma comment(lib, "opengl32.lib")

#if defined (_M_X64)
# pragma comment(lib, "../../../GameEngine/Externals/glew/lib/Release/x64/glew32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/freeglut/lib/x64/freeglut.lib")
# pragma comment(lib, "../../../GameEngine/Externals/OpenAL/lib/Win64/OpenAL32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libogg/lib/x64/libogg_static.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libvorbis/lib/x64/libvorbis_static.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libvorbis/lib/x64/libvorbisfile_static.lib")
# pragma comment(lib, "../../../GameEngine/Externals/Brofiler/ProfilerCore64.lib")
#else
# pragma comment(lib, "../../../GameEngine/Externals/glew/lib/Release/Win32/glew32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/freeglut/lib/freeglut.lib")
# pragma comment(lib, "../../../GameEngine/Externals/OpenAL/lib/Win32/OpenAL32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/Brofiler/ProfilerCore32.lib")
#endif

using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Input;

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
      if(pLogType == LogType::Error && gSettings.getErrorPopUps())
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

   Log::CurrentLogListener = &gLogListener;

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
   SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

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

   int iWindowWidth = Device::ScreenWidth;
   int iWindowHeight = Device::ScreenHeight;

   // create the main window
   GE::uint iWindowPositionX = (systemScreenWidth / 2) - (iWindowWidth / 2);
   GE::uint iWindowPositionY = (systemScreenHeight / 2) - (iWindowHeight / 2);

   // initialize rendering and sound systems
   glutInit(&__argc, __argv);
   glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
   glutInitWindowPosition(iWindowPositionX, iWindowPositionY);
   glutInitWindowSize(Device::ScreenWidth, Device::ScreenHeight);
   glutCreateWindow(GE_APP_NAME);
   glewInit();

   if(gSettings.getFullscreen())
   {
      glutFullScreen();
   }

   cRender = Allocator::alloc<RenderSystemES20>();
   GEInvokeCtor(RenderSystemES20, cRender)();
   cAudio = Allocator::alloc<AudioSystem>();
   GEInvokeCtor(AudioSystem, cAudio)();
   cAudio->init();
   cAudio->setListenerPosition(GE::Vector3(0.0f, 0.0f, 0.0f));

#if !defined (_DEBUG)
   // hide the mouse pointer
   glutSetCursor(GLUT_CURSOR_NONE);
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

   // game loop
   glutDisplayFunc(render);
   glutIdleFunc(render);

   glutKeyboardFunc(keyboardDown);
   glutKeyboardUpFunc(keyboardUp);
   glutMouseFunc(mouseButton);
   glutMotionFunc(mouseMove);
   glutPassiveMotionFunc(mouseMove);

   glutMainLoop();

   // end of the loop
   cStateManager.getActiveState()->deactivate();
   cStateManager.releaseStates();

   Allocator::free(cPixelToScreenX);
   Allocator::free(cPixelToScreenY);

   Application::shutDown();

   distributionPlatform.shutdown();

   return 0;
}

void render()
{
   dTimeNow = cTimer.getTime();
   dTimeDelta = dTimeNow - dTimeBefore;

   if(dTimeDelta >= dTimeInterval)
   {
      dTimeBefore = dTimeNow;

      float fTimeDelta = (float)dTimeDelta * 0.000001f;

      if(fTimeDelta > 1.0f)
         fTimeDelta = 1.0f / gSettings.getTargetFPS();

      Time::setDelta(fTimeDelta);

      TaskManager::getInstance()->update();
      TaskManager::getInstance()->render();

      glutSwapBuffers();
   }

   if(TaskManager::getInstance()->getExitPending())
   {
      glutLeaveMainLoop();
   }
}

GE::Vector2 getMouseScreenPosition()
{
   return GE::Vector2(cPixelToScreenX->y((float)pMouse.x), cPixelToScreenY->y((float)pMouse.y));
}

void keyboardDown(unsigned char key, int x, int y)
{
   InputSystem::getInstance()->inputKeyPress((char)key);
}

void keyboardUp(unsigned char key, int x, int y)
{
   InputSystem::getInstance()->inputKeyRelease((char)key);
}

void mouseButton(int button, int state, int x, int y)
{
   // left button
   if(button == GLUT_LEFT_BUTTON)
   {
      if(state == GLUT_DOWN)
      {
         bMouseLeftButton = true;
         vMouseLastPosition = getMouseScreenPosition();
         InputSystem::getInstance()->inputMouseLeftButton();
         InputSystem::getInstance()->inputTouchBegin(0, vMouseLastPosition);
      }
      else
      {
         bMouseLeftButton = false;
         vMouseLastPosition = getMouseScreenPosition();
         InputSystem::getInstance()->inputTouchEnd(0, vMouseLastPosition);
      }
   }
   // right button
   else if(button == GLUT_RIGHT_BUTTON)
   {
      if(state == GLUT_DOWN)
      {
         InputSystem::getInstance()->inputMouseRightButton();
      }
   }
   // mouse wheel forward
   else if(button == 3)
   {
      InputSystem::getInstance()->inputMouseWheel(+1);
   }
   // mouse wheel backward
   else if(button == 4)
   {
      InputSystem::getInstance()->inputMouseWheel(-1);
   }
}

void mouseMove(int x, int y)
{
   pMouse.x = x;
   pMouse.y = y;

   GE::Vector2 vMouseCurrentPosition = getMouseScreenPosition();
   InputSystem::getInstance()->inputMouse(vMouseCurrentPosition);

   if(bMouseLeftButton)
   {
      InputSystem::getInstance()->inputTouchMove(0, vMouseLastPosition, vMouseCurrentPosition);

      if(!gSettings.getFullscreen() &&
         (x < 0 || y < 0 || x >= Device::ScreenWidth || y >= Device::ScreenHeight))
      {
         mouseButton(GLUT_LEFT_BUTTON, GLUT_UP, x, y);
      }
   }

   vMouseLastPosition = vMouseCurrentPosition;
}
