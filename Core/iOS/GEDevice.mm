
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Device static class (iOS)
//
//  --- GEDevice.mm ---
//
//////////////////////////////////////////////////////////////////

#include "../GEDevice.h"
#include "../GEAllocator.h"
#include "Content/GEImageData.h"
#include "Content/GEAudioData.h"
#include "Types/GESTLTypes.h"
#include <fstream>

namespace GE { namespace Core
{
   using namespace Content;
   
   GESTLVector(GE::byte) pFileData;
   
   int Device::ScreenWidth = 0;
   int Device::ScreenHeight = 0;
   
   DeviceOrientation Device::Orientation = DeviceOrientation::Portrait;
   SystemLanguage Device::Language = SystemLanguage::English;
   
   int Device::getScreenWidth()
   {
      const CGSize& sScreenSize = [[UIScreen mainScreen] bounds].size;
      float fWidth = std::min(sScreenSize.width, sScreenSize.height);
      return (int)(fWidth * [[UIScreen mainScreen] scale]);
   }

   int Device::getScreenHeight()
   {
      const CGSize& sScreenSize = [[UIScreen mainScreen] bounds].size;
      float fHeight = std::max(sScreenSize.width, sScreenSize.height);
      return (int)(fHeight * [[UIScreen mainScreen] scale]);
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

   float Device::getAspectRatio()
   {
      return Orientation == DeviceOrientation::Portrait
         ? (float)getScreenHeight() / getScreenWidth()
         : (float)getScreenWidth() / getScreenHeight();
   }

   uint Device::getFileLength(const char* Filename)
   {
      std::ifstream fStream(Filename, std::ifstream::in | std::ifstream::binary);
      
      GEAssert(fStream.good());
      
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
      char sSubDir[256];
      sprintf(sSubDir, "content/%s", SubDir);
      
      NSString* nsSubDir = [NSString stringWithUTF8String:sSubDir];
      NSString* nsAppDir = [[NSBundle mainBundle] resourcePath];
      NSString* nsDirectory = [nsAppDir stringByAppendingPathComponent:nsSubDir];
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
   
   void Device::getContentFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
   {
      char sSubDir[256];
      sprintf(sSubDir, "content/%s", SubDir);
      
      NSString* nsSubDir = [NSString stringWithUTF8String:sSubDir];
      NSString* nsAppDir = [[NSBundle mainBundle] resourcePath];
      NSString* nsDirectory = [nsAppDir stringByAppendingPathComponent:nsSubDir];
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
               break;
            }
            
            iFileIndex++;
         }
      }
   }
   
   bool Device::contentFileExists(const char* SubDir, const char* Name, const char* Extension)
   {
      char sResourceName[256];
      sprintf(sResourceName, "content/%s/%s.%s", SubDir, Name, Extension);
      
      NSString* nsResourceName = [NSString stringWithUTF8String:sResourceName];
      NSString* nsFilePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:nsResourceName];
      const char* sFilePath = [nsFilePath UTF8String];
      
      return getFileLength(sFilePath) > 0;
   }

   void Device::readContentFile(ContentType Type, const char* SubDir, const char* Name, const char* Extension, ContentData* ContentData)
   {
      char sResourceName[256];   
      sprintf(sResourceName, "content/%s/%s.%s", SubDir, Name, Extension);
      
      NSString* nsResourceName = [NSString stringWithUTF8String:sResourceName];
      NSString* nsFilePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:nsResourceName];
      const char* sFilePath = [nsFilePath UTF8String];
      
      uint iFileLength = getFileLength(sFilePath);
      pFileData.resize(iFileLength);
      
      readFile(sFilePath, &pFileData[0], iFileLength);
      
      switch(Type)
      {
         case ContentType::Texture:
         case ContentType::FontTexture:
            static_cast<ImageData*>(ContentData)->load(iFileLength, (const char*)&pFileData[0]);
            break;
         case ContentType::Audio:
            static_cast<AudioData*>(ContentData)->load(iFileLength, (const char*)&pFileData[0]);
            break;
         default:
            ContentData->load(iFileLength, (const char*)&pFileData[0]);
            break;
      }
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
      
      pFileData.resize(iFileSize);
      fread(&pFileData[0], 1, iFileSize, file);
      ContentData->load(iFileSize, (const char*)&pFileData[0]);
      
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
   
   void Device::getUserFileName(const char* SubDir, const char* Extension, uint Index, char* Name)
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
               break;
            }
            
            iFileIndex++;
         }
      }
   }

   void Device::log(const char* Message, ...)
   {
     char sBuffer[256];

     va_list vArguments;
     va_start(vArguments, Message);
     vsprintf(sBuffer, Message, vArguments);
     va_end(vArguments);

     NSLog(@"%s", sBuffer);
   }
   
   int Device::getNumberOfCPUCores()
   {
      return (int)[[NSProcessInfo processInfo] processorCount];
   }
}}
