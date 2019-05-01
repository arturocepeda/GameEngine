
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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

int iFullscreenWidth;
int iFullscreenHeight;
bool bMousePositionSet;

Timer cTimer;
double dTimeInterval;
double dTimeDelta;
double dTimeBefore;
double dTimeNow;

// engine objects
RenderSystem* cRender;              // rendering system
AudioSystem* cAudio;                // audio system

// mouse
POINT pMouse;
Scaler* cPixelToScreenX;
Scaler* cPixelToScreenY;
bool bMouseLeftButton = false;
GE::Vector2 vMouseLastPosition(0.0f, 0.0f);

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

   // initialize the application
   StateManager cStateManager;
   Application::startUp(initAppModule);

   // screen size
   iFullscreenWidth = GetSystemMetrics(SM_CXSCREEN);
   iFullscreenHeight = GetSystemMetrics(SM_CYSCREEN);

#if defined (GE_FULLSCREEN_MODE)
   Device::ScreenWidth = iFullscreenWidth;
   Device::ScreenHeight = iFullscreenHeight;
#else
   Device::ScreenWidth = 1024;
   Device::ScreenHeight = 600;
#endif

   cPixelToScreenX = Allocator::alloc<Scaler>();
   GEInvokeCtor(Scaler, cPixelToScreenX)(0.0f, (float)Device::ScreenWidth, -1.0f, 1.0f);
   cPixelToScreenY = Allocator::alloc<Scaler>();
   GEInvokeCtor(Scaler, cPixelToScreenY)(0.0f, (float)Device::ScreenHeight, (float)Device::getAspectRatio(), (float)-Device::getAspectRatio());

   int iWindowWidth = Device::ScreenWidth;
   int iWindowHeight = Device::ScreenHeight;

   // system language
   char sLanguageName[32];
   LCID lcidUserDefault = GetUserDefaultLCID();
   GetLocaleInfoA(lcidUserDefault, LOCALE_SENGLISHLANGUAGENAME, sLanguageName, sizeof(sLanguageName) / sizeof(TCHAR));

   if(strcmp(sLanguageName, "English") == 0)
   {
      Device::Language = SystemLanguage::English;
   }
   else if(strcmp(sLanguageName, "Spanish") == 0)
   {
      Device::Language = SystemLanguage::Spanish;
   }
   else if(strcmp(sLanguageName, "German") == 0)
   {
      Device::Language = SystemLanguage::German;
   }

   // create the main window
   GE::uint iWindowPositionX = (iFullscreenWidth / 2) - (iWindowWidth / 2);
   GE::uint iWindowPositionY = (iFullscreenHeight / 2) - (iWindowHeight / 2);

   // initialize rendering and sound systems
   glutInit(&__argc, __argv);
   glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
   glutInitWindowPosition(iWindowPositionX, iWindowPositionY);
   glutInitWindowSize(Device::ScreenWidth, Device::ScreenHeight);
   glutCreateWindow(GE_APP_NAME);
   glewInit();

#if defined (GE_FULLSCREEN_MODE)
   glutFullScreen();
#endif

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

   dTimeInterval = 1000000.0 / GE_FPS;
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

   cAudio->release();
    
   GEInvokeDtor(AudioSystem, cAudio);
   Allocator::free(cAudio);
   GEInvokeDtor(RenderSystem, cRender);
   Allocator::free(cRender);
   GEInvokeDtor(TaskManager, cTaskManager);
   Allocator::free(cTaskManager);

   Allocator::free(cPixelToScreenX);
   Allocator::free(cPixelToScreenY);

   Application::shutDown();

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
         fTimeDelta = 1.0f / GE_FPS;

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
#if !defined (GE_FULLSCREEN_MODE)
      if(x < 0 || y < 0 || x >= Device::ScreenWidth || y >= Device::ScreenHeight)
      {
         mouseButton(GLUT_LEFT_BUTTON, GLUT_UP, x, y);
      }
      else
#endif
      {
         InputSystem::getInstance()->inputTouchMove(0, vMouseLastPosition, vMouseCurrentPosition);
      }
   }

   vMouseLastPosition = vMouseCurrentPosition;
}
