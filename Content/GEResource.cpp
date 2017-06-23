
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
#include "Core/GEDevice.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;


//
//  Resource
//
ObjectName Resource::EventResourceCreated = ObjectName("EventResourceCreated");
ObjectName Resource::EventResourceDestroyed = ObjectName("EventResourceDestroyed");

Resource::Resource(const ObjectName& Name, const ObjectName& GroupName, ResourceType Type)
   : EventHandlingObject(Name)
   , cGroupName(GroupName)
   , eType(Type)
{
   EventArgs sArgs;
   sArgs.Sender = this;
   triggerEventStatic(EventResourceCreated, &sArgs);

   Device::log("Resource created (%s): '%s'", strResourceType[(int)eType], cName.getString().c_str());
}

Resource::~Resource()
{
   EventArgs sArgs;
   sArgs.Sender = this;
   triggerEventStatic(EventResourceDestroyed, &sArgs);

   Device::log("Resource destroyed (%s): '%s'", strResourceType[(int)eType], cName.getString().c_str());
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
