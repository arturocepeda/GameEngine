
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEResource.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEResource.h"
#include "Core/GELog.h"
#include "Core/GEEvents.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;


//
//  Resource
//
Resource::Resource(const ObjectName& Name, const ObjectName& GroupName, ResourceType Type)
   : EventHandlingObject(Name)
   , cGroupName(GroupName)
   , eType(Type)
{
   EventArgs sArgs;
   sArgs.Sender = this;
   triggerEventStatic(Events::ResourceCreated, &sArgs);

   if(eType != ResourceType::Serializable)
   {
      Log::log(LogType::Info, "Resource created (%s): '%s'", strResourceType[(int)eType], cName.getString().c_str());
   }
}

Resource::~Resource()
{
   EventArgs sArgs;
   sArgs.Sender = this;
   triggerEventStatic(Events::ResourceDestroyed, &sArgs);

   if(eType != ResourceType::Serializable)
   {
      Log::log(LogType::Info, "Resource destroyed (%s): '%s'", strResourceType[(int)eType], cName.getString().c_str());
   }
}

const Core::ObjectName& Resource::getGroupName() const
{
   return cGroupName;
}

ResourceType Resource::getType() const
{
   return eType;
}

uint Resource::getSizeInBytes() const
{
   return 0;
}


//
//  SerializableResource
//
SerializableResource::SerializableResource(const ObjectName& Name, const ObjectName& GroupName, const ObjectName& TypeName)
   : Resource(Name, GroupName, ResourceType::Serializable)
   , Serializable(TypeName)
{
   Log::log(LogType::Info, "Resource created (%s): '%s'", getClassName().getString().c_str(), cName.getString().c_str());
}

SerializableResource::~SerializableResource()
{
   Log::log(LogType::Info, "Resource destroyed (%s): '%s'", getClassName().getString().c_str(), cName.getString().c_str());
}

void SerializableResource::setName(const Core::ObjectName& Name)
{
   cName = Name;
}
