
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
   GESerializableEnum(ResourceType)
   {
      Texture,
      Font,
      Mesh,
      Skeleton,
      AnimationSet,
      Serializable,

      Count
   };


   class Resource : public Core::EventHandlingObject
   {
   protected:
      Core::ObjectName cGroupName;
      ResourceType eType;

   public:
      Resource(const Core::ObjectName& Name, const Core::ObjectName& GroupName, ResourceType Type);
      virtual ~Resource();

      const Core::ObjectName& getGroupName() const;
      ResourceType getType() const;

      virtual uint getSizeInBytes() const;
   };


   class SerializableResource : public Resource, public Core::Serializable
   {
   protected:
      SerializableResource(const Core::ObjectName& Name, const Core::ObjectName& GroupName, const Core::ObjectName& TypeName);

   public:
      virtual ~SerializableResource();
   };
}}
