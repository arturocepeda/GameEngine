
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


Resource::Resource(const ObjectName& Name, const ObjectName& GroupName, const ObjectName& TypeName)
   : EventHandlingObject(Name)
   , Serializable(TypeName)
   , cGroupName(GroupName)
{
   EventArgs sArgs;
   sArgs.Sender = this;
   triggerEventStatic(Events::ResourceCreated, &sArgs);

   Log::log(LogType::Info, "Resource created (%s): '%s'", getClassName().getString(), cName.getString());
}

Resource::~Resource()
{
   EventArgs sArgs;
   sArgs.Sender = this;
   triggerEventStatic(Events::ResourceDestroyed, &sArgs);

   Log::log(LogType::Info, "Resource destroyed (%s): '%s'", getClassName().getString(), cName.getString());
}

uint Resource::getSizeInBytes() const
{
   return 0;
}
