
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Multiplayer
//
//  --- GEMultiplayer.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEMultiplayer.h"

#include "Core/GELog.h"

#if defined (GE_PLATFORM_WINDOWS)
# include <WS2tcpip.h>
# pragma comment(lib, "ws2_32.lib")
#endif

using namespace GE::Core;
using namespace GE::Multiplayer;


//
//  RemoteConnection
//
RemoteConnection::RemoteConnection()
   : mSocket(0)
   , mIP(0u)
   , mPort(0u)
{
}

bool RemoteConnection::valid() const
{
   return mSocket > 0 || mIP > 0u;
}

const char* RemoteConnection::getIP() const
{
   static const size_t kBufferSize = 64u;
   static char ipString[kBufferSize];

   inet_ntop(AF_INET, &mIP, ipString, kBufferSize);

   return ipString;
}


//
//  Host
//
Host::Host(Protocol pProtocol)
   : mProtocol(pProtocol)
{
}

void Host::init()
{
#if defined (GE_PLATFORM_WINDOWS)
   // initialize winsock
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

   // create socket
   const int socketType = mProtocol == Protocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
   mSocket = socket(AF_INET, socketType, 0);

   if(mSocket == INVALID_SOCKET)
   {
      return;
   }

   // set non-blocking mode
   u_long iMode = 1u;
   ioctlsocket(mSocket, FIONBIO, &iMode);
}

void Host::release()
{
   closesocket(mSocket);

#if defined (GE_PLATFORM_WINDOWS)
   WSACleanup();
#endif
}


//
//  Server
//
Server::Server(Protocol pProtocol)
   : Host(pProtocol)
{
   init();
}

Server::~Server()
{
   release();
}

void Server::activateServer(uint16_t pPort)
{
   sockaddr_in address;
   memset(&address, 0, sizeof(address));

   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(pPort);

   if(bind(mSocket, (sockaddr*)&address, sizeof(address)) == 0)
   {
      Log::log(LogType::Info, "[Server] Bound successfully to the port %u", pPort);

      if(mProtocol == Protocol::TCP)
      {
         if(listen(mSocket, 1) == 0)
         {
            Log::log(LogType::Info, "[Server] Listening...");
         }
         else
         {
            Log::log(LogType::Error, "[Server] Error when trying to listen");
         }
      }
   }
   else
   {
      Log::log(LogType::Error, "[Server] Socket couldn't be bound to the port %u", pPort);
   }
}

void Server::acceptClientConnection()
{
   if(mProtocol == Protocol::TCP)
   {
      sockaddr_in remoteAddress;
      memset(&remoteAddress, 0, sizeof(sockaddr_in));
      int remoteAddressLength = (int)sizeof(remoteAddress);
      GESocket socket = accept(mSocket, (sockaddr*)&remoteAddress, &remoteAddressLength);

      if(socket != INVALID_SOCKET)
      {
         RemoteConnection connection;
         connection.mSocket = socket;
         connection.mIP = remoteAddress.sin_addr.s_addr;
         connection.mPort = htons(remoteAddress.sin_port);
         mConnectedClients.push_back(connection);

         Log::log(LogType::Info, "[Server] Established connection with client %s:%u",
            connection.getIP(), connection.mPort);
      }
   }
}

size_t Server::getConnectedClientsCount() const
{
   return mConnectedClients.size();
}

const RemoteConnection& Server::getConnectedClient(size_t pIndex) const
{
   GEAssert(pIndex < mConnectedClients.size());
   return mConnectedClients[pIndex];
}

void Server::sendMessage(const RemoteConnection& pClient, const char* pMessage, size_t pSize)
{
   if(mProtocol == Protocol::TCP)
   {
      send(pClient.mSocket, pMessage, (int)pSize, 0);
   }
   else
   {
      sockaddr_in clientAddress;
      memset(&clientAddress, 0, sizeof(clientAddress));

      clientAddress.sin_family = AF_INET;
      clientAddress.sin_addr.s_addr = pClient.mIP;
      clientAddress.sin_port = htons(pClient.mPort);

      sendto(mSocket, pMessage, (int)pSize, 0, (sockaddr*)&clientAddress, sizeof(clientAddress));
   }
}

int Server::receiveMessage(RemoteConnection* pClient, char* pBuffer, size_t pMaxSize)
{
   if(mProtocol == Protocol::TCP)
   {
      for(size_t i = 0u; i < mConnectedClients.size(); i++)
      {
         const int bytes = recv(mConnectedClients[i].mSocket, pBuffer, (int)pMaxSize, 0);

         if(bytes > 0)
         {
            pClient = &mConnectedClients[i];
            return bytes;
         }
      }
   }
   else
   {
      sockaddr_in clientAddress;
      int addressLength = (int)sizeof(clientAddress);

      const int bytes =
         recvfrom(mSocket, pBuffer, (int)pMaxSize, 0, (sockaddr*)&clientAddress, &addressLength);

      if(bytes > 0)
      {
         pClient->mIP = clientAddress.sin_addr.s_addr;
         pClient->mPort = htons(clientAddress.sin_port);

         bool clientRegistered = false;

         for(size_t i = 0u; i < mConnectedClients.size(); i++)
         {
            if(mConnectedClients[i].mIP == pClient->mIP)
            {
               clientRegistered = true;
               break;
            }
         }

         if(!clientRegistered)
         {
            mConnectedClients.push_back(*pClient);
            pClient = &mConnectedClients.back();

            Log::log(LogType::Info, "[Server] Established connection with client %s:%u",
               pClient->getIP(), pClient->mPort);
         }

         return bytes;
      }
   }

   return 0;
}


//
//  Client
//
Client::Client(Protocol pProtocol)
   : Host(pProtocol)
{
   memset(&mServerAddress, 0, sizeof(mServerAddress));
   init();
}

Client::~Client()
{
   release();
}

void Client::connectToServer(const char* pIP, uint16_t pPort)
{
   uint32_t binaryIP;

   if(inet_pton(AF_INET, pIP, &binaryIP))
   {
      mServerAddress.sin_family = AF_INET;
      mServerAddress.sin_addr.s_addr = binaryIP;
      mServerAddress.sin_port = htons(pPort);

      connect(mSocket, (sockaddr*)&mServerAddress, sizeof(mServerAddress));

      Log::log(LogType::Info, "[Client] Attempting to connect to %s:%u...", pIP, pPort);
   }
}

bool Client::connected() const
{
   fd_set set;
   set.fd_array[0] = mSocket;
   set.fd_count = 1u;

   timeval timeout;
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;

   return select(1, nullptr, &set, nullptr, &timeout) == 1;
}

void Client::sendMessage(const char* pMessage, size_t pSize)
{
   if(mProtocol == Protocol::TCP)
   {
      send(mSocket, pMessage, (int)pSize, 0);
   }
   else
   {
      sendto(mSocket, pMessage, (int)pSize, 0, (sockaddr*)&mServerAddress, sizeof(mServerAddress));
   }
}

int Client::receiveMessage(char* pBuffer, size_t pMaxSize)
{
   if(mProtocol == Protocol::TCP)
   {
      const int bytes = recv(mSocket, pBuffer, (int)pMaxSize, 0);

      if(bytes > 0)
      {
         return bytes;
      }
   }
   else
   {
      sockaddr_in remoteAddress;
      int fromLen = sizeof(remoteAddress);

      const int bytes =
         recvfrom(mSocket, pBuffer, (int)pMaxSize, 0, (sockaddr*)&remoteAddress, &fromLen);

      if(remoteAddress.sin_addr.s_addr == mServerAddress.sin_addr.s_addr)
      {
         return bytes;
      }
   }

   return 0;
}

