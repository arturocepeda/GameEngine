
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class
//
//  --- GEDevice.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Content/GEContentData.h"

namespace GE { namespace Core
{
   GESerializableEnum(SystemLanguage)
   {
      English,
      Spanish,
      German,

      Count
   };


   enum class DeviceOrientation
   {
      Portrait,
      Landscape
   };


   class Device
   {
   private:
      static uint getFileLength(const char* Filename);
      static uint readFile(const char* Filename, GE::byte* ReadBuffer, uint BufferSize);

   public:
      static int ScreenWidth;
      static int ScreenHeight;

      static SystemLanguage Language;

      static DeviceOrientation Orientation;
      static Quaternion Rotation;

      static int AudioSystemSampleRate;
      static int AudioSystemFramesPerBuffer;

      static int getScreenWidth();
      static int getScreenHeight();
      static float getAspectRatio();
   
      static int getTouchPadWidth();
      static int getTouchPadHeight();

      static int getNumberOfCPUCores();

      static bool contentFileExists(const char* SubDir, const char* Name, const char* Extension);
      static void readContentFile(Content::ContentType Type, const char* SubDir, const char* Name, const char* Extension, Content::ContentData* ContentData);

      static uint getContentFilesCount(const char* SubDir, const char* Extension);
      static void getContentFileName(const char* SubDir, const char* Extension, uint Index, char* Name);

      static bool userFileExists(const char* SubDir, const char* Name, const char* Extension);
      static void readUserFile(const char* SubDir, const char* Name, const char* Extension, Content::ContentData* ContentData);
      static void writeUserFile(const char* SubDir, const char* Name, const char* Extension, const Content::ContentData* ContentData);

      static uint getUserFilesCount(const char* SubDir, const char* Extension);
      static void getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name);

      static void log(const char* Message, ...);
   };
}}
