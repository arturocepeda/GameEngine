
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class (Windows Phone 8)
//
//  --- GEDevice.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"

#include <iostream>
#include <stdarg.h>
#include <fstream>

#include <windows.h>

using namespace GE::Core;
using namespace GE::Content;

int Device::ScreenWidth = 0;
int Device::ScreenHeight = 0;
DeviceOrientation Device::Orientation = DeviceOrientation::Portrait;

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

GE::uint Device::getContentFilesCount(const char* SubDir, const char* Extension)
{
   return 0;
}

void Device::getContentFileName(const char* SubDir, const char* Extension, GE::uint Index, char* Name)
{

}

void Device::readContentFile(ContentType Type, const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sFileName[256];
   sprintf(sFileName, "content\\%s\\%s.%s", SubDir, Name, Extension);

   GE::uint iFileLength = getFileLength(sFileName);
   GEAssert(iFileLength > 0);

   unsigned char* pFileData = new unsigned char[Type == ContentType::GenericTextData ? iFileLength + 1 : iFileLength];
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

   delete[] pFileData;
}

void Device::log(const char* Message, ...)
{
  char sBuffer[256];

  va_list vArguments;
  va_start(vArguments, Message);
  vsprintf(sBuffer, Message, vArguments);
  va_end(vArguments);

  OutputDebugString(sBuffer);
}

wchar_t* charToWChar(const char* text)
{
   size_t size = strlen(text) + 1;
   wchar_t* wa = new wchar_t[size];
   mbstowcs(wa, text, size);
   return wa;
}

std::wstring getFullPath(const char* Filename)
{
   auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation; 
   std::wstring dir = folder->Path->ToString()->Data(); 
   return dir.append(L"\\").append(charToWChar(Filename));
}

GE::uint Device::getFileLength(const char* Filename)
{
   std::ifstream file(getFullPath(Filename), std::ios::in | std::ios::binary);
      
   if(file.is_open())
   {
      file.seekg(0, std::ios::end);
      std::streamsize size = file.tellg();
      file.close();
      return (GE::uint)size;
   }

   return 0;
}

GE::uint Device::readFile(const char* Filename, unsigned char* ReadBuffer, GE::uint BufferSize)
{
   std::ifstream file(getFullPath(Filename), std::ios::in | std::ios::binary);
      
   if(file.is_open())
   {
      file.seekg(0, std::ios::end);
      std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);

      file.read((char*)ReadBuffer, size);
      file.close();
      return (GE::uint)size;
   }

   return 0;
}

bool Device::userFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sFileName[128];
   sprintf(sFileName, "\\%s\\%s.%s", SubDir, Name, Extension);

   auto cStorageFolder = Windows::Storage::ApplicationData::Current->LocalFolder;   
   std::wstring wsFullPath = cStorageFolder->Path->ToString()->Data(); 
   wsFullPath.append(charToWChar(sFileName));

   FILE* pFile;
   _wfopen_s(&pFile, wsFullPath.c_str(), L"rb");

   if(pFile)
   {
      fclose(pFile);
      return true;
   }

   return false;
}

void Device::readUserFile(const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sFileName[128];
   sprintf(sFileName, "\\%s\\%s.%s", SubDir, Name, Extension);

   auto cStorageFolder = Windows::Storage::ApplicationData::Current->LocalFolder;   
   std::wstring wsFullPath = cStorageFolder->Path->ToString()->Data(); 
   wsFullPath.append(charToWChar(sFileName));

   FILE* pFile;
   _wfopen_s(&pFile, wsFullPath.c_str(), L"rb");
   GEAssert(pFile);

   fseek(pFile, 0L, SEEK_END);
   GE::uint iFileSize = ftell(pFile);
   fseek(pFile, 0L, SEEK_SET);

   char* sBuffer = Allocator::alloc<char>(iFileSize);
   fread(sBuffer, 1, iFileSize, pFile);
   ContentData->load(iFileSize, sBuffer);

   Allocator::free(sBuffer);
   fclose(pFile);
}

void Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension, const ContentData* ContentData)
{
   const GE::uint BufferSize = 128;
   char sDirectory[BufferSize];
   sprintf(sDirectory, "\\%s", SubDir);

   auto cStorageFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
   std::wstring wsDirectory = cStorageFolder->Path->ToString()->Data();
   wsDirectory.append(charToWChar(sDirectory));

   wcstombs(sDirectory, wsDirectory.c_str(), BufferSize);
   CreateDirectory(sDirectory, 0);

   char sFileName[128];
   sprintf(sFileName, "\\%s\\%s.%s", SubDir, Name, Extension);

   std::wstring wsFullPath = cStorageFolder->Path->ToString()->Data(); 
   wsFullPath.append(charToWChar(sFileName));

   FILE* pFile;
   _wfopen_s(&pFile, wsFullPath.c_str(), L"w+");
   GEAssert(pFile);

   fwrite(ContentData->getData(), 1, ContentData->getDataSize(), pFile);
   fflush(pFile);
   fclose(pFile);
}

GE::uint Device::getUserFilesCount(const char* SubDir, const char* Extension)
{
   return 0;
}

void Device::getUserFileName(const char* SubDir, const char* Extension, GE::uint Index, char* Name)
{

}

int Device::getNumberOfCPUCores()
{
   SYSTEM_INFO sInfo;
   GetNativeSystemInfo(&sInfo);
   return sInfo.dwNumberOfProcessors;
}
