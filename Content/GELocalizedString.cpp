
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
static const ObjectName LocalizedStringName("LocalizedString");
static const char* const kGlobalStringsSetFileName = "global";
static const char* kLanguageExtensions[] =
{
   "en",    // English
   "es",    // Spanish
   "de",    // German
   "fr",    // French
   "it",    // Italian
   "pt",    // Portuguese
   "ru",    // Russian
   "zh-CN", // ChineseSimplified
   "zh-TW", // ChineseTraditional
   "ja",    // Japanese
   "ko",    // Korean
};

LocalizedStringsManager::LocalizedStringsManager()
{
   ResourcesManager::getInstance()->registerObjectManager<LocalizedString>(LocalizedStringName, this);

   loadGlobalStringsSet();
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

void LocalizedStringsManager::loadStringsSetFromXml(const pugi::xml_node& pXmlRootNode)
{
   for(const pugi::xml_node& xmlString : pXmlRootNode.children("String"))
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

void LocalizedStringsManager::unloadStringsSetFromXml(const pugi::xml_node& pXmlRootNode)
{
   for(const pugi::xml_node& xmlString : pXmlRootNode.children("String"))
   {
      const char* sStringID = xmlString.attribute("id").value();

      ObjectName cStringID = ObjectName(sStringID);
      remove(cStringID);
   }
}

void LocalizedStringsManager::loadStringsSetFromStream(std::istream& pStream)
{
   const uint32_t stringsCount = (uint32_t)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
   GESTLString stringBuffer;

   for(uint32_t i = 0u; i < stringsCount; i++)
   {
      const ObjectName stringID = Value::fromStream(ValueType::ObjectName, pStream).getAsObjectName();
      GEAssert(!get(stringID));

      const size_t stringLength = (size_t)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      stringBuffer.resize(stringLength);
      pStream.read(const_cast<char*>(stringBuffer.c_str()), stringLength);

      LocalizedString* cLocaString = Allocator::alloc<LocalizedString>();
      GEInvokeCtor(LocalizedString, cLocaString)(stringID, stringBuffer.c_str());

      add(cLocaString);
   }
}

void LocalizedStringsManager::unloadStringsSetFromStream(std::istream& pStream)
{
   const uint32_t stringsCount = (uint32_t)Value::fromStream(ValueType::UShort, pStream).getAsUShort();

   for(uint32_t i = 0u; i < stringsCount; i++)
   {
      const ObjectName stringID = Value::fromStream(ValueType::ObjectName, pStream).getAsObjectName();
      GEAssert(get(stringID));
      remove(stringID);

      const size_t stringLength = (size_t)Value::fromStream(ValueType::UShort, pStream).getAsUShort();
      pStream.ignore(stringLength);
   }
}

void LocalizedStringsManager::loadGlobalStringsSet()
{
   ContentData stringsSetData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      if(Device::contentFileExists("Strings", kGlobalStringsSetFileName, "xml"))
      {
         Device::readContentFile(
            ContentType::GenericTextData, "Strings", kGlobalStringsSetFileName, "xml", &stringsSetData);

         pugi::xml_document xml;
         xml.load_buffer(stringsSetData.getData(), stringsSetData.getDataSize());
         const pugi::xml_node& xmlStrings = xml.child("Strings");

         loadStringsSetFromXml(xmlStrings);
      }
   }
   else
   {
      if(Device::contentFileExists("Strings", kGlobalStringsSetFileName, "ge"))
      {
         Device::readContentFile(
            ContentType::GenericBinaryData, "Strings", kGlobalStringsSetFileName, "ge", &stringsSetData);

         ContentDataMemoryBuffer memoryBuffer(stringsSetData);
         std::istream stream(&memoryBuffer);

         loadStringsSetFromStream(stream);
      }
   }
}

void LocalizedStringsManager::loadStringsSet(const char* Name)
{
   char extension[32];
   ContentData stringsSetData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      sprintf(extension, "%s.xml", kLanguageExtensions[(uint)Device::Language]);
      Device::readContentFile(ContentType::GenericTextData, "Strings", Name, extension, &stringsSetData);

      pugi::xml_document xml;
      xml.load_buffer(stringsSetData.getData(), stringsSetData.getDataSize());
      const pugi::xml_node& xmlStrings = xml.child("Strings");

      loadStringsSetFromXml(xmlStrings);
   }
   else
   {
      sprintf(extension, "%s.ge", kLanguageExtensions[(uint)Device::Language]);
      Device::readContentFile(ContentType::GenericBinaryData, "Strings", Name, extension, &stringsSetData);

      ContentDataMemoryBuffer memoryBuffer(stringsSetData);
      std::istream stream(&memoryBuffer);

      loadStringsSetFromStream(stream);
   }
}

void LocalizedStringsManager::unloadStringsSet(const char* Name)
{
   char extension[32];
   ContentData stringsSetData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      sprintf(extension, "%s.xml", kLanguageExtensions[(uint32_t)Device::Language]);
      Device::readContentFile(ContentType::GenericTextData, "Strings", Name, extension, &stringsSetData);

      pugi::xml_document xml;
      xml.load_buffer(stringsSetData.getData(), stringsSetData.getDataSize());
      const pugi::xml_node& xmlStrings = xml.child("Strings");

      unloadStringsSetFromXml(xmlStrings);
   }
   else
   {
      sprintf(extension, "%s.ge", kLanguageExtensions[(uint32_t)Device::Language]);
      Device::readContentFile(ContentType::GenericBinaryData, "Strings", Name, extension, &stringsSetData);

      ContentDataMemoryBuffer memoryBuffer(stringsSetData);
      std::istream stream(&memoryBuffer);

      unloadStringsSetFromStream(stream);
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
      loadStringsSet(fileNames[i].c_str());
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
