
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Multiplayer (WinSock2)
//
//  --- GEMultiplayerWinSock2.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEMultiplayerWinSock2.h"
#include <ws2tcpip.h>
#include <stdlib.h>

using namespace GE::Multiplayer;

//
//  WinSock2
//
void WinSock2::init(Protocol pProtocol)
{
   // initialize winsock
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2, 2), &wsaData);

   // create socket
   mSocket = pProtocol == Protocol::TCP
      ? socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
      : socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

   if(mSocket == INVALID_SOCKET)
   {
      return;
   }

   // set non-blocking mode
   u_long iMode = 1u;
   ioctlsocket(mSocket, FIONBIO, &iMode);
}

void WinSock2::release()
{
   closesocket(mSocket);
   WSACleanup();
}


//
//  ServerWinSock2
//
ServerWinSock2::ServerWinSock2(Protocol pProtocol)
   : Server(pProtocol)
{
   init(pProtocol);
}

ServerWinSock2::~ServerWinSock2()
{
   release();
}

void ServerWinSock2::activateServer(uint32_t pPort)
{
   mActive = false;

   sockaddr_in sServerAddress;
   memset(&sServerAddress, 0, sizeof(sServerAddress));

   sServerAddress.sin_family = AF_INET;
   sServerAddress.sin_addr.s_addr = INADDR_ANY;
   sServerAddress.sin_port = htons(pPort);

   mActive = bind(mSocket, (sockaddr*)&sServerAddress, sizeof(sServerAddress)) == 0;
}

void ServerWinSock2::sendMessage(const RemoteAddress& pClientAddress,
   const char* pMessage, uint32_t pSize)
{
   sockaddr_in sClientAddress;
   memset(&sClientAddress, 0, sizeof(sClientAddress));

   sClientAddress.sin_family = AF_INET;
   sClientAddress.sin_addr.s_addr = pClientAddress.mIP;
   sClientAddress.sin_port = htons(pClientAddress.mPort);

   sendto(mSocket, pMessage, (int)pSize, 0, (sockaddr*)&sClientAddress, sizeof(sClientAddress));
}

int ServerWinSock2::receiveMessage(RemoteAddress* pClientAddress,
   char* pBuffer, uint32_t pMaxSize)
{
   sockaddr_in clientAddress;
   int fromLen = sizeof(clientAddress);

   const int bytes =
      recvfrom(mSocket, pBuffer, (int)pMaxSize, 0, (sockaddr*)&clientAddress, &fromLen);

   if(bytes > 0)
   {
      pClientAddress->mIP = clientAddress.sin_addr.s_addr;
      pClientAddress->mPort = htons(clientAddress.sin_port);
   }

   return bytes;
}

char* ServerWinSock2::getClientIP(uint32_t pClient)
{
   static const size_t kBufferSize = 64u;
   static char ipString[kBufferSize];

   inet_ntop(AF_INET, &pClient, ipString, kBufferSize);

   return ipString;
}


//
//  ClientWinSock2
//
ClientWinSock2::ClientWinSock2(Protocol pProtocol)
   : Client(pProtocol)
{
   memset(&mServerAddress, 0, sizeof(mServerAddress));
   init(pProtocol);
}

ClientWinSock2::~ClientWinSock2()
{
   release();
}

void ClientWinSock2::connectToServer(const char* pIP, uint32_t pPort)
{
   mConnected = false;

   uint32_t binaryIP;

   if(inet_pton(AF_INET, pIP, &binaryIP))
   {
      mServerAddress.sin_family = AF_INET;
      mServerAddress.sin_addr.s_addr = binaryIP;
      mServerAddress.sin_port = htons(pPort);

      mConnected = connect(mSocket, (sockaddr*)&mServerAddress, sizeof(mServerAddress)) == 0;
   }
}

void ClientWinSock2::sendMessage(const char* pMessage, uint32_t pSize)
{
   sendto(mSocket, pMessage, (int)pSize, 0, (sockaddr*)&mServerAddress, sizeof(mServerAddress));
}

int ClientWinSock2::receiveMessage(char* pBuffer, uint32_t pMaxSize)
{
   sockaddr_in remoteAddress;
   int fromLen = sizeof(remoteAddress);

   const int bytes =
      recvfrom(mSocket, pBuffer, (int)pMaxSize, 0, (sockaddr*)&remoteAddress, &fromLen);

   if(remoteAddress.sin_addr.s_addr == mServerAddress.sin_addr.s_addr)
   {
      return bytes;
   }

   return 0;
}
