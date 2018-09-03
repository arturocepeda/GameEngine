
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Windows (DirectX 11)
//
//  --- main.dx11.cpp ---
//
//////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "main.dx11.h"
#include "config.h"

#include "Core/GEDevice.h"
#include "Core/GETimer.h"
#include "Core/GETime.h"
#include "Core/GEStateManager.h"
#include "Core/GETaskManager.h"
#include "Core/GEAllocator.h"
#include "Core/GEApplication.h"
#include "Core/GEProfiler.h"

#include "Rendering/DX11/GERenderSystemDX11.h"
#include "Audio/FMOD/GEAudioSystemFMOD.h"
#include "Input/GEInputSystem.h"

#pragma comment(lib, "GameEngine.DX11.lib")
#pragma comment(lib, "AppModule.lib")

#if defined (_M_X64)
# pragma comment(lib, "../../../GameEngine/Externals/OpenAL/lib/Win64/OpenAL32.lib")
#else
# pragma comment(lib, "../../../GameEngine/Externals/OpenAL/lib/Win32/OpenAL32.lib")
#endif

#if defined (GE_PLATFORM_WINDOWS)
# if defined (_M_X64)
#  pragma comment(lib, "../../../GameEngine/Externals/Brofiler/ProfilerCore64.lib")
# else
#  pragma comment(lib, "../../../GameEngine/Externals/Brofiler/ProfilerCore32.lib")
# endif
#endif

using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Input;

// window
HWND hWnd;

// engine objects
RenderSystem* cRender;              // rendering system
AudioSystem* cAudio;                // audio system
bool bEnd;                          // loop ending flag

// mouse
POINT pMouse;
Scaler* cPixelToScreenX;
Scaler* cPixelToScreenY;
bool bMouseLeftButton = false;
GE::Vector2 vMouseLastPosition(0.0f, 0.0f);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR sCmdLine, int iCmdShow)
{
    Application::Name = GE_APP_NAME;
    Application::ID = GE_APP_ID;
    Application::VersionString = GE_VERSION_STRING;
    Application::VersionNumber = GE_VERSION_NUMBER;
    
#if defined (GE_BINARY_CONTENT)
    Application::ContentType = ApplicationContentType::Bin;
#endif

    Application::startUp();

    // screen size
    int iFullscreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int iFullscreenHeight = GetSystemMetrics(SM_CYSCREEN);

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
    GEInvokeCtor(Scaler, cPixelToScreenY)
       (0.0f, (float)Device::ScreenHeight, (float)Device::getAspectRatio(), (float)-Device::getAspectRatio());

    int iWindowWidth = Device::ScreenWidth;
    int iWindowHeight = Device::ScreenHeight;

#if !defined (GE_FULLSCREEN_MODE)
    int iWindowBorderSize = 8;
    int iWindowCaptionSize = GetSystemMetrics(SM_CYSMCAPTION);

    iWindowWidth += (iWindowBorderSize * 2);
    iWindowHeight += iWindowCaptionSize + (iWindowBorderSize * 2);
#endif

    // system language
    char sLanguageName[32];
    LCID lcidUserDefault = GetUserDefaultLCID();
    GetLocaleInfo(lcidUserDefault, LOCALE_SENGLISHLANGUAGENAME, sLanguageName, sizeof(sLanguageName) / sizeof(TCHAR));

    if(strcmp(sLanguageName, "English") == 0)
       Device::Language = SystemLanguage::English;
    else if(strcmp(sLanguageName, "Spanish") == 0)
       Device::Language = SystemLanguage::Spanish;
    else if(strcmp(sLanguageName, "German") == 0)
       Device::Language = SystemLanguage::German;

    // class properties
    WNDCLASSEX wndClass;
    wndClass.cbSize = sizeof(wndClass);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = GE_APP_NAME;
    wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // register the class
    RegisterClassExA(&wndClass);

    // create the main window
    GE::uint iWindowPositionX = (iFullscreenWidth / 2) - (iWindowWidth / 2);
    GE::uint iWindowPositionY = (iFullscreenHeight / 2) - (iWindowHeight / 2);

    hWnd = CreateWindowExA(NULL, GE_APP_NAME, GE_APP_NAME, WS_CAPTION | WS_SYSMENU, iWindowPositionX, iWindowPositionY, 
                           iWindowWidth, iWindowHeight, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);

    // initialize rendering and sound systems
    cRender = Allocator::alloc<RenderSystemDX11>();
#if defined (GE_FULLSCREEN_MODE)
    GEInvokeCtor(RenderSystemDX11, cRender)(hWnd, false);
#else
    GEInvokeCtor(RenderSystemDX11, cRender)(hWnd, true);
#endif
    cAudio = Allocator::alloc<AudioSystemFMOD>();
    GEInvokeCtor(AudioSystemFMOD, cAudio);
    cAudio->init();
    cAudio->setListenerPosition(GE::Vector3(0.0f, 0.0f, 0.0f));

    // initialize the application module
    StateManager cStateManager;
    initAppModule();

#ifndef _DEBUG
    // hide the mouse pointer
    ShowCursor(false);
#endif

    // timer
    Timer cTimer;
    cTimer.start();
    Time::reset();

    bEnd = false;

    double dTimeInterval = 1000000.0 / GE_FPS;
    double dTimeDelta = 0.0;
    double dTimeBefore = 0.0;
    double dTimeNow;

    MSG iMsg;

#if defined (GE_MOUSE_INFINITY)
    bool bMousePositionSet;
#endif

    // create task manager
    TaskManager* cTaskManager = Allocator::alloc<TaskManager>();
    GEInvokeCtor(TaskManager, cTaskManager);

    // game loop
    while(!bEnd)
    {
        // input
        while(PeekMessage(&iMsg, NULL, 0, 0, PM_REMOVE)) 
        {
            if(iMsg.message == WM_CLOSE || iMsg.message == WM_QUIT) 
            {
                bEnd = true;
            }
            else 
            {
                TranslateMessage(&iMsg);
                DispatchMessage(&iMsg);
            }
        }

        GetCursorPos(&pMouse);

#if defined (GE_MOUSE_INFINITY)
        bMousePositionSet = false;

        if(pMouse.x < GE_MOUSE_CHECK_MARGIN)
        {
            pMouse.x = iFullscreenWidth - GE_MOUSE_SET_MARGIN;
            bMousePositionSet = true;
        }
        else if(pMouse.x >= (iFullscreenWidth - GE_MOUSE_CHECK_MARGIN))
        {
            pMouse.x = GE_MOUSE_SET_MARGIN;
            bMousePositionSet = true;
        }
        if(pMouse.y < GE_MOUSE_CHECK_MARGIN)
        {
            pMouse.y = iFullscreenHeight - GE_MOUSE_SET_MARGIN;
            bMousePositionSet = true;
        }
        else if(pMouse.y >= (iFullscreenHeight - GE_MOUSE_CHECK_MARGIN))
        {
            pMouse.y = GE_MOUSE_SET_MARGIN;
            bMousePositionSet = true;
        }          

        if(bMousePositionSet)
        {
            SetCursorPos(pMouse.x, pMouse.y);
            InputSystem::getInstance()->inputMouse(pMouse.x, pMouse.y);
        }
#endif

        GE::Vector2 vMouseCurrentPosition = GetMouseScreenPosition();

        if(fabs(vMouseCurrentPosition.X - vMouseLastPosition.X) > GE_EPSILON ||
           fabs(vMouseCurrentPosition.Y - vMouseLastPosition.Y) > GE_EPSILON)
        {
           InputSystem::getInstance()->inputMouse(vMouseCurrentPosition);

           if(bMouseLeftButton)
           {
              InputSystem::getInstance()->inputTouchMove(0, vMouseLastPosition, vMouseCurrentPosition);
           }

           vMouseLastPosition = vMouseCurrentPosition;
        }

        // update and render
        dTimeNow = cTimer.getTime();
        dTimeDelta = dTimeNow - dTimeBefore;

        if(dTimeDelta >= dTimeInterval)
        {
            GEProfilerFrame("MainThread");

            dTimeBefore = dTimeNow;

            float fTimeDelta = (float)dTimeDelta * 0.000001f;

            if(fTimeDelta > 1.0f)
               fTimeDelta = 1.0f / GE_FPS;

            Time::setDelta(fTimeDelta);

            TaskManager::getInstance()->update();
            TaskManager::getInstance()->render();

            bEnd = bEnd || TaskManager::getInstance()->getExitPending();
        }
    }

    cStateManager.getActiveState()->deactivate();
    cStateManager.releaseStates();

    cAudio->release();
    
    GEInvokeDtor(AudioSystem, cAudio);
    Allocator::free(cAudio);
    GEInvokeDtor(RenderSystem, cRender);
    Allocator::free(cRender);
    GEInvokeDtor(TaskManager, cTaskManager);
    Allocator::free(cTaskManager);

    GEInvokeDtor(Scaler, cPixelToScreenX);
    Allocator::free(cPixelToScreenX);
    GEInvokeDtor(Scaler, cPixelToScreenY);
    Allocator::free(cPixelToScreenY);

    Application::shutDown();

    return (int)iMsg.wParam;
}

GE::Vector2 GetMouseScreenPosition()
{
   POINT pMouseClient = pMouse;
   ScreenToClient(hWnd, &pMouseClient);

   return GE::Vector2(
      cPixelToScreenX->y((float)pMouseClient.x),
      cPixelToScreenY->y((float)pMouseClient.y));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
   switch(iMsg)
   {
   case WM_CLOSE:
      bEnd = true;
      return 0;

   case WM_KEYDOWN:
      InputSystem::getInstance()->inputKeyPress((char)wParam);
      return 0;

   case WM_KEYUP:
      InputSystem::getInstance()->inputKeyRelease((char)wParam);
      return 0;

   case WM_LBUTTONDOWN:
      bMouseLeftButton = true;
      vMouseLastPosition = GetMouseScreenPosition();
      InputSystem::getInstance()->inputMouseLeftButton();
      InputSystem::getInstance()->inputTouchBegin(0, vMouseLastPosition);
#if !defined (GE_FULLSCREEN_MODE)
      SetCapture(hWnd);
#endif
      return 0;

   case WM_LBUTTONUP:
      bMouseLeftButton = false;
      vMouseLastPosition = GetMouseScreenPosition();
      InputSystem::getInstance()->inputTouchEnd(0, vMouseLastPosition);
#if !defined (GE_FULLSCREEN_MODE)
      ReleaseCapture();
#endif
      return 0;

   case WM_RBUTTONDOWN:
      InputSystem::getInstance()->inputMouseRightButton();
      return 0;

   case WM_MOUSEWHEEL:
      InputSystem::getInstance()->inputMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
      return 0;

#if !defined (GE_FULLSCREEN_MODE)
   case WM_MOUSEMOVE:
      {
         RECT rWindow;
         GetWindowRect(hWnd, &rWindow);

         if(!PtInRect(&rWindow, pMouse))
            PostMessage(hWnd, WM_LBUTTONUP, 0, 0);
      }
      return 0;
#endif
   }
    
   return DefWindowProc(hWnd, iMsg, wParam, lParam);
}
