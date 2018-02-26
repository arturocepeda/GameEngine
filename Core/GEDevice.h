
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
#include "Types/GESTLTypes.h"
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
      typedef GESTLVector(GE::byte) IOBuffer;
      typedef GESTLMap(GE::uint, IOBuffer) IOBuffersMap;

      static GEMutex mIOMutex;
      static IOBuffersMap* mIOBuffers;

      static IOBuffer* requestIOBuffer(GE::uint iSize);

      static GE::uint getFileLength(const char* Filename);
      static GE::uint readFile(const char* Filename, GE::byte* ReadBuffer, GE::uint BufferSize);

   public:
      static int ScreenWidth;
      static int ScreenHeight;

      static SystemLanguage Language;

      static DeviceOrientation Orientation;
      static Quaternion Rotation;

      static int AudioSystemSampleRate;
      static int AudioSystemFramesPerBuffer;

      static void init();
      static void release();

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
   };
}}
