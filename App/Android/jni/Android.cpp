
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Android
//
//  --- Android.cpp ---
//
//////////////////////////////////////////////////////////////////


#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <memory>
#include <vector>

#include "cpu-features.h"

#include "config.h"

#include "Rendering/OpenGL/GERenderSystemES20.h"
#include "Audio/GEAudioSystem.h"
#include "Input/GEInputSystem.h"

#include "Core/GEDevice.h"
#include "Core/GEStateManager.h"
#include "Core/GETaskManager.h"
#include "Core/GETimer.h"
#include "Core/GETime.h"
#include "Core/GEApplication.h"
#include "Core/GELog.h"
#include "Core/GEDistributionPlatform.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Audio;
using namespace GE::Input;

static RenderSystem* cRender = nullptr;
static AudioSystem* cAudio = nullptr;
static TaskManager* cTaskManager = nullptr;
static AppSettings gSettings;
static DistributionPlatform gDistributionPlatform;

static bool bInitialized = false;
static bool bPaused = false;

static StateManager cStateManager;
static Timer cTimer;
static double dTime = 0.0;

static int iFingerID[GE_MAX_FINGERS];
static Vector2 vFingerPosition[GE_MAX_FINGERS];

static Scaler* cPixelToScreenX = nullptr;
static Scaler* cPixelToScreenY = nullptr;

static JavaVM* gJavaVM = nullptr;

class AndroidLogListener : public LogListener
{
public:
   virtual void onLog(LogType pType, const char* pMessage) override
   {
      int logPrio = ANDROID_LOG_DEFAULT;

      switch(pType)
      {
         case LogType::Info:
            logPrio = ANDROID_LOG_INFO;
            break;
         case LogType::Warning:
            logPrio = ANDROID_LOG_WARN;
            break;
         case LogType::Error:
            logPrio = ANDROID_LOG_ERROR;
            break;
      }

      __android_log_print(logPrio, GE_APP_NAME, "%s", pMessage);
   }
};
static AndroidLogListener gLogListener;

extern "C"
{
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_Initialize(JNIEnv* env, jobject obj, jint width, jint height);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_UpdateFrame(JNIEnv* env, jobject obj);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_Pause(JNIEnv* env, jobject obj);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_Resume(JNIEnv* env, jobject obj);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_SetAudioManagerValues(JNIEnv* env, jobject obj, jint sampleRate, jint framesPerBuffer);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputTouchDown(JNIEnv* env, jclass clazz, jint index, jfloat x, jfloat y);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputTouchMove(JNIEnv* env, jclass clazz, jint index, jfloat x, jfloat y);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputTouchUp(JNIEnv* env, jclass clazz, jint index, jfloat x, jfloat y);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputButtonDown(JNIEnv* env, jclass clazz, jint button);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputButtonUp(JNIEnv* env, jclass clazz, jint button);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_UpdateAccelerometerStatus(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jfloat z);
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_UpdateDeviceRotationVector(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jfloat z, jfloat w);
};

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_Initialize(JNIEnv* env, jobject obj, jint width, jint height)
{
   if(bInitialized)
      return;

   env->GetJavaVM(&gJavaVM);
   GEAssert(gJavaVM);

   Application::Name = GE_APP_NAME;
   Application::ID = GE_APP_ID;
   Application::VersionString = GE_VERSION_STRING;
   Application::VersionNumber = GE_VERSION_NUMBER;

#if defined (GE_BINARY_CONTENT)
   Application::ContentType = ApplicationContentType::Bin;
#endif

   Log::addListener(&gLogListener);

   gDistributionPlatform.init();

   // initialize the application
   Application::startUp(initAppModule);

   // screen size
   Device::ScreenWidth = width;
   Device::ScreenHeight = height;

   // device orientation
#ifdef GE_ORIENTATION_PORTRAIT
   Device::Orientation = DeviceOrientation::Portrait;
   Device::AspectRatio = (float)width / (float)height;
#else
   Device::Orientation = DeviceOrientation::Landscape;
   Device::AspectRatio = (float)height / (float)width;
#endif

   // IDs for touch management
   for(int i = 0; i < GE_MAX_FINGERS; i++)
      iFingerID[i] = -1;

   cPixelToScreenX = new Scaler(0.0f, Device::ScreenWidth, -1.0f, 1.0f);
   cPixelToScreenY = new Scaler(0.0f, Device::ScreenHeight, Device::getAspectRatio(), -Device::getAspectRatio());

   // set the input device
   InputSystem::getInstance()->setCurrentInputDevice(InputDevice::Touchpad);
   
   // initialize rendering system
   cRender = new RenderSystemES20();

   // initialize audio system
#if defined (GE_AUDIO_SUPPORT)
   cAudio = new AudioSystem();
   cAudio->init();
#endif

   // start the timer
   cTimer.start();
   dTime = 0.0;
   Time::reset();

   // create task manager
   cTaskManager = new TaskManager();

   // set the initialized flag
   bInitialized = true;
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_UpdateFrame(JNIEnv* env, jobject obj)
{
   if(bPaused)
      return;

   if(cTaskManager->getExitPending())
      exit(0);

   double dCurrentTime = cTimer.getTime();
   Time::setDelta((dCurrentTime - dTime) * 0.000001f);
   dTime = dCurrentTime;

   cTaskManager->update();
   cTaskManager->render();
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_Pause(JNIEnv* env, jobject obj)
{
   if(!bInitialized)
      return;

   bPaused = true;
   cTimer.stop();
   dTime = 0.0;
   cStateManager.getActiveState()->pause();
   cAudio->pauseAll();
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_Resume(JNIEnv* env, jobject obj)
{
   if(!bInitialized)
      return;

   bPaused = false;
   cTimer.start();
   cStateManager.getActiveState()->resume();
   cAudio->resumeAll();
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_SetAudioManagerValues(JNIEnv* env, jobject obj, jint sampleRate, jint framesPerBuffer)
{
   Device::AudioSystemSampleRate = sampleRate;
   Device::AudioSystemFramesPerBuffer = framesPerBuffer;
}

GE::Vector2 pixelToScreen(const GE::Vector2& vPixelPosition)
{
   return GE::Vector2((float)cPixelToScreenX->y(vPixelPosition.X), (float)cPixelToScreenY->y(vPixelPosition.Y));
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputTouchDown(JNIEnv* env, jclass clazz, jint index, jfloat x, jfloat y)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == -1)
      {
         iFingerID[i] = index;
         vFingerPosition[i] = pixelToScreen(Vector2(x, y));
         InputSystem::getInstance()->inputTouchBegin(i, vFingerPosition[i]);
         break;
      }
   }
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputTouchMove(JNIEnv* env, jclass clazz, jint index, jfloat x, jfloat y)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == index)
      {
         Vector2 vPreviousPosition = vFingerPosition[i];
         vFingerPosition[i] = pixelToScreen(Vector2(x, y));
         InputSystem::getInstance()->inputTouchMove(i, vPreviousPosition, vFingerPosition[i]);
         break;
      }
   }
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputTouchUp(JNIEnv* env, jclass clazz, jint index, jfloat x, jfloat y)
{
   for(int i = 0; i < GE_MAX_FINGERS; i++)
   {
      if(iFingerID[i] == index)
      {
         iFingerID[i] = -1;
         vFingerPosition[i] = pixelToScreen(Vector2(x, y));
         InputSystem::getInstance()->inputTouchEnd(i, vFingerPosition[i]);
         break;
      }
   }
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputButtonDown(JNIEnv* env, jclass clazz, jint button)
{
   
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_InputButtonUp(JNIEnv* env, jclass clazz, jint button)
{
   
}

const float AccelFactor = 0.01f;

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_UpdateAccelerometerStatus(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jfloat z)
{
   if(bInitialized)
   {
      InputSystem::getInstance()->updateAccelerometerStatus(Vector3(x * -AccelFactor, y * -AccelFactor, z * AccelFactor));
   }
}

JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_UpdateDeviceRotationVector(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jfloat z, jfloat w)
{
   if(bInitialized)
   {
      Device::Rotation.X = x;
      Device::Rotation.Y = y;
      Device::Rotation.Z = z;
      Device::Rotation.W = w;
   }
}

SystemLanguage Device::requestOSLanguage()
{
   GEAssert(gJavaVM);
   JNIEnv* env = nullptr;
   const jint attachmentResult = gJavaVM->AttachCurrentThread(&env, nullptr);
   GEAssert(attachmentResult == JNI_OK);
   GEAssert(env);

   jclass localeClass = env->FindClass("java/util/Locale");
   jmethodID localeDefaultMethod = env->GetStaticMethodID(localeClass, "getDefault", "()Ljava/util/Locale;");
   jmethodID localeGetLanguageMethod = env->GetMethodID(localeClass, "getLanguage", "()Ljava/lang/String;");
   jobject localeInstance = env->CallStaticObjectMethod(localeClass, localeDefaultMethod);
   jstring languageStringObj = (jstring)env->CallObjectMethod(localeInstance, localeGetLanguageMethod);

   const char* language = env->GetStringUTFChars(languageStringObj, nullptr);

   if(strncmp(language, "en", 2u) == 0)
   {
      return SystemLanguage::English;
   }
   if(strncmp(language, "es", 2u) == 0)
   {
      return SystemLanguage::Spanish;
   }
   if(strncmp(language, "de", 2u) == 0)
   {
      return SystemLanguage::German;
   }
   if(strncmp(language, "fr", 2u) == 0)
   {
      return SystemLanguage::French;
   }
   if(strncmp(language, "it", 2u) == 0)
   {
      return SystemLanguage::Italian;
   }
   if(strncmp(language, "pt", 2u) == 0)
   {
      return SystemLanguage::Portuguese;
   }
   if(strncmp(language, "ru", 2u) == 0)
   {
      return SystemLanguage::Russian;
   }
   if(strncmp(language, "zh_CN", 5u) == 0 || strncmp(language, "zh", 2u) == 0)
   {
      return SystemLanguage::ChineseSimplified;
   }
   if(strncmp(language, "zh_TW", 5u) == 0)
   {
      return SystemLanguage::ChineseTraditional;
   }
   if(strncmp(language, "ja", 2u) == 0)
   {
      return SystemLanguage::Japanese;
   }
   if(strncmp(language, "ko", 2u) == 0)
   {
      return SystemLanguage::Korean;
   }

   return SystemLanguage::English;
}

int Device::getNumberOfCPUCores()
{
   return android_getCpuCount();
}
