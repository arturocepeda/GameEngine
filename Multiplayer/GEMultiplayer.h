
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

namespace GE { namespace Multiplayer
{
   struct SRemoteAddress
   {
      unsigned int IP;
      unsigned int Port;
   };

   class Client
   {
   public:
      virtual ~Client();

      virtual void connectToServer(const char* IP, unsigned int Port) = 0;

      virtual void sendMessage(const char* Message, unsigned int Size) = 0;
      virtual int receiveMessage(char* Buffer, unsigned int MaxSize) = 0;
   };

   class Server
   {
   public:
      virtual ~Server();

      virtual void activeServer(unsigned int Port) = 0;

      virtual void sendMessage(const SRemoteAddress& ClientAddress, const char* Message, unsigned int Size) = 0;
      virtual int receiveMessage(SRemoteAddress* ClientAddress, char* Buffer, unsigned int MaxSize) = 0;

      virtual char* getClientIP(unsigned int Client) = 0;
   };
}}
