
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

#include "Externals/glew/include/GL/glew.h"
#include "Externals/freeglut/include/GL/glut.h"

#include "Rendering/OpenGL/GERenderSystemES20.h"
#include "Audio/FMOD/GEAudioSystemFMOD.h"

#pragma comment(lib, "GameEngine.OpenGL.lib")
#pragma comment(lib, "pugixml.Windows.lib")
#pragma comment(lib, "stb.Windows.lib")
#pragma comment(lib, "AppModule.lib")

#if defined (_M_X64)
# pragma comment(lib, "../../../GameEngine/Externals/glew/lib/Release/x64/glew32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/freeglut/lib/x64/freeglut.lib")
# pragma comment(lib, "../../../GameEngine/Externals/FMOD/lib/fmodex64_vc.lib")
# pragma comment(lib, "../../../GameEngine/Externals/Brofiler/ProfilerCore64.lib")
#else
# pragma comment(lib, "../../../GameEngine/Externals/glew/lib/Release/Win32/glew32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/freeglut/lib/freeglut.lib")
# pragma comment(lib, "../../../GameEngine/Externals/FMOD/lib/fmodex_vc.lib")
# pragma comment(lib, "../../../GameEngine/Externals/Brofiler/ProfilerCore32.lib")
#endif

using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;

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

int main(int argc, char* argv[])
{
    Application::Name = GE_APP_NAME;
    Application::ID = GE_APP_ID;
    Application::VersionString = GE_VERSION_STRING;
    Application::VersionNumber = GE_VERSION_NUMBER;
    Application::ExecutablePath = (const char*)argv[0];

    for(int i = 1; i < argc; i++)
       Application::Arguments.push_back(argv[i]);

#if defined (GE_BINARY_CONTENT)
    Application::ContentType = ApplicationContentType::Bin;
#endif

    Application::startUp();

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
       Device::Language = SystemLanguage::English;
    else if(strcmp(sLanguageName, "Spanish") == 0)
       Device::Language = SystemLanguage::Spanish;
    else if(strcmp(sLanguageName, "German") == 0)
       Device::Language = SystemLanguage::German;

    // create the main window
    GE::uint iWindowPositionX = (iFullscreenWidth / 2) - (iWindowWidth / 2);
    GE::uint iWindowPositionY = (iFullscreenHeight / 2) - (iWindowHeight / 2);

    // initialize rendering and sound systems
    glutInit(&argc, argv);
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
    cAudio = Allocator::alloc<AudioSystemFMOD>();
    GEInvokeCtor(AudioSystemFMOD, cAudio)();
    cAudio->init();
    cAudio->setListenerPosition(GE::Vector3(0.0f, 0.0f, 0.0f));

#ifndef _DEBUG
    // hide the mouse pointer
    //ShowCursor(false);
#endif

    // timer
    cTimer.start();
    Time::reset();

    dTimeInterval = 1000000.0 / GE_FPS;
    dTimeDelta = 0.0;
    dTimeBefore = 0.0;
    dTimeNow;
    
    // create and register the states
    StateManager cStateManager;
    registerStates(cStateManager);

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

      State* cCurrentState = StateManager::getInstance()->getActiveState();

      if(cCurrentState)
         cCurrentState->inputMouse(pMouse.x, pMouse.y);

      TaskManager::getInstance()->update();
      TaskManager::getInstance()->render();

      glutSwapBuffers();
   }
}

GE::Vector2 getMouseScreenPosition()
{
   return GE::Vector2(cPixelToScreenX->y((float)pMouse.x), cPixelToScreenY->y((float)pMouse.y));
}

void keyboardDown(unsigned char key, int x, int y)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();
   cCurrentState->inputKeyPress((char)key);
}

void keyboardUp(unsigned char key, int x, int y)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();
   cCurrentState->inputKeyRelease((char)key);
}

void mouseButton(int button, int state, int x, int y)
{
   State* cCurrentState = StateManager::getInstance()->getActiveState();

   // left button
   if(button == GLUT_LEFT_BUTTON)
   {
      if(state == GLUT_DOWN)
      {
         bMouseLeftButton = true;
         vMouseLastPosition = getMouseScreenPosition();
         cCurrentState->inputMouseLeftButton();
         cCurrentState->inputTouchBegin(0, vMouseLastPosition);
      }
      else
      {
         bMouseLeftButton = false;
         vMouseLastPosition = getMouseScreenPosition();
         cCurrentState->inputTouchEnd(0, vMouseLastPosition);
      }
   }
   // right button
   else if(button == GLUT_RIGHT_BUTTON)
   {
      if(state == GLUT_DOWN)
         cCurrentState->inputMouseRightButton();
   }
   // mouse wheel forward
   else if(button == 3)
   {
      cCurrentState->inputMouseWheel(+1);
   }
   // mouse wheel backward
   else if(button == 4)
   {
      cCurrentState->inputMouseWheel(-1);
   }
}

void mouseMove(int x, int y)
{
   pMouse.x = x;
   pMouse.y = y;

   State* cCurrentState = StateManager::getInstance()->getActiveState();
   cCurrentState->inputMouse(x, y);

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
         GE::Vector2 vMouseCurrentPosition = getMouseScreenPosition();
         cCurrentState->inputTouchMove(0, vMouseLastPosition, vMouseCurrentPosition);
         vMouseLastPosition = vMouseCurrentPosition;      
      }
   }
}
