
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
#include "Core/GEDevice.h"
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
const ObjectName LocalizedStringName = ObjectName("LocalizedString");

LocalizedStringsManager::LocalizedStringsManager()
{
   ResourcesManager::getInstance()->registerObjectManager<LocalizedString>(LocalizedStringName, this);
}

void LocalizedStringsManager::loadStringsSet(const char* Name)
{
   char sFileName[64];
   sprintf(sFileName, "%s.%s", Name, strSystemLanguage[(uint)Device::Language]);

   ContentData cStringsSetData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      if(!Device::contentFileExists("Strings", sFileName, "xml"))
         sprintf(sFileName, "%s.en", Name);

      Device::readContentFile(ContentType::GenericTextData, "Strings", sFileName, "xml", &cStringsSetData);

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
      if(!Device::contentFileExists("Strings", sFileName, "ge"))
         sprintf(sFileName, "%s.en", Name);

      Device::readContentFile(ContentType::GenericBinaryData, "Strings", sFileName, "ge", &cStringsSetData);
      ContentDataMemoryBuffer sMemoryBuffer(cStringsSetData);
      std::istream sStream(&sMemoryBuffer);

      uint iStringsCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iStringsCount; i++)
      {
         ObjectName cStringID = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         GEAssert(!get(cStringID));

         LocalizedString* cLocaString = Allocator::alloc<LocalizedString>();
         GEInvokeCtor(LocalizedString, cLocaString)(cStringID, Value::fromStream(ValueType::String, sStream).getAsString());

         add(cLocaString);
      }
   }
}

void LocalizedStringsManager::unloadStringsSet(const char* Name)
{
   char sFileName[64];
   sprintf(sFileName, "%s.%s", Name, strSystemLanguage[(uint)Device::Language]);

   ContentData cStringsSetData;

   if(Application::ContentType == ApplicationContentType::Xml)
   {
      Device::readContentFile(ContentType::GenericTextData, "Strings", sFileName, "xml", &cStringsSetData);

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
      Device::readContentFile(ContentType::GenericBinaryData, "Strings", sFileName, "ge", &cStringsSetData);
      ContentDataMemoryBuffer sMemoryBuffer(cStringsSetData);
      std::istream sStream(&sMemoryBuffer);

      uint iStringsCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      for(uint i = 0; i < iStringsCount; i++)
      {
         ObjectName cStringID = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
         GEAssert(get(cStringID));
         remove(cStringID);
      }
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

   if(it == mVariables.end())
      return 0;

   return it->second.c_str();
}
