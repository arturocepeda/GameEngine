
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GELocalizedString.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GELocalizedString.h"
#include "Core/GEAllocator.h"
#include "Core/GEApplication.h"
#include "Core/GEValue.h"
#include "Content/GEResourcesManager.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;


//
//  LocalizedString
//
LocalizedString::LocalizedString(const ObjectName& Name, const char* Str)
    : Object(Name)
{
   const uint32_t stringLength = (uint32_t)strlen(Str);
   sString = Allocator::alloc<char>(stringLength + 1);
   strcpy(sString, Str);
}

LocalizedString::~LocalizedString()
{
   Allocator::free(sString);
}

const char* LocalizedString::getString() const
{
   return sString;
}


//
//  LocalizedStringsManager
//
const ObjectName LocalizedStringName("LocalizedString");

static const char* kLanguageExtensions[] =
{
   "en", // English
   "es", // Spanish
   "de", // German
};

LocalizedStringsManager::LocalizedStringsManager()
{
   ResourcesManager::getInstance()->registerObjectManager<LocalizedString>(LocalizedStringName, this);
   loadStrings();
}

void LocalizedStringsManager::getStringSetNames(FileNamesList* pOutFileNames)
{
   char extension[32];

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      sprintf(extension, "%s.xml", kLanguageExtensions[(uint)Device::Language]);
   }
   else
   {
      sprintf(extension, "%s.ge", kLanguageExtensions[(uint)Device::Language]);
   }   

   Device::getContentFileNames("Strings", extension, pOutFileNames);
}

void LocalizedStringsManager::loadStringsSet(const char* Name)
{
   char extension[32];
   ContentData cStringsSetData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      sprintf(extension, "%s.xml", kLanguageExtensions[(uint)Device::Language]);
      Device::readContentFile(ContentType::GenericTextData, "Strings", Name, extension, &cStringsSetData);

      pugi::xml_document xml;
      xml.load_buffer(cStringsSetData.getData(), cStringsSetData.getDataSize());
      const pugi::xml_node& xmlStrings = xml.child("Strings");

      for(const pugi::xml_node& xmlString : xmlStrings.children("String"))
      {
         const char* sStringID = xmlString.attribute("id").value();
         const char* sText = xmlString.attribute("text").value();

         ObjectName cStringID = ObjectName(sStringID);
         GEAssert(!get(cStringID));

         LocalizedString* cLocaString = Allocator::alloc<LocalizedString>();
         GEInvokeCtor(LocalizedString, cLocaString)(cStringID, sText);

         add(cLocaString);
      }
   }
   else
   {
      sprintf(extension, "%s.ge", kLanguageExtensions[(uint)Device::Language]);
      Device::readContentFile(ContentType::GenericBinaryData, "Strings", Name, extension, &cStringsSetData);
      ContentDataMemoryBuffer sMemoryBuffer(cStringsSetData);
      std::istream sStream(&sMemoryBuffer);

      const uint32_t stringsCount = (uint32_t)Value::fromStream(ValueType::UShort, sStream).getAsUShort();
      GESTLString stringBuffer;

      for(uint32_t i = 0u; i < stringsCount; i++)
      {
         const ObjectName stringID = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         GEAssert(!get(stringID));

         const size_t stringLength = (size_t)Value::fromStream(ValueType::UShort, sStream).getAsUShort();
         stringBuffer.resize(stringLength);
         sStream.read(const_cast<char*>(stringBuffer.c_str()), stringLength);

         LocalizedString* cLocaString = Allocator::alloc<LocalizedString>();
         GEInvokeCtor(LocalizedString, cLocaString)(stringID, stringBuffer.c_str());

         add(cLocaString);
      }
   }
}

void LocalizedStringsManager::unloadStringsSet(const char* Name)
{
   char extension[32];
   ContentData cStringsSetData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      sprintf(extension, "%s.xml", kLanguageExtensions[(uint32_t)Device::Language]);
      Device::readContentFile(ContentType::GenericTextData, "Strings", Name, extension, &cStringsSetData);

      pugi::xml_document xml;
      xml.load_buffer(cStringsSetData.getData(), cStringsSetData.getDataSize());
      const pugi::xml_node& xmlStrings = xml.child("Strings");

      for(const pugi::xml_node& xmlString : xmlStrings.children("String"))
      {
         const char* sStringID = xmlString.attribute("id").value();

         ObjectName cStringID = ObjectName(sStringID);
         remove(cStringID);
      }
   }
   else
   {
      sprintf(extension, "%s.ge", kLanguageExtensions[(uint32_t)Device::Language]);
      Device::readContentFile(ContentType::GenericBinaryData, "Strings", Name, extension, &cStringsSetData);
      ContentDataMemoryBuffer sMemoryBuffer(cStringsSetData);
      std::istream sStream(&sMemoryBuffer);

      const uint32_t iStringsCount = (uint32_t)Value::fromStream(ValueType::UShort, sStream).getAsUShort();

      for(uint32_t i = 0u; i < iStringsCount; i++)
      {
         const ObjectName cStringID = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         GEAssert(get(cStringID));
         remove(cStringID);

         const size_t stringLength = (size_t)Value::fromStream(ValueType::UShort, sStream).getAsUShort();
         sStream.ignore(stringLength);
      }
   }
}

void LocalizedStringsManager::loadStrings()
{
   FileNamesList fileNames;
   getStringSetNames(&fileNames);

   if(fileNames.empty())
   {
      Device::Language = SystemLanguage::English;
      getStringSetNames(&fileNames);
      GEAssert(!fileNames.empty());
   }

   for(size_t i = 0u; i < fileNames.size(); i++)
   {
      LocalizedStringsManager::getInstance()->loadStringsSet(fileNames[i].c_str());
   }
}

void LocalizedStringsManager::unloadStrings()
{
   FileNamesList fileNames;
   getStringSetNames(&fileNames);

   for(size_t i = 0u; i < fileNames.size(); i++)
   {
      LocalizedStringsManager::getInstance()->unloadStringsSet(fileNames[i].c_str());
   }
}

void LocalizedStringsManager::setVariable(const ObjectName& VariableName, const char* VariableValue)
{
   GESTLString sVariableValue(VariableValue);
   mVariables[VariableName.getID()] = sVariableValue;
}

const char* LocalizedStringsManager::getVariable(const ObjectName& VariableName)
{
   GESTLMap(uint32_t, GESTLString)::const_iterator it = mVariables.find(VariableName.getID());
   return it != mVariables.end() ? it->second.c_str() : nullptr;
}
