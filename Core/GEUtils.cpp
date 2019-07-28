
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEUtils.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEUtils.h"
#include "GEAllocator.h"
#include "GESerializable.h"
#include "GEDevice.h"
#include "Types/GETypes.h"
#include "Content/GEContentData.h"

#include <sstream>

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;

//
//  Scaler
//
Scaler::Scaler(float x0, float x1, float y0, float y1)
{
   float dx = x1 - x0;
   float dy = y1 - y0;

   m = dy / dx;
   b = y0 - (m * x0);
}

Scaler::~Scaler()
{
}

float Scaler::y(float x)
{
   return (m * x) + b;
}

float Scaler::x(float y)
{
   return (y - b) / m;
}


//
//  XmlStringWriter
//
XmlStringWriter::XmlStringWriter()
   : sBuffer(0)
{
}

XmlStringWriter::~XmlStringWriter()
{
   releaseBuffer();
}

void XmlStringWriter::releaseBuffer()
{
   if(sBuffer)
      Allocator::free(sBuffer);
}

void XmlStringWriter::write(const void* data, size_t size)
{
   releaseBuffer();
   sBuffer = Allocator::alloc<char>((uint)size);
   memcpy(sBuffer, data, size);

   // set the end of the string
   char* pCursor = sBuffer + size - 1;

   while(*pCursor != '>')
      pCursor--;

   *(pCursor + 1) = '\0';
}

const char* XmlStringWriter::getData()
{
   return sBuffer;
}


//
//  SerializableIO
//
bool SerializableIO::saveToXmlFile(Serializable* Obj, const char* Directory, const char* FileName)
{
   pugi::xml_document xml;
   pugi::xml_node xmlSettings = xml.append_child(Obj->getClassName().getString());

   Obj->saveToXml(xmlSettings);

   XmlStringWriter xmlWriter;
   xml.save(xmlWriter);

   const char* xmlContent = xmlWriter.getData();
   ContentData cContentData;
   cContentData.load((uint)strlen(xmlContent), xmlContent);

   Device::writeUserFile(Directory, FileName, "xml", &cContentData);

   return true;
}

bool SerializableIO::saveToBinaryFile(Serializable* Obj, const char* Directory, const char* FileName)
{
   std::stringstream sStream;
   Obj->saveToStream(sStream);
      
   std::stringstream::pos_type iStreamSize = sStream.tellp();
   sStream.seekp(0, std::ios::beg);

   ContentData cContentData;
   cContentData.load((uint)iStreamSize, sStream);

   Device::writeUserFile(Directory, FileName, "ge", &cContentData);

   return true;
}

bool SerializableIO::loadFromXmlFile(Serializable* Obj, const char* Directory, const char* FileName)
{
   if(!Device::userFileExists(Directory, FileName, "xml"))
      return false;

   ContentData cContentData;
   Device::readUserFile(Directory, FileName, "xml", &cContentData);

   pugi::xml_document xml;
   xml.load_buffer(cContentData.getData(), cContentData.getDataSize());
   Obj->loadFromXml(xml.child(Obj->getClassName().getString()));

   return true;
}

bool SerializableIO::loadFromBinaryFile(Serializable* Obj, const char* Directory, const char* FileName)
{
   if(!Device::userFileExists(Directory, FileName, "ge"))
      return false;

   ContentData cContentData;
   Device::readUserFile(Directory, FileName, "ge", &cContentData);

   ContentDataMemoryBuffer sMemoryBuffer(cContentData);
   std::istream sStream(&sMemoryBuffer);

   Obj->loadFromStream(sStream);

   return true;
}


//
//  FNV-1a Hash
//
uint32_t GE::Core::hash(const char* pString)
{
   if(!pString || pString[0] == '\0')
      return 0u;

   const uint32_t kOffsetBasis = 2166136261u;
   const uint32_t kFNVPrime = 16777619u;

   uint32_t hash = kOffsetBasis;
   uint32_t i = 0u;

   while(pString[i] != '\0')
   {
      hash ^= pString[i];
      hash *= kFNVPrime;
      i++;
   }

   return hash;
}

bool GE::Core::isHash(const char* pString)
{
   const size_t strLength = strlen(pString);
   bool strIsHash = strLength == 8u;

   if(strIsHash)
   {
      for(size_t i = 0u; i < 8u; i++)
      {
         if(!isxdigit(pString[i]))
         {
            strIsHash = false;
            break;
         }
      }
   }

   return strIsHash;
}

void GE::Core::toHashPath(char* pPath)
{
   if(!pPath || pPath[0] == '\0')
      return;

   char sourcePath[256];
   const size_t pathLength = strlen(pPath);
   memcpy(sourcePath, pPath, pathLength + 1u);
   size_t cursorPath = 0u;

   pPath[0] = '\0';
   size_t cursorHashPath = 0u;

   const size_t kBufferSize = 64u;
   char buffer[kBufferSize];
   memset(buffer, 0, kBufferSize);
   size_t cursorBuffer = 0u;

   while(true)
   {
      if(sourcePath[cursorPath] != '/' &&
         sourcePath[cursorPath] != '\\' &&
         sourcePath[cursorPath] != '.' &&
         sourcePath[cursorPath] != '*' &&
         sourcePath[cursorPath] != '\0')
      {
         buffer[cursorBuffer++] = sourcePath[cursorPath++];
      }
      else
      {
         if(buffer[0] != '\0')
         {
            sprintf(pPath, "%s%x", pPath, hash(buffer));
            while(pPath[++cursorHashPath] != '\0');

            memset(buffer, 0, cursorBuffer);
            cursorBuffer = 0u;
         }

         pPath[cursorHashPath++] = sourcePath[cursorPath++];
         pPath[cursorHashPath] = '\0';

         if(sourcePath[cursorPath - 1u] == '\0')
            break;
      }
   }
}
