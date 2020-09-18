
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Multiplayer (WinSock2)
//
//  --- GEMultiplayerWinSock2.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Multiplayer/GEMultiplayer.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

namespace GE { namespace Multiplayer
{
   class WinSock2
   {
   protected:
      SOCKET mSocket;

      void init(Protocol pProtocol);
      void release();
   };


   class ServerWinSock2 : public Server, public WinSock2
   {
   public:
      ServerWinSock2(Protocol pProtocol);
      virtual ~ServerWinSock2();

      virtual void activateServer(uint32_t pPort) override;

      virtual void sendMessage(const RemoteAddress& pClientAddress,
         const char* pMessage, uint32_t pSize) override;
      virtual int receiveMessage(RemoteAddress* pClientAddress,
         char* pBuffer, uint32_t pMaxSize) override;

      virtual char* getClientIP(uint32_t pClient) override;
   };


   class ClientWinSock2 : public Client, public WinSock2
   {
   private:
      sockaddr_in mServerAddress;

   public:
      ClientWinSock2(Protocol pProtocol);
      virtual ~ClientWinSock2();

      virtual void connectToServer(const char* pIP, uint32_t pPort) override;

      virtual void sendMessage(const char* pMessage, uint32_t pSize) override;
      virtual int receiveMessage(char* pBuffer, uint32_t pMaxSize) override;
   };
}}
