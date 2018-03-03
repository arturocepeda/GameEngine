
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Android
//
//  --- main.cpp ---
//
//////////////////////////////////////////////////////////////////


#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>

#include "android_native_app_glue.h"
#include "cpu-features.h"

#include "config.h"
#include "Rendering/OpenGL/GERenderSystemES20.h"
#include "Audio/OpenSL/GEAudioSystemOpenSL.h"
#include "Core/GEDevice.h"
#include "Core/GEStateManager.h"
#include "Core/GETaskManager.h"
#include "Core/GETimer.h"
#include "Core/GETime.h"
#include "Core/GEApplication.h"



using namespace GE;
using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Content;


//
//  Android objects
//
struct android_app* aApp = 0;
int aSDKVersion;

EGLDisplay eglDisplay = EGL_NO_DISPLAY;
EGLSurface eglSurface = EGL_NO_SURFACE;
EGLContext eglContext = EGL_NO_CONTEXT;
EGLConfig eglConfig = 0;

#if defined (GE_USE_ACCELEROMETER)
ASensorManager* aSensorManager = 0;
const ASensor* aSensorAccelerometer = 0;
ASensorEventQueue* aSensorEventQueue = 0;
#endif


//
//  Game Engine objects
//
RenderSystem* cRender = 0;
AudioSystem* cAudio = 0;
TaskManager* cTaskManager = 0;

bool bInitialized = false;
bool bPaused = false;

StateManager cStateManager;
Timer cTimer;
double dTime;

int iFingerID[GE_MAX_FINGERS];
Vector2 vFingerPosition[GE_MAX_FINGERS];

Scaler* cPixelToScreenX = 0;
Scaler* cPixelToScreenY = 0;



int appInitDisplay()
{
   const EGLint eglChooseConfigAttribs[] =
   {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_BLUE_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_RED_SIZE, 8,
      EGL_DEPTH_SIZE, 16,
      EGL_NONE
   };

   EGLint w, h, format;
   EGLint numConfigs;

   eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

   eglInitialize(eglDisplay, 0, 0);
   eglChooseConfig(eglDisplay, eglChooseConfigAttribs, &eglConfig, 1, &numConfigs);
   eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &format);

   ANativeWindow_setBuffersGeometry(aApp->window, 0, 0, format);

   eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, aApp->window, NULL);

   EGLint eglCreateContextAttribs[] = 
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, eglCreateContextAttribs);
   eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

   eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &w);
   eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &h);

   Device::ScreenWidth = w;
   Device::ScreenHeight = h;

   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

   return 0;
}

void appDrawFrame()
{
   if(!eglDisplay)
      return;

   cTaskManager->update();
   cTaskManager->render();

   eglSwapBuffers(eglDisplay, eglSurface);
}

void appShutDownDisplay()
{
   eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroySurface(eglDisplay, eglSurface);
   eglSurface = EGL_NO_SURFACE;
}

void appSaveState()
{
}

void appLoadState()
{
}

void onInitWindow()
{
   if(!aApp->window)
      return;

   if(bInitialized)
   {
      eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, aApp->window, NULL);
      eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
      return;
   }

   Application::Name = GE_APP_NAME;
   Application::ID = GE_APP_ID;
   Application::VersionString = GE_VERSION_STRING;
   Application::VersionNumber = GE_VERSION_NUMBER;

#if defined (GE_BINARY_CONTENT)
   Application::ContentType = ApplicationContentType::Bin;
#endif

   Application::startUp();

   // initialize OpenGL
   appInitDisplay();

   // device orientation
#ifdef GE_ORIENTATION_PORTRAIT
   Device::Orientation = DeviceOrientation::Portrait;
#else
   Device::Orientation = DeviceOrientation::Landscape;
#endif

   // system language
   JNIEnv* env = aApp->activity->env;
   aApp->activity->vm->AttachCurrentThread(&env, NULL);

   jclass localeClass = env->FindClass("java/util/Locale");
   jmethodID localeDefaultMethod = env->GetStaticMethodID(localeClass, "getDefault", "()Ljava/util/Locale;");
   jmethodID localeGetLanguageMethod = env->GetMethodID(localeClass, "getLanguage", "()Ljava/lang/String;");
   jobject localeInstance = env->CallStaticObjectMethod(localeClass, localeDefaultMethod);
   jstring languageStringObj = (jstring)env->CallObjectMethod(localeInstance, localeGetLanguageMethod);

   const char* sLanguage = env->GetStringUTFChars(languageStringObj, 0);
   aApp->activity->vm->DetachCurrentThread();

   if(strcmp(sLanguage, "en") == 0)
      Device::Language = SystemLanguage::English;
   else if(strcmp(sLanguage, "es") == 0)
      Device::Language = SystemLanguage::Spanish;
   else if(strcmp(sLanguage, "de") == 0)
      Device::Language = SystemLanguage::German;

   // IDs for touch management
   for(int i = 0; i < GE_MAX_FINGERS; i++)
      iFingerID[i] = -1;

   cPixelToScreenX = new Scaler(0.0f, Device::ScreenWidth, -1.0f, 1.0f);
   cPixelToScreenY = new Scaler(0.0f, Device::ScreenHeight, Device::getAspectRatio(), -Device::getAspectRatio());

   // initialize rendering system
   cRender = new RenderSystemES20();

   // initialize audio system
   if(aSDKVersion >= 17)
   {
      aApp->activity->vm->AttachCurrentThread(&env, NULL);

      jclass contextClass = env->FindClass("android/content/Context");
      jfieldID audioServiceField = env->GetStaticFieldID(contextClass, "AUDIO_SERVICE", "Ljava/lang/String;");
      jstring audioServiceFieldValue = (jstring)env->GetStaticObjectField(contextClass, audioServiceField);
      jmethodID getSystemServiceIDMethod = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
      jobject audioManagerInstance = env->CallObjectMethod(aApp->activity->clazz, getSystemServiceIDMethod, audioServiceFieldValue);

      jclass audioManagerClass = env->FindClass("android/media/AudioManager");
      jmethodID getPropertyMethod = env->GetMethodID(audioManagerClass, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");

      jstring jPropertyValue =
         (jstring)env->CallObjectMethod(audioManagerInstance, getPropertyMethod, env->NewStringUTF("android.media.property.OUTPUT_SAMPLE_RATE"));
      const char* sPropertyValue = env->GetStringUTFChars(jPropertyValue, 0);
      Device::AudioSystemSampleRate = Parser::parseInt(sPropertyValue);

      jPropertyValue =
         (jstring)env->CallObjectMethod(audioManagerInstance, getPropertyMethod, env->NewStringUTF("android.media.property.OUTPUT_FRAMES_PER_BUFFER"));
      sPropertyValue = env->GetStringUTFChars(jPropertyValue, 0);
      Device::AudioSystemFramesPerBuffer = Parser::parseInt(sPropertyValue);

      aApp->activity->vm->DetachCurrentThread();
   }

#if defined (GE_AUDIO_SUPPORT)
   cAudio = new AudioSystemOpenSL();
   cAudio->init();
#endif

   // initialize the application module
   initAppModule();

   // start the timer
   cTimer.start();
   dTime = 0.0;
   Time::reset();

   // create task manager
   cTaskManager = new TaskManager();

   // draw the first frame
   appDrawFrame();

   bInitialized = true;
}

void onPause()
{
#if defined (GE_USE_ACCELEROMETER)
   if(aSensorAccelerometer)
      ASensorEventQueue_disableSensor(aSensorEventQueue, aSensorAccelerometer);
#endif

   appDrawFrame();

   bPaused = true;
   cTimer.stop();
   dTime = 0.0;
   cStateManager.getActiveState()->pause();
}

void onResume()
{
   if(!bInitialized || !bPaused)
      return;

   bPaused = false;
   cTimer.start();
   cStateManager.getActiveState()->resume();

#if defined (GE_USE_ACCELEROMETER)
   if(aSensorAccelerometer)
   {
      ASensorEventQueue_enableSensor(aSensorEventQueue, aSensorAccelerometer);
      ASensorEventQueue_setEventRate(aSensorEventQueue, aSensorAccelerometer, (1000L / GE_FPS) * 1000);
   }
#endif
}

void onLowMemory()
{
}

void appHandleCmd(struct android_app* app, int32_t cmd)
{
   switch(cmd)
   {
   case APP_CMD_SAVE_STATE:
      appSaveState();
      break;
   case APP_CMD_INIT_WINDOW:
      onInitWindow();
      break;
   case APP_CMD_TERM_WINDOW:
      appShutDownDisplay();
      break;
   case APP_CMD_PAUSE:
      onPause();
      break;
   case APP_CMD_RESUME:
      onResume();
      break;
   case APP_CMD_LOW_MEMORY:
      onLowMemory();
      break;
   }
}

void enableImmersiveMode()
{
   JNIEnv* env;
   aApp->activity->vm->AttachCurrentThread(&env, NULL);

   jclass activityClass = env->FindClass("android/app/NativeActivity");
   jmethodID getWindowMethod = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");

   jclass windowClass = env->FindClass("android/view/Window");
   jmethodID getDecorViewMethod = env->GetMethodID(windowClass, "getDecorView", "()Landroid/view/View;");

   jclass viewClass = env->FindClass("android/view/View");
   jmethodID setSystemUiVisibilityMethod = env->GetMethodID(viewClass, "setSystemUiVisibility", "(I)V");

   jobject window = env->CallObjectMethod(aApp->activity->clazz, getWindowMethod);
   jobject decorView = env->CallObjectMethod(window, getDecorViewMethod);

   const int SystemUIFlagsCount = 6;
   const char* SystemUIFlags[SystemUIFlagsCount] =
   {
      "SYSTEM_UI_FLAG_HIDE_NAVIGATION",
      "SYSTEM_UI_FLAG_FULLSCREEN",
      "SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION",
      "SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN",
      "SYSTEM_UI_FLAG_IMMERSIVE",
      "SYSTEM_UI_FLAG_IMMERSIVE_STICKY"
   };

   int uiFlags = 0;

   for(int i = 0; i < SystemUIFlagsCount; i++)
   {
      jfieldID flagField = env->GetStaticFieldID(viewClass, SystemUIFlags[i], "I");
      uiFlags |= env->GetStaticIntField(viewClass, flagField);
   }

   env->CallVoidMethod(decorView, setSystemUiVisibilityMethod, uiFlags);
}

void appHandleCmdUIThread(struct android_app* app, int32_t cmd)
{
   if(cmd == APP_CMD_RESUME && aSDKVersion >= 19)
      enableImmersiveMode();
}

GE::Vector2 pixelToScreen(const GE::Vector2& vPixelPosition)
{
   return GE::Vector2((float)cPixelToScreenX->y(vPixelPosition.X), (float)cPixelToScreenY->y(vPixelPosition.Y));
}

void onInputTouchDown(int index, float x, float y)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == -1)
      {
         iFingerID[i] = index;
         vFingerPosition[i] = pixelToScreen(Vector2(x, y));
         cStateManager.getActiveState()->inputTouchBegin(i, vFingerPosition[i]);
         break;
      }
   }
}

void onInputTouchMove(int index, float x, float y)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == index)
      {
         Vector2 vPreviousPosition = vFingerPosition[i];
         vFingerPosition[i] = pixelToScreen(Vector2(x, y));
         cStateManager.getActiveState()->inputTouchMove(i, vPreviousPosition, vFingerPosition[i]);
         break;
      }
   }
}

void onInputTouchUp(int index, float x, float y)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == index)
      {
         iFingerID[i] = -1;
         vFingerPosition[i] = pixelToScreen(Vector2(x, y));
         cStateManager.getActiveState()->inputTouchEnd(i, vFingerPosition[i]);
         break;
      }
   }
}

int32_t appHandleInput(struct android_app* app, AInputEvent* event)
{
   if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
   {
      int32_t iAction = AMotionEvent_getAction(event);
      int32_t iActionType = iAction & AMOTION_EVENT_ACTION_MASK;
      int32_t iActionPointerIndex = iAction >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

      switch(iActionType)
      {
         case AMOTION_EVENT_ACTION_DOWN:
         case AMOTION_EVENT_ACTION_POINTER_DOWN:
            {
               onInputTouchDown(
                  AMotionEvent_getPointerId(event, iActionPointerIndex),
                  AMotionEvent_getX(event, iActionPointerIndex),
                  AMotionEvent_getY(event, iActionPointerIndex));
            }
            break;

         case AMOTION_EVENT_ACTION_MOVE:
            {
               size_t iNumPointers = AMotionEvent_getPointerCount(event);

               for(size_t iPointerIndex = 0; iPointerIndex < iNumPointers; iPointerIndex++)
               {
                  onInputTouchMove(
                     AMotionEvent_getPointerId(event, iPointerIndex),
                     AMotionEvent_getX(event, iPointerIndex),
                     AMotionEvent_getY(event, iPointerIndex));
               }
            }
            break;

         case AMOTION_EVENT_ACTION_UP:
         case AMOTION_EVENT_ACTION_POINTER_UP:
         case AMOTION_EVENT_ACTION_CANCEL:
            {
               onInputTouchUp(
                  AMotionEvent_getPointerId(event, iActionPointerIndex),
                  AMotionEvent_getX(event, iActionPointerIndex),
                  AMotionEvent_getY(event, iActionPointerIndex));
            }
            break;
      }

      return 1;
   }

   return 0;
}

void android_main(struct android_app* state)
{
   state->onAppCmd = appHandleCmd;
   state->onAppCmdUIThread = appHandleCmdUIThread;
   state->onInputEvent = appHandleInput;
   aApp = state;

   JNIEnv* env = aApp->activity->env;
   aApp->activity->vm->AttachCurrentThread(&env, NULL);

   jclass buildVersionClass = env->FindClass("android/os/Build$VERSION");
   jfieldID sdkIntField = env->GetStaticFieldID(buildVersionClass, "SDK_INT", "I");
   aSDKVersion = env->GetStaticIntField(buildVersionClass, sdkIntField);

   aApp->activity->vm->DetachCurrentThread();

#if defined (GE_USE_ACCELEROMETER)
   aSensorManager = ASensorManager_getInstance();
   aSensorAccelerometer = ASensorManager_getDefaultSensor(aSensorManager, ASENSOR_TYPE_ACCELEROMETER);

   if(aSensorAccelerometer)
   {
      aSensorEventQueue = ASensorManager_createEventQueue(aSensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);
      ASensorEventQueue_enableSensor(aSensorEventQueue, aSensorAccelerometer);
      ASensorEventQueue_setEventRate(aSensorEventQueue, aSensorAccelerometer, (1000L / GE_FPS) * 1000);
   }
#endif

   if(state->savedState)
      appLoadState();

   while(1)
   {
      int ident;
      int events;
      struct android_poll_source* source;

      while((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
      {
         if(source)
            source->process(state, source);

#if defined (GE_USE_ACCELEROMETER)
         if(ident == LOOPER_ID_USER)
         {
            if(aSensorAccelerometer)
            {
               const float AccelFactor = 0.01f;
               ASensorEvent event;

               while(ASensorEventQueue_getEvents(aSensorEventQueue, &event, 1) > 0)
               {
                  if(bInitialized)
                  {
                     cStateManager.getActiveState()->updateAccelerometerStatus(Vector3(
                        event.acceleration.x * -AccelFactor,
                        event.acceleration.y * -AccelFactor,
                        event.acceleration.z * AccelFactor));
                     Device::log("Acc: %.2f, %.2f, %.2f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                  }
               }
            }
         }
#endif

         if(state->destroyRequested != 0)
         {
            appShutDownDisplay();
            return;
         }
      }

      if(bPaused)
         continue;

      double dCurrentTime = cTimer.getTime();
      Time::setDelta((dCurrentTime - dTime) * 0.000001f);
      dTime = dCurrentTime;

      appDrawFrame();
   }
}


//
//  GE::Core::Device methods
//
uint Device::getFileLength(const char* Filename)
{
   AAssetManager* aAssetManager = aApp->activity->assetManager;
   AAsset* aAsset = AAssetManager_open(aAssetManager, Filename, AASSET_MODE_UNKNOWN);

   if(aAsset)
   {
      int iReadBytes = AAsset_getLength(aAsset);
      AAsset_close(aAsset);
      return iReadBytes;
   }

   return 0;
}

uint Device::readFile(const char* Filename, unsigned char* ReadBuffer, uint BufferSize)
{
   AAssetManager* aAssetManager = aApp->activity->assetManager;
   AAsset* aAsset = AAssetManager_open(aAssetManager, Filename, AASSET_MODE_UNKNOWN);

   if(aAsset)
   {
      int iReadBytes = AAsset_read(aAsset, ReadBuffer, BufferSize);
      AAsset_close(aAsset);
      return iReadBytes;
   }

   return 0;
}

uint Device::getContentFilesCount(const char* SubDir, const char* Extension)
{
   AAssetManager* aAssetManager = aApp->activity->assetManager;
   AAssetDir* aAssetDir = AAssetManager_openDir(aAssetManager, SubDir);

   if(!aAssetDir)
      return 0;

   uint iExtensionLength = strlen(Extension);
   uint iFilesCount = 0;
   const char* sFileName = 0;

   do
   {
      sFileName = AAssetDir_getNextFileName(aAssetDir);

      if(sFileName)
      {
         uint iFileNameLength = strlen(sFileName);

         if(iFileNameLength < iExtensionLength)
            continue;

         bool bHasExtension = true;
         uint iCharIndex = iFileNameLength - iExtensionLength;

         for(uint i = 0; i < iExtensionLength; i++, iCharIndex++)
         {
            if(sFileName[iCharIndex] != Extension[i])
            {
               bHasExtension = false;
               break;
            }
         }

         if(bHasExtension)
            iFilesCount++;
      }
   }
   while(sFileName);

   AAssetDir_close(aAssetDir);

   return iFilesCount;
}

void Device::getContentFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   AAssetManager* aAssetManager = aApp->activity->assetManager;
   AAssetDir* aAssetDir = AAssetManager_openDir(aAssetManager, SubDir);

   if(!aAssetDir)
      return;

   uint iExtensionLength = strlen(Extension);
   uint iFileIndex = 0;
   const char* sFileName = 0;

   do
   {
      sFileName = AAssetDir_getNextFileName(aAssetDir);

      if(sFileName)
      {
         uint iFileNameLength = strlen(sFileName);

         if(iFileNameLength < iExtensionLength)
            continue;

         bool bHasExtension = true;
         uint iCharIndex = iFileNameLength - iExtensionLength;

         for(uint i = 0; i < iExtensionLength; i++, iCharIndex++)
         {
            if(sFileName[iCharIndex] != Extension[i])
            {
               bHasExtension = false;
               break;
            }
         }

         if(bHasExtension)
         {
            if(iFileIndex == Index)
            {
               uint iReturnStringLength = iFileNameLength - iExtensionLength - 1;
               strncpy(Name, sFileName, iReturnStringLength);
               Name[iReturnStringLength] = '\0';
               break;
            }

            iFileIndex++;
         }
      }
   }
   while(sFileName);

   AAssetDir_close(aAssetDir);
}

int Device::getNumberOfCPUCores()
{
   return android_getCpuCount();
}
