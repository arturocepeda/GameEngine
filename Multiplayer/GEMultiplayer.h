
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Multiplayer
//
//  --- GEMultiplayer.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include <stddef.h>
#include <cstdint>
#include <functional>

namespace GE { namespace Multiplayer
{
   enum class Mode
   {
      Internet,
      LAN,
      AdHoc
   };
   enum class Protocol
   {
      TCP,
      UDP
   };


   class RemoteConnection
   {
   protected:
      RemoteConnection() {}
      virtual ~RemoteConnection() {}

   public:
      virtual bool valid() const = 0;
      virtual const char* getID() const = 0;
   };


   class Server
   {
   public:
      static Server* request(Mode pMode, Protocol pProtocol);
      static void release(Server* pServer);

   protected:
      Protocol mProtocol;

      Server(Protocol pProtocol)
         : mProtocol(pProtocol)
      {
      }
      virtual ~Server()
      {
         onLocalDisconnection = nullptr;
         onRemoteDisconnection = nullptr;
      }

   public:
      std::function<void()> onLocalDisconnection;
      std::function<void(const RemoteConnection*)> onRemoteDisconnection;

      virtual void activateServer(uint16_t pPort) = 0;

      virtual void acceptClientConnection() = 0;
      virtual size_t getConnectedClientsCount() const = 0;
      virtual const RemoteConnection* getConnectedClient(size_t pIndex) const = 0;

      virtual void sendMessage(const RemoteConnection* pClient, const char* pMessage, size_t pSize) = 0;
      virtual size_t receiveMessage(RemoteConnection** pOutClient, char* pBuffer, size_t pMaxSize) = 0;
   };


   class Client
   {
   public:
      static Client* request(Mode pMode, Protocol pProtocol);
      static void release(Client* pClient);

   protected:
      Protocol mProtocol;

      Client(Protocol pProtocol)
         : mProtocol(pProtocol)
      {
      }
      virtual ~Client()
      {
         onLocalDisconnection = nullptr;
         onRemoteDisconnection = nullptr;
      }

   public:
      std::function<void()> onLocalDisconnection;
      std::function<void()> onRemoteDisconnection;

      virtual void connectToServer(const char* pID, uint16_t pPort) = 0;
      virtual bool connected() const = 0;

      virtual void sendMessage(const char* pMessage, size_t pSize) = 0;
      virtual size_t receiveMessage(char* pBuffer, size_t pMaxSize) = 0;
   };
}}
