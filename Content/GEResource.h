
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
      LocalizedString,

      Count
   };


   class Resource : public Core::EventHandlingObject
   {
   protected:
      Core::ObjectName cGroupName;
      ResourceType eType;

   public:
      static Core::ObjectName EventResourceCreated;
      static Core::ObjectName EventResourceDestroyed;

      Resource(const Core::ObjectName& Name, const Core::ObjectName& GroupName, ResourceType Type);
      virtual ~Resource();

      const Core::ObjectName& getGroupName() const;
      ResourceType getType() const;

      virtual uint getSizeInBytes() const;
   };
}}
