
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEResource.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEObject.h"
#include "Core/GESerializable.h"

namespace GE { namespace Content
{
   class Resource : public Core::EventHandlingObject, public Core::Serializable
   {
   protected:
      Core::ObjectName cGroupName;

   public:
      Resource(const Core::ObjectName& Name, const Core::ObjectName& GroupName, const Core::ObjectName& TypeName);
      virtual ~Resource();

      void setName(const Core::ObjectName& Name) { cName = Name; }
      const Core::ObjectName& getGroupName() const { return cGroupName; }

      virtual uint getSizeInBytes() const;
   };
}}
