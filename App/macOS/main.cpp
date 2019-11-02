
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
#include "Core/GEDistributionPlatform.h"

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
       Application::Arguments.push_back(argv[i]);

#if defined (GE_BINARY_CONTENT)
    Application::ContentType = ApplicationContentType::Bin;
#endif

    // initialize the distribution platform
    DistributionPlatform distributionPlatform;
   
    if(!distributionPlatform.init())
       return 1;
   
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

    if(gSettings.getFullscreen())
    {
       glutFullScreen();
       CGDisplayHideCursor(kCGDirectMainDisplay);
    }

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
      glutDestroyWindow(glutGetWindow());
      exit(0);
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
      if(gSettings.getFullscreen())
      {
         InputSystem::getInstance()->inputTouchMove(0, vMouseLastPosition, vMouseCurrentPosition);
      }
      else if(x < 0 || y < 0 || x >= Device::ScreenWidth || y >= Device::ScreenHeight)
      {
         mouseButton(GLUT_LEFT_BUTTON, GLUT_UP, x, y);
      }
   }

   vMouseLastPosition = vMouseCurrentPosition;
}
