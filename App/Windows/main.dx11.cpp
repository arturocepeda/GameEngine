
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

#include "main.dx11.h"
#include "config.h"

#include "Core/GEDevice.h"
#include "Core/GETimer.h"
#include "Core/GETime.h"
#include "Core/GEStateManager.h"
#include "Core/GETaskManager.h"
#include "Core/GEAllocator.h"
#include "Core/GEApplication.h"
#include "Core/GELog.h"
#include "Core/GEProfiler.h"
#include "Core/GEDistributionPlatform.h"

#include "Rendering/DX11/GERenderSystemDX11.h"
#include "Audio/GEAudioSystem.h"
#include "Input/GEInputSystem.h"
#include "Input/GEGamepad.XInput.h"

#pragma comment(lib, "GameEngine.DX11.lib")
#pragma comment(lib, "AppModule.lib")

#if defined (_M_X64)
# pragma comment(lib, "../../../GameEngine/Externals/OpenAL/lib/Win64/OpenAL32.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libogg/lib/x64/libogg_static.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libvorbis/lib/x64/libvorbis_static.lib")
# pragma comment(lib, "../../../GameEngine/Externals/libvorbis/lib/x64/libvorbisfile_static.lib")
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
AppSettings gSettings;              // settings
bool bEnd;                          // loop ending flag

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR sCmdLine, int iCmdShow)
{
   Application::Name = GE_APP_NAME;
   Application::ID = GE_APP_ID;
   Application::VersionString = GE_VERSION_STRING;
   Application::VersionNumber = GE_VERSION_NUMBER;
    
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
   GEInvokeCtor(Scaler, cPixelToScreenY)
      (0.0f, (float)Device::ScreenHeight, (float)Device::getAspectRatio(), (float)-Device::getAspectRatio());

   int iWindowWidth = Device::ScreenWidth;
   int iWindowHeight = Device::ScreenHeight;

   if(!gSettings.getFullscreen())
   {
      int iWindowBorderSize = 8;
      int iWindowCaptionSize = GetSystemMetrics(SM_CYSMCAPTION);

      iWindowWidth += (iWindowBorderSize * 2);
      iWindowHeight += iWindowCaptionSize + (iWindowBorderSize * 2);
   }

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
   GE::uint iWindowPositionX = (systemScreenWidth / 2) - (iWindowWidth / 2);
   GE::uint iWindowPositionY = (systemScreenHeight / 2) - (iWindowHeight / 2);

   const DWORD wndStyle = gSettings.getFullscreen()
      ? WS_EX_TOPMOST | WS_POPUP
      : WS_CAPTION | WS_SYSMENU;

   hWnd =
      CreateWindowExA(NULL, GE_APP_NAME, GE_APP_NAME, wndStyle,
         iWindowPositionX, iWindowPositionY, 
         iWindowWidth, iWindowHeight,
         NULL, NULL, hInstance, NULL);
   ShowWindow(hWnd, iCmdShow);
   UpdateWindow(hWnd);

   // initialize rendering and sound systems
   cRender = Allocator::alloc<RenderSystemDX11>();
   GEInvokeCtor(RenderSystemDX11, cRender)(hWnd, !gSettings.getFullscreen());
   cAudio = Allocator::alloc<AudioSystem>();
   GEInvokeCtor(AudioSystem, cAudio);
   cAudio->init();
   cAudio->setListenerPosition(GE::Vector3(0.0f, 0.0f, 0.0f));

#ifndef _DEBUG
   // hide the mouse pointer
   ShowCursor(false);
#endif

   // timer
   Timer cTimer;
   cTimer.start();
   Time::reset();

   bEnd = false;

   double dTimeInterval = 1000000.0 / gSettings.getTargetFPS();
   double dTimeDelta = 0.0;
   double dTimeBefore = 0.0;
   double dTimeNow;

   MSG iMsg;

   // create task manager
   TaskManager* cTaskManager = Allocator::alloc<TaskManager>();
   GEInvokeCtor(TaskManager, cTaskManager);

   // game loop
   bool vsync = gSettings.getVSync();

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
      if(gSettings.getVSync() != vsync)
      {
         vsync = gSettings.getVSync();

         dTimeDelta = 0.0;
         dTimeBefore = 0.0;
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
         GEProfilerFrame("MainThread");

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

         bEnd |= TaskManager::getInstance()->getExitPending();
      }
   }

   cStateManager.getActiveState()->deactivate();
   cStateManager.releaseStates();

   GEInvokeDtor(Scaler, cPixelToScreenX);
   Allocator::free(cPixelToScreenX);
   GEInvokeDtor(Scaler, cPixelToScreenY);
   Allocator::free(cPixelToScreenY);

   Application::shutDown();

   distributionPlatform.shutdown();

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
      {
         const bool keyRepeat = (lParam & 0x40000000) > 0;

         if(!keyRepeat)
         {
            InputSystem::getInstance()->inputKeyPress((char)wParam);
         }
      }

      return 0;

   case WM_KEYUP:
      InputSystem::getInstance()->inputKeyRelease((char)wParam);
      return 0;

   case WM_CHAR:
      InputSystem::getInstance()->inputKeyText((uint16_t)wParam);
      return 0;

   case WM_LBUTTONDOWN:
      bMouseLeftButton = true;
      vMouseLastPosition = GetMouseScreenPosition();
      InputSystem::getInstance()->inputMouseLeftButton();
      InputSystem::getInstance()->inputTouchBegin(0, vMouseLastPosition);

      if(!gSettings.getFullscreen())
      {
         SetCapture(hWnd);
      }

      return 0;

   case WM_LBUTTONUP:
      bMouseLeftButton = false;
      vMouseLastPosition = GetMouseScreenPosition();
      InputSystem::getInstance()->inputTouchEnd(0, vMouseLastPosition);

      if(!gSettings.getFullscreen())
      {
         ReleaseCapture();
      }

      return 0;

   case WM_RBUTTONDOWN:
      InputSystem::getInstance()->inputMouseRightButton();
      return 0;

   case WM_MOUSEWHEEL:
      InputSystem::getInstance()->inputMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
      return 0;

   case WM_MOUSEMOVE:
      if(!gSettings.getFullscreen())
      {
         RECT rWindow;
         GetWindowRect(hWnd, &rWindow);

         if(!PtInRect(&rWindow, pMouse))
         {
            PostMessage(hWnd, WM_LBUTTONUP, 0, 0);
         }
      }

      return 0;
   }
    
   return DefWindowProc(hWnd, iMsg, wParam, lParam);
}
