
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
      SOCKET hSocket;
      int iStatus;

      void init();
      void release();
   };

   class ClientWinSock2 : public Client, public WinSock2
   {
   private:
      sockaddr_in sServerAddress;

   public:
      ClientWinSock2();
      ~ClientWinSock2();

      void connectToServer(const char* IP, unsigned int Port) override;

      void sendMessage(const char* Message, unsigned int Size) override;
      int receiveMessage(char* Buffer, unsigned int MaxSize) override;
   };

   class ServerWinSock2 : public Server, public WinSock2
   {
   public:
      ServerWinSock2();
      ~ServerWinSock2();

      void activeServer(unsigned int Port) override;

      void sendMessage(const SRemoteAddress& ClientAddress, const char* Message, unsigned int Size) override;
      int receiveMessage(SRemoteAddress* ClientAddress, char* Buffer, unsigned int MaxSize) override;

      char* getClientIP(unsigned int Client) override;
   };
}}
