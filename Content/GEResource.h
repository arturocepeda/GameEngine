
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

namespace GE { namespace Content
{
   GESerializableEnum(ResourceType)
   {
      ShaderProgram,
      Texture,
      Material,
      Font,
      Mesh,
      Skeleton,
      AnimationSet,

      Count
   };


   class Resource : public Core::Object
   {
   protected:
      Core::ObjectName cGroupName;

   public:
      Resource(const Core::ObjectName& Name, const Core::ObjectName& GroupName);
      virtual ~Resource();

      const Core::ObjectName& getGroupName() const;

      virtual uint getSizeInBytes() const;
   };
}}
