
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

#include "Types/GESTLTypes.h"
#include "Core/GEPlatform.h"

#include <cstdint>

#if defined (GE_PLATFORM_WINDOWS)
# include <winsock2.h>
#else
# include <sys/socket.h>
# include <arpa/inet.h>
#endif

#if defined (GE_PLATFORM_WINDOWS)
# define GESocket SOCKET
#else
# define GESocket int
#endif

namespace GE { namespace Multiplayer
{
   enum class Protocol
   {
      TCP,
      UDP
   };


   struct RemoteConnection
   {
      GESocket mSocket;
      uint32_t mIP;
      uint16_t mPort;

      RemoteConnection();

      bool valid() const;
   };


   class Host
   {
   protected:
      GESocket mSocket;
      Protocol mProtocol;

      Host(Protocol pProtocol);

      void init();
      void release();
   };


   class Server : public Host
   {
   private:
      GESTLVector(RemoteConnection) mConnectedClients;

   public:
      Server(Protocol pProtocol);
      virtual ~Server();

      void activateServer(uint16_t pPort);

      void acceptClientConnection();
      size_t getConnectedClientsCount() const;
      const RemoteConnection& getConnectedClient(size_t pIndex) const;

      void sendMessage(const RemoteConnection& pClient, const char* pMessage, size_t pSize);
      int receiveMessage(RemoteConnection* pClient, char* pBuffer, size_t pMaxSize);

      char* getClientIP(uint32_t pClient);
   };


   class Client : public Host
   {
   private:
      sockaddr_in mServerAddress;

   public:
      Client(Protocol pProtocol);
      virtual ~Client();

      void connectToServer(const char* pIP, uint16_t pPort);
      bool connected() const;

      void sendMessage(const char* pMessage, size_t pSize);
      int receiveMessage(char* pBuffer, size_t pMaxSize);
   };
}}
