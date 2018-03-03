
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Windows Phone 8
//
//  --- AppMain.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "AppMain.h"
#include <ppltasks.h>
#include <csignal>

#include "Core/GEDevice.h"
#include "Core/GETime.h"
#include "Core/GEApplication.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::Devices::Sensors;

using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;

AppMain::AppMain()
   : cRender(nullptr)
   , cAudio(nullptr)
   , cTaskManager(nullptr)
   , bWindowClosed(false)
   , bWindowVisible(true)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
      iFingerID[i] = -1;
}

void AppMain::Initialize(CoreApplicationView^ applicationView)
{
#ifndef NDEBUG
   // break whenever an assertion fails
   signal(SIGABRT, [](int){ __debugbreak(); }); 
#endif

   Application::Name = GE_APP_NAME;
   Application::ID = GE_APP_ID;
   Application::VersionString = GE_VERSION_STRING;
   Application::VersionNumber = GE_VERSION_NUMBER;

   Application::startUp();

   applicationView->Activated +=
      ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &AppMain::OnActivated);
   CoreApplication::Suspending +=
      ref new EventHandler<SuspendingEventArgs^>(this, &AppMain::OnSuspending);
   CoreApplication::Resuming +=
      ref new EventHandler<Platform::Object^>(this, &AppMain::OnResuming);
}

int convertDipsToPixels(float dips)
{
   static const float dipsPerInch = 96.0f;
   return (int)floor(dips * Windows::Graphics::Display::DisplayProperties::LogicalDpi / dipsPerInch + 0.5f);
}

void AppMain::SetWindow(CoreWindow^ window)
{
   // register callbacks
   window->VisibilityChanged +=
      ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &AppMain::OnVisibilityChanged);
   window->Closed += 
      ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &AppMain::OnWindowClosed);
   window->PointerPressed +=
      ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &AppMain::OnPointerPressed);
   window->PointerMoved +=
      ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &AppMain::OnPointerMoved);
   window->PointerReleased +=
      ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &AppMain::OnPointerReleased);

   // screen settings
#ifdef GE_ORIENTATION_PORTRAIT
   Device::ScreenWidth = convertDipsToPixels(window->Bounds.Width);
   Device::ScreenHeight = convertDipsToPixels(window->Bounds.Height);
   Device::Orientation = DeviceOrientation::Portrait;

   cPixelToScreenX = new Scaler(0.0f, (float)Device::ScreenWidth, -1.0f, 1.0f);
   cPixelToScreenY = new Scaler(0.0f, (float)Device::ScreenHeight, Device::getAspectRatio(), -Device::getAspectRatio());

   DisplayProperties::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Portrait;
#else
   Device::ScreenWidth = convertDipsToPixels(window->Bounds.Height);
   Device::ScreenHeight = convertDipsToPixels(window->Bounds.Width);
   Device::Orientation = DeviceOrientation::Landscape;

   cPixelToScreenX = new Scaler(0.0f, (float)Device::ScreenHeight, -Device::getAspectRatio(), Device::getAspectRatio());
   cPixelToScreenY = new Scaler(0.0f, (float)Device::ScreenWidth, 1.0f, -1.0f);

   DisplayProperties::AutoRotationPreferences = Windows::Graphics::Display::DisplayOrientations::Landscape;
#endif

   // create render system
   cRender = new RenderSystemDX11(CoreWindow::GetForCurrentThread());
   cRender->setBackgroundColor(GE::Color(0.5f, 0.5f, 1.0f));

   // create audio system
   cAudio = new AudioSystemXAudio2();
   cAudio->init();

   // initialize the application module
   initAppModule();

   // create task manager
   cTaskManager = new TaskManager();

#ifdef GE_USE_ACCELEROMETER
   // initialize accelerometer
   wpAccelerometer = Accelerometer::GetDefault();
   wpAccelerometer->ReportInterval = (unsigned int)(GE_ACCELEROMETER_UPDATE * 1000.0f);
   wpAccelerometer->ReadingChanged +=
      ref new TypedEventHandler<Accelerometer^, AccelerometerReadingChangedEventArgs^>(this, &AppMain::OnAccelerometerReading);
#endif
}

void AppMain::Load(Platform::String^ entryPoint)
{
}

void AppMain::Run()
{
   cTimer.start();
   Time::reset();

   double dCurrentTime = cTimer.getTime();
   double dPreviousTime = dCurrentTime;
   double dDeltaTime = 0.0;
   double dTotalTime = 0.0;

   while(!bWindowClosed)
   {
      if(bWindowVisible)
      {
         CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

         dCurrentTime = cTimer.getTime();
         dDeltaTime = dCurrentTime - dPreviousTime;
         dTotalTime += dDeltaTime;
         dPreviousTime = dCurrentTime;

         Time::setDelta((float)dDeltaTime * 0.000001f);

         TaskManager::getInstance()->update();
         TaskManager::getInstance()->render();
      }
      else
      {
         CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
      }
   }
}

void AppMain::Uninitialize()
{
}

void AppMain::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
   bWindowVisible = args->Visible;
}

void AppMain::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
   bWindowClosed = true;
}

GE::Vector2 AppMain::PixelToScreen(const GE::Vector2& vPixelPosition)
{
   return Device::Orientation == DeviceOrientation::Portrait
      ? GE::Vector2((float)cPixelToScreenX->y(vPixelPosition.X), (float)cPixelToScreenY->y(vPixelPosition.Y))
      : GE::Vector2((float)-cPixelToScreenY->y(vPixelPosition.Y), (float)cPixelToScreenX->y(vPixelPosition.X));
}

void AppMain::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == -1)
      {
         iFingerID[i] = (int)args->CurrentPoint->PointerId;
         vFingerPosition[i] = PixelToScreen(GE::Vector2(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y));
         cStateManager.getActiveState()->inputTouchBegin(i, vFingerPosition[i]);
         break;
      }
   }
}

void AppMain::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == (int)args->CurrentPoint->PointerId)
      {
         GE::Vector2 vPreviousPosition = vFingerPosition[i];
         vFingerPosition[i] = PixelToScreen(GE::Vector2(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y));
         cStateManager.getActiveState()->inputTouchMove(i, vPreviousPosition, vFingerPosition[i]);
         break;
      }
   }
}

void AppMain::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == (int)args->CurrentPoint->PointerId)
      {
         iFingerID[i] = -1;
         vFingerPosition[i] = PixelToScreen(GE::Vector2(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y));
         cStateManager.getActiveState()->inputTouchEnd(i, vFingerPosition[i]);
         break;
      }
   }
}

void AppMain::OnAccelerometerReading(Accelerometer^ accelerometer, AccelerometerReadingChangedEventArgs^ args)
{
   cStateManager.getActiveState()->updateAccelerometerStatus(
      GE::Vector3((float)args->Reading->AccelerationX, (float)args->Reading->AccelerationY, (float)args->Reading->AccelerationZ));
}

void AppMain::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
   CoreWindow::GetForCurrentThread()->Activate();
}

void AppMain::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
   SuspendingDeferral^ rcDeferral = args->SuspendingOperation->GetDeferral();
   RenderSystemDX11Helper::releaseResourcesForSuspending();

   Concurrency::create_task([this, rcDeferral]()
   {
      rcDeferral->Complete();
   });
}
 
void AppMain::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
   RenderSystemDX11Helper::createWindowSizeDependentResources();
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
   return ref new AppMain();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
   auto dxApplicationSource = ref new Direct3DApplicationSource();
   CoreApplication::Run(dxApplicationSource);
   return 0;
}
