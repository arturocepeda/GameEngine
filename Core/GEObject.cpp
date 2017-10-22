
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEObject.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEObject.h"
#include "GEUtils.h"

using namespace GE;
using namespace GE::Core;

//
//  ObjectName
//
const ObjectName ObjectName::Empty;

ObjectName::ObjectName()
   : iID(0)
{
}

ObjectName::ObjectName(uint ID)
   : iID(ID)
{
}

ObjectName::ObjectName(const char* Name)
{
   sName = GESTLString(Name);
   iID = hash(sName);
}

ObjectName::ObjectName(const GESTLString& Name)
{
   iID = hash(Name);
   sName = Name;
}

ObjectName::ObjectName(void* Ptr)
{
   char sBuffer[32];
   sprintf(sBuffer, "0x%p", Ptr);

   sName = GESTLString(sBuffer);
   iID = hash(sName);
}

ObjectName::~ObjectName()
{
}

uint ObjectName::getID() const
{
   return iID;
}

const GESTLString& ObjectName::getString() const
{
   return sName;
}

bool ObjectName::isEmpty() const
{
   return iID == 0;
}

bool ObjectName::operator==(const ObjectName& Other) const
{
   return iID == Other.iID;
}

bool ObjectName::operator!=(const ObjectName& Other) const
{
   return iID != Other.iID;
}


//
//  Object
//
Object::Object(uint ID)
   : cName(ID)
{
}

Object::Object(const char* Name)
   : cName(Name)
{
}

Object::Object(const GESTLString& Name)
   : cName(Name)
{
}

Object::Object(const ObjectName& Name)
   : cName(Name)
{
}

Object::~Object()
{
}

const ObjectName& Object::getName() const
{
   return cName;
}


//
//  EventHandlingObject
//
EventHandlingObject::EventHandlerList EventHandlingObject::vStaticEventHandlers;

EventHandlingObject::EventHandlingObject(const ObjectName& Name)
   : Object(Name)
{
}

EventHandlingObject::~EventHandlingObject()
{
}

void EventHandlingObject::connectEventCallback(EventHandlerList& EventHandlers, const ObjectName& EventName,
                                               const ObjectName& CallbackName, const EventCallback& callback)
{
   GEAssert(callback != nullptr);

   EventHandler* sHandler = 0;

   for(uint i = 0; i < EventHandlers.size(); i++)
   {
      if(EventHandlers[i].EventName == EventName)
      {
         sHandler = &EventHandlers[i];
         break;
      }
   }

   if(!sHandler)
   {
      EventHandler sNewEventHandler;
      sNewEventHandler.EventName = EventName;
      EventHandlers.push_back(sNewEventHandler);
      sHandler = &EventHandlers[EventHandlers.size() - 1];
   }

#if defined (GE_DEVELOPMENT)
   for(uint i = 0; i < sHandler->Callbacks.size(); i++)
   {
      GEAssert(sHandler->Callbacks[i].CallbackName != CallbackName);
   }
#endif

   EventCallbackEntry sCallbackEntry;
   sCallbackEntry.CallbackName = CallbackName;
   sCallbackEntry.callback = callback;
   sHandler->Callbacks.push_back(sCallbackEntry);
}

void EventHandlingObject::disconnectEventCallback(EventHandlerList& EventHandlers, const ObjectName& EventName,
                                                  const ObjectName& CallbackName)
{
   EventHandler* sHandler = 0;

   for(uint i = 0; i < EventHandlers.size(); i++)
   {
      if(EventHandlers[i].EventName == EventName)
      {
         sHandler = &EventHandlers[i];
         break;
      }
   }

   if(!sHandler)
      return;

   for(uint i = 0; i < sHandler->Callbacks.size(); i++)
   {
      if(sHandler->Callbacks[i].CallbackName == CallbackName)
      {
         sHandler->Callbacks.erase(sHandler->Callbacks.begin() + i);
         break;
      }
   }
}

void EventHandlingObject::triggerEventStatic(const ObjectName& EventName, const EventArgs* Args)
{
   reactToEvent(vStaticEventHandlers, EventName, Args);
}

bool EventHandlingObject::reactToEvent(EventHandlerList& EventHandlers, const ObjectName& EventName, const EventArgs* Args)
{
   EventHandler* sHandler = 0;

   for(uint i = 0; i < EventHandlers.size(); i++)
   {
      if(EventHandlers[i].EventName == EventName)
      {
         sHandler = &EventHandlers[i];
         break;
      }
   }

   if(!sHandler)
      return false;

   for(uint i = 0; i < sHandler->Callbacks.size(); i++)
   {
      if(sHandler->Callbacks[i].callback(Args))
         return true;
   }

   return false;
}

void EventHandlingObject::triggerEvent(const ObjectName& EventName, const EventArgs* Args)
{
   bool bHandled = reactToEvent(vStaticEventHandlers, EventName, Args);

   if(!bHandled)
      reactToEvent(vEventHandlers, EventName, Args);
}

void EventHandlingObject::connectStaticEventCallback(const ObjectName& EventName, const ObjectName& CallbackName,
                                                     const EventCallback& callback)
{
   connectEventCallback(vStaticEventHandlers, EventName, CallbackName, callback);
}

void EventHandlingObject::disconnectStaticEventCallback(const ObjectName& EventName, const ObjectName& CallbackName)
{
   disconnectEventCallback(vStaticEventHandlers, EventName, CallbackName);
}

void EventHandlingObject::connectEventCallback(const ObjectName& EventName, const ObjectName& CallbackName,
                                               const EventCallback& callback)
{
   connectEventCallback(vEventHandlers, EventName, CallbackName, callback);
}

void EventHandlingObject::disconnectEventCallback(const ObjectName& EventName, const ObjectName& CallbackName)
{
   disconnectEventCallback(vEventHandlers, EventName, CallbackName);
}
