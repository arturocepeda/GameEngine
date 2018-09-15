
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEObject.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Types/GESTLTypes.h"

#include <string>
#include <functional>

namespace GE { namespace Core
{
   class ObjectNameRegistry
   {
   private:
      typedef GESTLMap(uint, const char*) Registry;

      static Registry* mRegistry;
      static char* sStringBuffer;
      static char* sStringBufferPtr;

   public:
      static void registerString(uint Hash, const char* String);
      static const char* retrieveString(uint Hash);
   };


   class ObjectName
   {
   private:
      uint iID;
#if defined (GE_EDITOR_SUPPORT)
      char* sString;
#endif

   public:
      static const ObjectName Empty;

      ObjectName();
      ObjectName(uint ID);
      ObjectName(const char* Name);
      ObjectName(void* Ptr);
      ~ObjectName();

      uint getID() const;
      const char* getString() const;

      bool isEmpty() const;

      bool operator==(const ObjectName& Other) const;
      bool operator!=(const ObjectName& Other) const;
   };


   class Object
   {
   protected:
      ObjectName cName;

   public:
      Object(uint ID);
      Object(const char* Name);
      Object(const ObjectName& Name);
      ~Object();

      const ObjectName& getName() const;
   };


   struct EventArgs
   {
      Object* Sender;
      void* Data;

      EventArgs() : Sender(0), Data(0) {}
   };


   typedef std::function<bool(const EventArgs*)> EventCallback;


   struct EventCallbackEntry
   {
      ObjectName CallbackName;
      EventCallback callback;
   };


   struct EventHandler
   {
      ObjectName EventName;
      GESTLVector(EventCallbackEntry) Callbacks;
   };


   class EventHandlingObject : public Object
   {
   protected:
      typedef GESTLVector(EventHandler) EventHandlerList;

      static EventHandlerList vStaticEventHandlers;
      EventHandlerList vEventHandlers;

      static void connectEventCallback(EventHandlerList& EventHandlers, const ObjectName& EventName,
         const ObjectName& CallbackName, const EventCallback& callback);
      static void disconnectEventCallback(EventHandlerList& EventHandlers, const ObjectName& EventName,
         const ObjectName& CallbackName);

      static bool reactToEvent(EventHandlerList& EventHandlers, const ObjectName& EventName, const EventArgs* Args);

   public:
      EventHandlingObject(const ObjectName& Name);
      ~EventHandlingObject();

      static void connectStaticEventCallback(const ObjectName& EventName, const ObjectName& CallbackName,
         const EventCallback& callback);
      static void disconnectStaticEventCallback(const ObjectName& EventName, const ObjectName& CallbackName);

      static void triggerEventStatic(const ObjectName& EventName, const EventArgs* Args = 0);

      void connectEventCallback(const ObjectName& EventName, const ObjectName& CallbackName,
         const EventCallback& callback);
      void disconnectEventCallback(const ObjectName& EventName, const ObjectName& CallbackName);

      void triggerEvent(const ObjectName& EventName, const EventArgs* Args = 0);
   };
}}
