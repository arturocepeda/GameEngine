
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class (Windows)
//
//  --- GEDevice.cpp ---
//
//////////////////////////////////////////////////////////////////

#include <windows.h>
#include <Shlobj.h>

#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Core/GEApplication.h"
#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"

#include <iostream>
#include <stdarg.h>
#include <fstream>

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;

int Device::ScreenWidth = 0;
int Device::ScreenHeight = 0;

SystemLanguage Device::Language = SystemLanguage::English;
DeviceOrientation Device::Orientation = DeviceOrientation::Portrait;

LogListener* Device::CurrentLogListener = 0;

int Device::AudioSystemSampleRate = 0;
int Device::AudioSystemFramesPerBuffer = 0;

int Device::getScreenWidth()
{
   return ScreenWidth;
}

int Device::getScreenHeight()
{
   return ScreenHeight;
}

float Device::getAspectRatio()
{
   return (float)ScreenHeight / ScreenWidth;
}

int Device::getTouchPadWidth()
{
   return ScreenWidth;
}

int Device::getTouchPadHeight()
{
   return ScreenHeight;
}

GESTLString getFullPath(const char* Filename)
{
   const int BufferSize = 256;
   char sBuffer[BufferSize];

   GetCurrentDirectory(BufferSize, sBuffer);
   sprintf(sBuffer, "%s\\%s", sBuffer, Filename);

   return GESTLString(sBuffer);
}

uint Device::getContentFilesCount(const char* SubDir, const char* Extension)
{
   char sPath[MAX_PATH];
   DWORD dw = GetCurrentDirectory(MAX_PATH, sPath);
   GEAssert(dw >= 0);

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\%s\\*.%s", sPath, SubDir, Extension);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return 0;

   uint iFilesCount = 1;

   while(FindNextFile(hFile, &sFileData))
      iFilesCount++;

   FindClose(hFile);

   return iFilesCount;
}

void Device::getContentFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   char sPath[MAX_PATH];
   DWORD dw = GetCurrentDirectory(MAX_PATH, sPath);
   GEAssert(dw >= 0);

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\%s\\*.%s", sPath, SubDir, Extension);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   uint iCurrentFileIndex = 0;

   while(iCurrentFileIndex < Index && FindNextFile(hFile, &sFileData))
      iCurrentFileIndex++;

   FindClose(hFile);

   if(iCurrentFileIndex == Index)
   {
      size_t iLength = strlen(sFileData.cFileName) - (strlen(Extension) + 1);
      memcpy(Name, sFileData.cFileName, iLength);
      Name[iLength] = '\0';
   }
}

bool Device::contentFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sFileName[256];
   sprintf(sFileName, "%s\\%s.%s", SubDir, Name, Extension);

   return getFileLength(sFileName) > 0;
}

void Device::readContentFile(ContentType Type, const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sFileName[256];
   sprintf(sFileName, "%s\\%s.%s", SubDir, Name, Extension);

   uint iFileLength = getFileLength(sFileName);

   GEAssert(iFileLength > 0);

   byte* pFileData =
      Allocator::alloc<byte>(Type == ContentType::GenericTextData ? iFileLength + 1 : iFileLength);
   readFile(sFileName, pFileData, iFileLength);

   if(Type == ContentType::GenericTextData)
      pFileData[iFileLength] = '\0';

   switch(Type)
   {
   case ContentType::Texture:
   case ContentType::FontTexture:
      static_cast<ImageData*>(ContentData)->load(iFileLength, (const char*)pFileData);
      break;
   case ContentType::Audio:
      static_cast<AudioData*>(ContentData)->load(iFileLength, (const char*)pFileData);
      break;
   default:
      ContentData->load(Type == ContentType::GenericTextData ? iFileLength + 1 : iFileLength, (const char*)pFileData);
   }

   Allocator::free(pFileData);
}

bool Device::userFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sPath[MAX_PATH];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sFullPath[MAX_PATH];

   if(!SubDir || strlen(SubDir) == 0)
      sprintf(sFullPath, "%s\\%s\\%s.%s", sPath, Application::Name, Name, Extension);
   else
      sprintf(sFullPath, "%s\\%s\\%s\\%s.%s", sPath, Application::Name, SubDir, Name, Extension);

   std::ifstream file(sFullPath, std::ios::in | std::ios::binary);

   if(file.is_open())
   {
      file.close();
      return true;
   }

   return false;
}

void Device::readUserFile(const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sPath[MAX_PATH];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sFullPath[MAX_PATH];

   if(!SubDir || strlen(SubDir) == 0)
      sprintf(sFullPath, "%s\\%s\\%s.%s", sPath, Application::Name, Name, Extension);
   else
      sprintf(sFullPath, "%s\\%s\\%s\\%s.%s", sPath, Application::Name, SubDir, Name, Extension);

   std::ifstream file(sFullPath, std::ios::in | std::ios::binary);
   GEAssert(file.is_open());

   file.seekg(0, std::ios::end);
   uint iFileLength = (uint)file.tellg();
   file.seekg(0, std::ios::beg);

   byte* pFileData = Allocator::alloc<byte>(iFileLength);
   file.read((char*)pFileData, iFileLength);
   file.close();

   ContentData->load(iFileLength, (const char*)pFileData);
   Allocator::free(pFileData);
}

void Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension, const ContentData* ContentData)
{
   char sPath[MAX_PATH];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sDirectory[MAX_PATH];
   sprintf(sDirectory, "%s\\%s", sPath, Application::Name);
   CreateDirectory(sDirectory, NULL);
   
   if(SubDir && strlen(SubDir) > 0)
   {
      sprintf(sDirectory, "%s\\%s\\%s", sPath, Application::Name, SubDir);
      CreateDirectory(sDirectory, NULL);
   }

   char sFullPath[MAX_PATH];
   sprintf(sFullPath, "%s\\%s.%s", sDirectory, Name, Extension);

   std::ofstream file(sFullPath, std::ios::out | std::ios::binary);
   GEAssert(file.is_open());

   file.write(ContentData->getData(), ContentData->getDataSize());
   file.close();
}

uint Device::getUserFilesCount(const char* SubDir, const char* Extension)
{
   char sPath[MAX_PATH];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sFindString[MAX_PATH];

   if(!SubDir || strlen(SubDir) == 0)
      sprintf(sFindString, "%s\\%s\\*.%s", sPath, Application::Name, Extension);
   else
      sprintf(sFindString, "%s\\%s\\%s\\*.%s", sPath, Application::Name, SubDir, Extension);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return 0;

   uint iFilesCount = 1;

   while(FindNextFile(hFile, &sFileData))
      iFilesCount++;

   FindClose(hFile);

   return iFilesCount;
}

void Device::getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   char sPath[MAX_PATH];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sFindString[MAX_PATH];

   if(!SubDir || strlen(SubDir) == 0)
      sprintf(sFindString, "%s\\%s\\*.%s", sPath, Application::Name, Extension);
   else
      sprintf(sFindString, "%s\\%s\\%s\\*.%s", sPath, Application::Name, SubDir, Extension);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   uint iCurrentFileIndex = 0;

   while(iCurrentFileIndex < Index && FindNextFile(hFile, &sFileData))
      iCurrentFileIndex++;

   FindClose(hFile);

   if(iCurrentFileIndex == Index)
   {
      size_t iLength = strlen(sFileData.cFileName) - (strlen(Extension) + 1);
      memcpy(Name, sFileData.cFileName, iLength);
      Name[iLength] = '\0';
   }
}

void Device::log(const char* Message, ...)
{
#if defined(GE_EDITOR_SUPPORT)
  char sBuffer[1024];

  va_list vArguments;
  va_start(vArguments, Message);
  vsprintf(sBuffer, Message, vArguments);
  va_end(vArguments);

  OutputDebugString(sBuffer);
  OutputDebugString("\n");

  if(CurrentLogListener)
  {
     CurrentLogListener->onLog(sBuffer);
  }
#endif
}

uint Device::getFileLength(const char* Filename)
{
   GESTLString sFullPath = getFullPath(Filename);
   std::ifstream file(sFullPath.c_str(), std::ios::in | std::ios::binary);
      
   if(file.is_open())
   {
      file.seekg(0, std::ios::end);
      std::streamsize size = file.tellg();
      file.close();
      return (uint)size;
   }

   return 0;
}

uint Device::readFile(const char* Filename, GE::byte* ReadBuffer, uint BufferSize)
{
   GESTLString sFullPath = getFullPath(Filename);
   std::ifstream file(sFullPath.c_str(), std::ios::in | std::ios::binary);
      
   if(file.is_open())
   {
      file.seekg(0, std::ios::end);
      std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);

      file.read((char*)ReadBuffer, size);
      file.close();
      return (uint)size;
   }

   return 0;
}

int Device::getNumberOfCPUCores()
{
   SYSTEM_INFO sInfo;
   GetNativeSystemInfo(&sInfo);
   return sInfo.dwNumberOfProcessors;
}
