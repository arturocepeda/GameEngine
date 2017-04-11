
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
#include "Content/GEResource.h"

namespace GE { namespace Content
{
   class LocalizedString : public Resource
   {
   private:
      GESTLString sString;

   public:
      LocalizedString(const Core::ObjectName& Name, const Core::ObjectName& GroupName, const GESTLString& Str);
      ~LocalizedString();

      const GESTLString& getString() const;
   };


   class LocalizedStringsManager : public Core::ObjectManager<LocalizedString>, public Core::Singleton<LocalizedStringsManager>
   {
   public:
      LocalizedStringsManager();

      void loadStringsSet(const char* Name);
      void unloadStringsSet(const char* Name);
   };
}}
