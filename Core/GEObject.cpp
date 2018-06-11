
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
//  ObjectNameRegistry
//
ObjectNameRegistry::Registry* ObjectNameRegistry::mRegistry = 0;
char* ObjectNameRegistry::sStringBuffer = 0;
char* ObjectNameRegistry::sStringBufferPtr = 0;

void ObjectNameRegistry::registerString(uint Hash, const char* String)
{
   if(!mRegistry)
   {
      mRegistry = Allocator::alloc<Registry>();
      GEInvokeCtor(Registry, mRegistry);

      sStringBuffer = Allocator::alloc<char>(1 * 1024 * 1024);
      sStringBuffer[0] = '\0';

      (*mRegistry)[0] = sStringBuffer;
      sStringBufferPtr = sStringBuffer + 1;
   }

   if(mRegistry->find(Hash) == mRegistry->end())
   {
      (*mRegistry)[Hash] = sStringBufferPtr;
      strcpy(sStringBufferPtr, String);
      sStringBufferPtr += strlen(String) + 1;
   }
#if defined (GE_EDITOR_SUPPORT)
   else
   {
      GEAssert(strcmp(String, retrieveString(Hash)) == 0);
   }
#endif
}

const char* ObjectNameRegistry::retrieveString(uint Hash)
{
   Registry::const_iterator it = mRegistry->find(Hash);
   return it != mRegistry->end()
      ? it->second
      : sStringBuffer;
}


//
//  ObjectName
//
const ObjectName ObjectName::Empty;

ObjectName::ObjectName()
   : iID(0)
#if defined (GE_EDITOR_SUPPORT)
   , sString(0)
#endif
{
}

ObjectName::ObjectName(uint ID)
   : iID(ID)
#if defined (GE_EDITOR_SUPPORT)
   , sString(0)
#endif
{
}

ObjectName::ObjectName(const char* Name)
{
   iID = hash(Name);
   ObjectNameRegistry::registerString(iID, Name);
#if defined (GE_EDITOR_SUPPORT)
   sString = (char*)ObjectNameRegistry::retrieveString(iID);
#endif
}

ObjectName::ObjectName(void* Ptr)
#if defined (GE_EDITOR_SUPPORT)
   : sString(0)
#endif
{
   char sBuffer[32];
   sprintf(sBuffer, "0x%p", Ptr);

   iID = hash(sBuffer);
}

ObjectName::~ObjectName()
{
}

uint ObjectName::getID() const
{
   return iID;
}

const char* ObjectName::getString() const
{
   return ObjectNameRegistry::retrieveString(iID);
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
