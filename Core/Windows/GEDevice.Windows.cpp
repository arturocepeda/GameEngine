
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
#include "Core/GEApplication.h"
#include "Core/GEUtils.h"
#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"

#include <iostream>
#include <stdarg.h>
#include <fstream>

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;

const uint MaxPath = 256u;

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
   char sBuffer[MaxPath];

   GetCurrentDirectory(MaxPath, sBuffer);
   sprintf(sBuffer, "%s\\%s", sBuffer, Filename);

   return GESTLString(sBuffer);
}

void toWindowsSubDir(const char* SubDir, char* WinSubDir)
{
   strcpy(WinSubDir, SubDir);
   size_t iLength = strlen(WinSubDir);

   for(size_t i = 0; i < iLength; i++)
   {
      if(WinSubDir[i] == '/')
      {
         WinSubDir[i] = '\\';
      }
   }
}

uint Device::getContentFilesCount(const char* SubDir, const char* Extension)
{
   char sPath[MaxPath];
   DWORD dw = GetCurrentDirectory(MaxPath, sPath);
   GEAssert(dw >= 0);

   const size_t contentPathOffset = ContentHashPath ? strlen(sPath) + 1u : 0u;

   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   char sFindString[MaxPath];
   sprintf(sFindString, "%s\\%s\\*.%s", sPath, sSubDir, Extension);

   if(ContentHashPath)
   {
      toHashPath(sFindString + contentPathOffset);
   }

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return 0;

   uint iFilesCount = 1;

   while(FindNextFile(hFile, &sFileData))
   {
      iFilesCount++;
   }

   FindClose(hFile);

   return iFilesCount;
}

bool Device::getContentFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   char sPath[MaxPath];
   DWORD dw = GetCurrentDirectory(MaxPath, sPath);
   GEAssert(dw >= 0);

   const size_t contentPathOffset = ContentHashPath ? strlen(sPath) + 1u : 0u;

   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   char sFindString[MaxPath];
   sprintf(sFindString, "%s\\%s\\*.%s", sPath, sSubDir, Extension);

   if(ContentHashPath)
   {
      toHashPath(sFindString + contentPathOffset);
   }

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return false;

   uint iCurrentFileIndex = 0;

   while(iCurrentFileIndex < Index && FindNextFile(hFile, &sFileData))
   {
      iCurrentFileIndex++;
   }

   FindClose(hFile);

   if(iCurrentFileIndex == Index)
   {
      char buffer[64];
      const char* extension = Extension;

      if(ContentHashPath)
      {
         strcpy(buffer, Extension);
         toHashPath(buffer);
         extension = buffer;
      }

      size_t iLength = strlen(sFileData.cFileName) - (strlen(extension) + 1u);
      memcpy(Name, sFileData.cFileName, iLength);
      Name[iLength] = '\0';
   }

   return true;
}

bool Device::contentFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   char sFileName[MaxPath];
   sprintf(sFileName, "%s\\%s.%s", sSubDir, Name, Extension);

   if(ContentHashPath)
   {
      toHashPath(sFileName);
   }

   return getFileLength(sFileName) > 0;
}

void Device::readContentFile(ContentType Type, const char* SubDir, const char* Name, const char* Extension,
   ContentData* ContentData)
{
   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   char sFileName[MaxPath];

   if(ContentHashPath)
   {
      if(isHash(Name))
      {
         toHashPath(sSubDir);
         
         char extension[32];
         strcpy(extension, Extension);
         toHashPath(extension);

         sprintf(sFileName, "%s\\%s.%s", sSubDir, Name, extension);
      }
      else
      {
         sprintf(sFileName, "%s\\%s.%s", sSubDir, Name, Extension);
         toHashPath(sFileName);
      }
   }
   else
   {
      sprintf(sFileName, "%s\\%s.%s", sSubDir, Name, Extension);
   }

   uint iFileLength = getFileLength(sFileName);

   GEAssert(iFileLength > 0);

   uint iRequiredSize = Type == ContentType::GenericTextData ? iFileLength + 1 : iFileLength;
   IOBuffer* pBuffer = requestIOBuffer(iRequiredSize); 
   byte* pFileData = &pBuffer->at(0);
   readFile(sFileName, pFileData, iFileLength);

   if(Type == ContentType::GenericTextData)
   {
      pFileData[iFileLength] = '\0';
   }

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
}

std::ifstream Device::openContentFile(const char* SubDir, const char* Name, const char* Extension)
{
   char subDir[MaxPath];
   toWindowsSubDir(SubDir, subDir);

   char fileName[MaxPath];
   sprintf(fileName, "%s\\%s.%s", subDir, Name, Extension);

   if(ContentHashPath)
   {
      toHashPath(fileName);
   }

   const GESTLString fullPath = getFullPath(fileName);
   return std::ifstream(fullPath.c_str(), std::ios::in | std::ios::binary);
}

bool Device::userFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sPath[MaxPath];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   char sFullPath[MaxPath];

   if(strlen(sSubDir) == 0)
   {
      sprintf(sFullPath, "%s\\%s\\%s.%s", sPath, Application::Name, Name, Extension);
   }
   else
   {
      sprintf(sFullPath, "%s\\%s\\%s\\%s.%s", sPath, Application::Name, sSubDir, Name, Extension);
   }

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
   char sPath[MaxPath];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   char sFullPath[MaxPath];

   if(strlen(sSubDir) == 0)
   {
      sprintf(sFullPath, "%s\\%s\\%s.%s", sPath, Application::Name, Name, Extension);
   }
   else
   {
      sprintf(sFullPath, "%s\\%s\\%s\\%s.%s", sPath, Application::Name, sSubDir, Name, Extension);
   }

   std::ifstream file(sFullPath, std::ios::in | std::ios::binary);
   GEAssert(file.is_open());

   file.seekg(0, std::ios::end);
   uint iFileLength = (uint)file.tellg();
   file.seekg(0, std::ios::beg);

   IOBuffer* pBuffer = requestIOBuffer(iFileLength); 
   byte* pFileData = &pBuffer->at(0);
   file.read((char*)pFileData, iFileLength);
   file.close();

   ContentData->load(iFileLength, (const char*)pFileData);
}

std::ofstream Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension)
{
   char sPath[MaxPath];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sDirectory[MaxPath];
   sprintf(sDirectory, "%s\\%s", sPath, Application::Name);
   CreateDirectory(sDirectory, NULL);
   
   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   if(strlen(sSubDir) > 0)
   {
      sprintf(sDirectory, "%s\\%s\\%s", sPath, Application::Name, sSubDir);
      CreateDirectory(sDirectory, NULL);
   }

   char sFullPath[MaxPath];
   sprintf(sFullPath, "%s\\%s.%s", sDirectory, Name, Extension);

   return std::ofstream(sFullPath, std::ios::out | std::ios::binary);
}

void Device::deleteUserFile(const char* SubDir, const char* Name, const char* Extension)
{
   char sPath[MaxPath];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sDirectory[MaxPath];
   sprintf(sDirectory, "%s\\%s", sPath, Application::Name);

   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   if(strlen(sSubDir) > 0)
   {
      sprintf(sDirectory, "%s\\%s\\%s", sPath, Application::Name, sSubDir);
   }

   char sFullPath[MaxPath];
   sprintf(sFullPath, "%s\\%s.%s", sDirectory, Name, Extension);

   DeleteFile(sFullPath);
}

uint Device::getUserFilesCount(const char* SubDir, const char* Extension)
{
   char sPath[MaxPath];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   char sFindString[MaxPath];

   if(strlen(sSubDir) == 0)
      sprintf(sFindString, "%s\\%s\\*.%s", sPath, Application::Name, Extension);
   else
      sprintf(sFindString, "%s\\%s\\%s\\*.%s", sPath, Application::Name, sSubDir, Extension);

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

bool Device::getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
{
   char sPath[MaxPath];
   HRESULT hr = SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, sPath);
   GEAssert(hr >= 0);

   char sSubDir[MaxPath];
   toWindowsSubDir(SubDir, sSubDir);

   char sFindString[MaxPath];

   if(strlen(sSubDir) == 0)
      sprintf(sFindString, "%s\\%s\\*.%s", sPath, Application::Name, Extension);
   else
      sprintf(sFindString, "%s\\%s\\%s\\*.%s", sPath, Application::Name, sSubDir, Extension);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return false;

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

   return true;
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

SystemLanguage Device::requestOSLanguage()
{
   char sLanguageName[32];
   LCID lcidUserDefault = GetUserDefaultLCID();
   GetLocaleInfo(lcidUserDefault, LOCALE_SENGLISHLANGUAGENAME, sLanguageName, sizeof(sLanguageName) / sizeof(TCHAR));

   if(strcmp(sLanguageName, "English") == 0)
   {
      return SystemLanguage::English;
   }
   else if(strcmp(sLanguageName, "Spanish") == 0)
   {
      return SystemLanguage::Spanish;
   }
   else if(strcmp(sLanguageName, "German") == 0)
   {
      return SystemLanguage::German;
   }

   return SystemLanguage::English;
}

void Device::requestSupportedScreenResolutions(GESTLVector(Point)* pOutResolutions)
{
   GEAssert(pOutResolutions);
   pOutResolutions->clear();

   int modeNum = 0;
   int modeWidth = 0;
   int modeHeight = 0;

   DEVMODE devMode = { 0 };
   devMode.dmSize = sizeof(DEVMODE);

   while(EnumDisplaySettings(nullptr, modeNum++, &devMode))
   {
      if(devMode.dmPelsWidth >= 800 &&
         (devMode.dmPelsWidth != modeWidth || devMode.dmPelsHeight != modeHeight))
      {
         modeWidth = devMode.dmPelsWidth;
         modeHeight = devMode.dmPelsHeight;
         pOutResolutions->emplace(pOutResolutions->begin(), modeWidth, modeHeight);
      }
   }
}

void Device::openWebPage(const char* pURL)
{
   ShellExecuteA(NULL, "open", pURL, NULL, NULL, SW_SHOWNORMAL);
}
