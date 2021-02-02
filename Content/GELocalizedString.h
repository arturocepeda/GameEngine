
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GELocalizedString.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Core/GEObjectManager.h"
#include "Core/GESingleton.h"
#include "Core/GEDevice.h"
#include "Content/GEResource.h"

namespace GE { namespace Content
{
   class LocalizedString : public Core::Object
   {
   private:
      char* sString;

   public:
      LocalizedString(const Core::ObjectName& Name, const char* Str);
      ~LocalizedString();

      const char* getString() const;
   };


   class LocalizedStringsManager : public Core::ObjectManager<LocalizedString>, public Core::Singleton<LocalizedStringsManager>
   {
   private:
      GESTLMap(uint32_t, GESTLString) mVariables;
      GESTLSet(uint32_t) mGlobalStringIDs;

      void getStringSetNames(Core::FileNamesList* pOutFileNames);

      void loadStringsSetFromXml(const pugi::xml_node& pXmlRootNode);
      void unloadStringsSetFromXml(const pugi::xml_node& pXmlRootNode);

      void loadStringsSetFromStream(std::istream& pStream);
      void unloadStringsSetFromStream(std::istream& pStream);

      void loadGlobalStringsSet();

      void loadStringsSet(const char* Name);
      void unloadStringsSet(const char* Name);

   public:
      LocalizedStringsManager();

      void loadStrings();
      void unloadStrings();

      void setVariable(const Core::ObjectName& VariableName, const char* VariableValue);
      const char* getVariable(const Core::ObjectName& VariableName);

      bool isGlobal(const Core::ObjectName& pStringID) const;
   };
}}
