
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
   class ObjectName
   {
   private:
      uint iID;
      GESTLString sName;

   public:
      static const ObjectName Empty;

      ObjectName();
      ObjectName(uint ID);
      ObjectName(const char* Name);
      ObjectName(const GESTLString& Name);
      ObjectName(void* Ptr);
      ~ObjectName();

      uint getID() const;
      const GESTLString& getString() const;
      const char* getCString() const;

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
      Object(const GESTLString& Name);
      Object(const ObjectName& Name);
      ~Object();

      const ObjectName& getName() const;
   };


   struct EventArgs
   {
      Object* Sender;
      void* Args;

      EventArgs() : Sender(0), Args(0) {}
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
