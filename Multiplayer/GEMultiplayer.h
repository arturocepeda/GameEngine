
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

#include <cstdint>

namespace GE { namespace Multiplayer
{
   enum class Protocol
   {
      TCP,
      UDP
   };


   struct RemoteAddress
   {
      uint32_t mIP;
      uint32_t mPort;
   };


   class Server
   {
   protected:
      bool mActive;

   public:
      Server(Protocol pProtocol);
      virtual ~Server();

      virtual void activateServer(uint32_t pPort) = 0;
      bool active() const;

      virtual void sendMessage(const RemoteAddress& pClientAddress,
         const char* pMessage, uint32_t pSize) = 0;
      virtual int receiveMessage(RemoteAddress* pClientAddress,
         char* pBuffer, uint32_t pMaxSize) = 0;

      virtual char* getClientIP(uint32_t pClient) = 0;
   };


   class Client
   {
   protected:
      bool mConnected;

   public:
      Client(Protocol pProtocol);
      virtual ~Client();

      virtual void connectToServer(const char* pIP, uint32_t pPort) = 0;
      bool connected() const;

      virtual void sendMessage(const char* pMessage, uint32_t pSize) = 0;
      virtual int receiveMessage(char* pBuffer, uint32_t pMaxSize) = 0;
   };
}}
