
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  macOS
//
//  --- main.cpp ---
//
//////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "main.h"
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
#include "Input/GEInputSystem.h"

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
       Application::Arguments.push_back(argv[i]);

#if defined (GE_BINARY_CONTENT)
    Application::ContentType = ApplicationContentType::Bin;
#endif

    Application::startUp();

    // screen size
    CGRect cgBounds = CGDisplayBounds(kCGDirectMainDisplay);
    iFullscreenWidth = (int)cgBounds.size.width;
    iFullscreenHeight = (int)cgBounds.size.height;

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
    strcpy(sLanguageName, "English");

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

#if defined (GE_FULLSCREEN_MODE)
    glutFullScreen();
    CGDisplayHideCursor(kCGDirectMainDisplay);
#endif

    cRender = Allocator::alloc<RenderSystemES20>();
    GEInvokeCtor(RenderSystemES20, cRender)();
    //cAudio = Allocator::alloc<AudioSystemOpenAL>();
    //GEInvokeCtor(AudioSystemOpenAL, cAudio)();
    //cAudio->init();
    //cAudio->setListenerPosition(GE::Vector3(0.0f, 0.0f, 0.0f));

    // timer
    cTimer.start();
    Time::reset();

    dTimeInterval = 1000000.0 / GE_FPS;
    dTimeDelta = 0.0;
    dTimeBefore = 0.0;
    
    // create and register the states
    StateManager cStateManager;
    registerStates();

    // create the task manager
    TaskManager* cTaskManager = Allocator::alloc<TaskManager>();
    GEInvokeCtor(TaskManager, cTaskManager);

    // initialize app module
    initAppModule();

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
}

GE::Vector2 getMouseScreenPosition()
{
   return GE::Vector2(cPixelToScreenX->y((float)iMouseX), cPixelToScreenY->y((float)iMouseY));
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
   iMouseX = x;
   iMouseY = y;
   
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