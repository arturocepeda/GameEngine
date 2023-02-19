
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class (iOS)
//
//  --- GEDevice.iOS.mm ---
//
//////////////////////////////////////////////////////////////////

#include "../GEDevice.h"
#include "../GEApplication.h"
#include "../GEAllocator.h"
#include "../GEUtils.h"

#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"

#include <fstream>

static const char* kContentPath[] =
{
   "content/",
   "contentBin/"
};
static const size_t kContentPathOffset[] =
{
   8u,   // "content/"
   11u,  // "contentBin/"
};

namespace GE { namespace Core
{
   using namespace Content;

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
      const CGSize& sScreenSize = [[UIScreen mainScreen] bounds].size;
      return (int)std::min(sScreenSize.width, sScreenSize.height);
   }

   int Device::getTouchPadHeight()
   {
      const CGSize& sScreenSize = [[UIScreen mainScreen] bounds].size;
      return (int)std::max(sScreenSize.width, sScreenSize.height);
   }

   uint Device::getFileLength(const char* Filename)
   {
      std::ifstream fStream(Filename, std::ifstream::in | std::ifstream::binary);
      
      if(!fStream.good())
      {
         return 0u;
      }
      
      fStream.seekg(0, std::ifstream::end);
      return (uint)fStream.tellg();
   }

   uint Device::readFile(const char* Filename, unsigned char* ReadBuffer, unsigned int BufferSize)
   {
      std::ifstream fStream(Filename, std::ifstream::in | std::ifstream::binary);
      
      GEAssert(fStream.good());
      
      fStream.read((char*)ReadBuffer, BufferSize);
      return fStream ? BufferSize : 0;
   }

   uint Device::getContentFilesCount(const char* SubDir, const char* Extension)
   {
      const char* contentPath = kContentPath[(uint32_t)Application::ContentType];
      
      char sSubDir[256];
      sprintf(sSubDir, "%s%s", contentPath, SubDir);
      
      char sExtension[64];
      strcpy(sExtension, Extension);
      
      if(ContentHashPath)
      {
         const size_t contentPathOffset = kContentPathOffset[(uint32_t)Application::ContentType];
         toHashPath(sSubDir + contentPathOffset);
         toHashPath(sExtension);
      }
      
      NSString* nsSubDir = [NSString stringWithUTF8String:sSubDir];
      NSString* nsAppDir = [[NSBundle mainBundle] resourcePath];
      NSString* nsDirectory = [nsAppDir stringByAppendingPathComponent:nsSubDir];
      NSArray* nsDirectoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:nsDirectory error:nil];
      NSString* nsExtension = [NSString stringWithUTF8String:sExtension];
      
      uint iFilesCount = 0;
      
      for(NSString* nsFileName in nsDirectoryContents)
      {
         if([nsFileName hasSuffix:nsExtension])
            iFilesCount++;
      }
      
      return iFilesCount;
   }
   
   bool Device::getContentFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
   {
      const char* contentPath = kContentPath[(uint32_t)Application::ContentType];
      
      char sSubDir[256];
      sprintf(sSubDir, "%s%s", contentPath, SubDir);
      
      char sExtension[64];
      strcpy(sExtension, Extension);
      
      if(ContentHashPath)
      {
         const size_t contentPathOffset = kContentPathOffset[(uint32_t)Application::ContentType];
         toHashPath(sSubDir + contentPathOffset);
         toHashPath(sExtension);
      }
      
      NSString* nsSubDir = [NSString stringWithUTF8String:sSubDir];
      NSString* nsAppDir = [[NSBundle mainBundle] resourcePath];
      NSString* nsDirectory = [nsAppDir stringByAppendingPathComponent:nsSubDir];
      NSArray* nsDirectoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:nsDirectory error:nil];
      NSString* nsExtension = [NSString stringWithUTF8String:sExtension];
      
      uint iFileIndex = 0;
      
      for(NSString* nsFileName in nsDirectoryContents)
      {
         if([nsFileName hasSuffix:nsExtension])
         {
            if(iFileIndex == Index)
            {
               NSRange nsRange = [nsFileName rangeOfString:nsExtension];
               NSString* nsFileNameWithoutExtension = [nsFileName substringToIndex:(nsRange.location - 1)];
               strcpy(Name, [nsFileNameWithoutExtension fileSystemRepresentation]);
               
               return true;
            }
            
            iFileIndex++;
         }
      }
      
      return false;
   }
   
   bool Device::contentFileExists(const char* SubDir, const char* Name, const char* Extension)
   {
      const char* contentPath = kContentPath[(uint32_t)Application::ContentType];
      
      char sResourceName[256];
      sprintf(sResourceName, "%s%s/%s.%s", contentPath, SubDir, Name, Extension);
      
      if(ContentHashPath)
      {
         const size_t contentPathOffset = kContentPathOffset[(uint32_t)Application::ContentType];
         toHashPath(sResourceName + contentPathOffset);
      }
      
      NSString* nsResourceName = [NSString stringWithUTF8String:sResourceName];
      NSString* nsFilePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:nsResourceName];
      const char* sFilePath = [nsFilePath UTF8String];
      
      return getFileLength(sFilePath) > 0;
   }

   void Device::readContentFile(ContentType Type, const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
   {
      char subDir[64];
      strcpy(subDir, SubDir);
      
      char name[64];
      strcpy(name, Name);
      
      char extension[64];
      strcpy(extension, Extension);
      
      if(ContentHashPath)
      {
         toHashPath(subDir);
         
         if(!isHash(Name))
         {
            toHashPath(name);
         }
         
         toHashPath(extension);
      }
      
      const char* contentPath = kContentPath[(uint32_t)Application::ContentType];
      
      char sResourceName[256];
      sprintf(sResourceName, "%s%s/%s.%s", contentPath, subDir, name, extension);
      
      NSString* nsResourceName = [NSString stringWithUTF8String:sResourceName];
      NSString* nsFilePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:nsResourceName];
      const char* sFilePath = [nsFilePath UTF8String];
      
      unsigned int iFileLength = getFileLength(sFilePath);
      unsigned int iArraySize = iFileLength;
      
      if(Type == ContentType::GenericTextData)
         iArraySize++;
      
      unsigned char* pFileData = new unsigned char[iArraySize];
      readFile(sFilePath, pFileData, iFileLength);

      if(Type == ContentType::GenericTextData)
      {
         pFileData[iFileLength] = '\0';
         iFileLength++;
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
            ContentData->load(iFileLength, (const char*)pFileData);
            break;
      }
      
      delete[] pFileData;
   }
   
   bool Device::userFileExists(const char* SubDir, const char* Name, const char* Extension)
   {
      char sFileName[64];
      sprintf(sFileName, "%s.%s", Name, Extension);
      NSString* nsFileName = [NSString stringWithUTF8String:sFileName];
      NSString* nsSubDir = [NSString stringWithUTF8String:SubDir];
      
      NSArray* nsPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
      NSString* nsDocumentsDirectory = [nsPaths objectAtIndex:0];
      NSString* nsDirectory = [nsDocumentsDirectory stringByAppendingPathComponent:nsSubDir];
      NSString* aFile = [nsDirectory stringByAppendingPathComponent:nsFileName];
      
      FILE* file = fopen([aFile fileSystemRepresentation], "rb");
      
      if(file)
      {
         fclose(file);
         return true;
      }
      
      return false;
   }
   
   void Device::readUserFile(const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
   {
      char sFileName[64];
      sprintf(sFileName, "%s.%s", Name, Extension);
      NSString* nsFileName = [NSString stringWithUTF8String:sFileName];
      NSString* nsSubDir = [NSString stringWithUTF8String:SubDir];
      
      NSArray* nsPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
      NSString* nsDocumentsDirectory = [nsPaths objectAtIndex:0];
      NSString* nsDirectory = [nsDocumentsDirectory stringByAppendingPathComponent:nsSubDir];
      NSString* aFile = [nsDirectory stringByAppendingPathComponent:nsFileName];
      
      FILE* file = fopen([aFile fileSystemRepresentation], "rb");
      GEAssert(file);
      
      fseek(file, 0L, SEEK_END);
      uint iFileSize = (uint)ftell(file);
      fseek(file, 0L, SEEK_SET);
      
      char* sBuffer = Allocator::alloc<char>(iFileSize);
      fread(sBuffer, 1, iFileSize, file);
      ContentData->load(iFileSize, sBuffer);
      
      Allocator::free(sBuffer);
      fclose(file);
   }
   
   void createDirectoryIfNonExisting(NSString* Dir)
   {
      NSFileManager* nsFileManager = [NSFileManager defaultManager];
      BOOL bIsDirectory;
      
      if(![nsFileManager fileExistsAtPath:Dir isDirectory:&bIsDirectory] || !bIsDirectory)
      {
         NSURL* nsUrl = [NSURL fileURLWithPath:Dir isDirectory:YES];
         [nsFileManager createDirectoryAtURL:nsUrl withIntermediateDirectories:YES attributes:nil error:nil];
      }
   }
   
   void Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension, const ContentData* ContentData)
   {
      char sFileName[64];
      sprintf(sFileName, "%s.%s", Name, Extension);
      NSString* nsFileName = [NSString stringWithUTF8String:sFileName];
      NSString* nsSubDir = [NSString stringWithUTF8String:SubDir];
      
      NSArray* nsPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
      NSString* nsDocumentsDirectory = [nsPaths objectAtIndex:0];
      NSString* nsDirectory = [nsDocumentsDirectory stringByAppendingPathComponent:nsSubDir];
      NSString* aFile = [nsDirectory stringByAppendingPathComponent:nsFileName];
      
      createDirectoryIfNonExisting(nsDirectory);
      FILE* file = fopen([aFile fileSystemRepresentation], "w+");
      GEAssert(file);
      
      fwrite(ContentData->getData(), 1, ContentData->getDataSize(), file);
      fflush(file);
      fclose(file);
   }

   std::ofstream Device::writeUserFile(const char* SubDir, const char* Name, const char* Extension)
   {
      char sFileName[64];
      sprintf(sFileName, "%s.%s", Name, Extension);
      NSString* nsFileName = [NSString stringWithUTF8String:sFileName];
      NSString* nsSubDir = [NSString stringWithUTF8String:SubDir];
      
      NSArray* nsPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
      NSString* nsDocumentsDirectory = [nsPaths objectAtIndex:0];
      NSString* nsDirectory = [nsDocumentsDirectory stringByAppendingPathComponent:nsSubDir];
      NSString* aFile = [nsDirectory stringByAppendingPathComponent:nsFileName];
      
      createDirectoryIfNonExisting(nsDirectory);
      const char* filePath = [aFile fileSystemRepresentation];
      
      return std::ofstream(filePath, std::ios::out | std::ios::binary);
   }
   
   void Device::deleteUserFile(const char* SubDir, const char* Name, const char* Extension)
   {
      char sFileName[64];
      sprintf(sFileName, "%s.%s", Name, Extension);
      NSString* nsFileName = [NSString stringWithUTF8String:sFileName];
      NSString* nsSubDir = [NSString stringWithUTF8String:SubDir];
      
      NSArray* nsPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
      NSString* nsDocumentsDirectory = [nsPaths objectAtIndex:0];
      NSString* nsDirectory = [nsDocumentsDirectory stringByAppendingPathComponent:nsSubDir];
      NSString* aFile = [nsDirectory stringByAppendingPathComponent:nsFileName];
      
      NSFileManager* nsFileManager = [NSFileManager defaultManager];
      [nsFileManager removeItemAtPath:aFile error:NULL];
   }
   
   uint Device::getUserFilesCount(const char* SubDir, const char* Extension)
   {
      NSString* nsSubDir = [NSString stringWithUTF8String:SubDir];
      NSArray* nsPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
      NSString* nsDocumentsDirectory = [nsPaths objectAtIndex:0];
      NSString* nsDirectory = [nsDocumentsDirectory stringByAppendingPathComponent:nsSubDir];
      NSArray* nsDirectoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:nsDirectory error:nil];
      NSString* nsExtension = [NSString stringWithUTF8String:Extension];
      
      uint iFilesCount = 0;
      
      for(NSString* nsFileName in nsDirectoryContents)
      {
         if([nsFileName hasSuffix:nsExtension])
            iFilesCount++;
      }
      
      return iFilesCount;
   }
   
   bool Device::getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
   {
      NSString* nsSubDir = [NSString stringWithUTF8String:SubDir];
      NSArray* nsPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
      NSString* nsDocumentsDirectory = [nsPaths objectAtIndex:0];
      NSString* nsDirectory = [nsDocumentsDirectory stringByAppendingPathComponent:nsSubDir];
      NSArray* nsDirectoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:nsDirectory error:nil];
      NSString* nsExtension = [NSString stringWithUTF8String:Extension];
      
      uint iFileIndex = 0;
      
      for(NSString* nsFileName in nsDirectoryContents)
      {
         if([nsFileName hasSuffix:nsExtension])
         {
            if(iFileIndex == Index)
            {
               NSRange nsRange = [nsFileName rangeOfString:nsExtension];
               NSString* nsFileNameWithoutExtension = [nsFileName substringToIndex:(nsRange.location - 1)];
               strcpy(Name, [nsFileNameWithoutExtension fileSystemRepresentation]);
               
               return true;
            }
            
            iFileIndex++;
         }
      }
      
      return false;
   }
   
   int Device::getNumberOfCPUCores()
   {
      return (int)[[NSProcessInfo processInfo] processorCount];
   }

   SystemLanguage Device::requestOSLanguage()
   {
      NSString* nsLanguage = [[NSLocale preferredLanguages] objectAtIndex:0];
      const char* strLanguage = [nsLanguage UTF8String];
      
      SystemLanguage systemLanguage = SystemLanguage::English;
      
      for(size_t i = 0u; i < (size_t)SystemLanguage::Count; i++)
      {
         if(strcmp(kSystemLanguageCodes[i], strLanguage) == 0)
         {
            systemLanguage = (SystemLanguage)i;
            break;
         }
      }
      
      return systemLanguage;
   }

   void Device::requestSupportedScreenResolutions(GESTLVector(Point)* pOutResolutions)
   {
   }

   void Device::showVirtualKeyboard(const char* pText)
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
      return nullptr;
   }

   void Device::openWebPage(const char* pURL)
   {
      [[UIApplication sharedApplication] openURL:
         [NSURL URLWithString:[NSString stringWithUTF8String:pURL]]
         options:@{} completionHandler:nil];
   }
}}
