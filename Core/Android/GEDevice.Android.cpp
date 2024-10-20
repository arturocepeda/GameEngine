
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class (Android)
//
//  --- GEDevice.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDevice.h"
#include "Core/GEAllocator.h"
#include "Core/GEApplication.h"
#include "Core/GEUtils.h"
#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"
#include "Types/GESTLTypes.h"

using namespace GE::Core;
using namespace GE::Content;

void Device::platformInit()
{
}

void Device::platformUpdate()
{
}

void Device::platformRelease()
{
}

int Device::getTouchPadWidth()
{
   return ScreenWidth;
}

int Device::getTouchPadHeight()
{
   return ScreenHeight;
}

SystemLanguage Device::requestOSLanguage()
{
   return SystemLanguage::English;
}

void Device::requestSupportedScreenResolutions(GESTLVector(Point)* pOutResolutions)
{
}

static void getContentFullPath(const char* pSubDir, const char* pName, const char* pExtension, char* pFullPath)
{
   char name[64];
   strcpy(name, pName);

   char extension[64];
   strcpy(extension, pExtension);

   if(Device::ContentHashPath)
   {
      if(!isHash(pName))
      {
         toHashPath(name);
      }

      toHashPath(extension);
   }

   if(strcmp(pSubDir, ".") == 0)
   {
      sprintf(pFullPath, "%s.%s", name, extension);
   }
   else
   {
      char subDir[64];
      strcpy(subDir, pSubDir);

      if(Device::ContentHashPath)
      {
         toHashPath(subDir);
      }

      sprintf(pFullPath, "%s/%s.%s", subDir, name, extension);
   }
}

bool Device::contentFileExists(const char* SubDir, const char* Name, const char* Extension)
{
   char sFileName[256];
   getContentFullPath(SubDir, Name, Extension, sFileName);

   return getFileLength(sFileName) > 0;
}

void Device::readContentFile(ContentType Type, const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
{
   char sFileName[256];
   getContentFullPath(SubDir, Name, Extension, sFileName);

   const unsigned int iFileLength = getFileLength(sFileName);
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

void Device::showVirtualKeyboard(const char*)
{
}

void Device::hideVirtualKeyboard()
{
}

bool Device::getVirtualKeyboardActive()
{
   return false;
}

const char* Device::getVirtualKeyboardCurrentText()
{
   return "";
}

void Device::openWebPage(const char* pURL)
{
}
