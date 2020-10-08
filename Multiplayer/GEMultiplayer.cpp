
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

#include "Types/GESTLTypes.h"
#include "Core/GEPlatform.h"
#include "Core/GELog.h"

#if defined (GE_PLATFORM_WINDOWS)
# include <winsock2.h>
# include <WS2tcpip.h>

# pragma comment(lib, "ws2_32.lib")

# define GESocket SOCKET
#else
# include <sys/socket.h>
# include <arpa/inet.h>

# define GESocket int
#endif


namespace GE { namespace Multiplayer
{
   class RemoteConnectionSocket : public RemoteConnection
   {
   public:
      GESocket mSocket;
      uint32_t mIP;
      uint16_t mPort;

      RemoteConnectionSocket();
      virtual ~RemoteConnectionSocket();

      virtual bool valid() const override;
      virtual const char* getID() const override;
   };


   class Host
   {
   protected:
      GESocket mSocket;

      Host(Protocol pProtocol);
      ~Host();
   };


   class ServerSocket : public Server, protected Host
   {
   private:
      GESTLVector(RemoteConnectionSocket) mConnectedClients;

   public:
      ServerSocket(Protocol pProtocol);
      virtual ~ServerSocket();

      virtual void activateServer(uint16_t pPort) override;

      virtual void acceptClientConnection() override;
      virtual size_t getConnectedClientsCount() const override;
      virtual const RemoteConnection* getConnectedClient(size_t pIndex) const override;

      virtual void sendMessage(const RemoteConnection* pClient, const char* pMessage, size_t pSize) override;
      virtual size_t receiveMessage(RemoteConnection** pOutClient, char* pBuffer, size_t pMaxSize) override;
   };


   class ClientSocket : public Client, protected Host
   {
   private:
      sockaddr_in mServerAddress;

   public:
      ClientSocket(Protocol pProtocol);
      virtual ~ClientSocket();

      virtual void connectToServer(const char* pID, uint16_t pPort) override;
      virtual bool connected() const override;

      virtual void sendMessage(const char* pMessage, size_t pSize) override;
      virtual size_t receiveMessage(char* pBuffer, size_t pMaxSize) override;
   };
}}



using namespace GE::Core;
using namespace GE::Multiplayer;


//
//  Server
//
Server* Server::request(Protocol pProtocol)
{
   ServerSocket* server = Allocator::alloc<ServerSocket>();
   GEInvokeCtor(ServerSocket, server)(pProtocol);
   return server;
}

void Server::release(Server* pServer)
{
   GEInvokeDtor(Server, pServer);
   Allocator::free(pServer);
}


//
//  Client
//
Client* Client::request(Protocol pProtocol)
{
   ClientSocket* client = Allocator::alloc<ClientSocket>();
   GEInvokeCtor(ClientSocket, client)(pProtocol);
   return client;
}

void Client::release(Client* pClient)
{
   GEInvokeDtor(Client, pClient);
   Allocator::free(pClient);
}


//
//  RemoteConnectionSocket
//
RemoteConnectionSocket::RemoteConnectionSocket()
   : mSocket(0)
   , mIP(0u)
   , mPort(0u)
{
}

RemoteConnectionSocket::~RemoteConnectionSocket()
{
}

bool RemoteConnectionSocket::valid() const
{
   return mSocket > 0 || mIP > 0u;
}

const char* RemoteConnectionSocket::getID() const
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
{
#if defined (GE_PLATFORM_WINDOWS)
   // initialize winsock
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

   // create socket
   const int socketType = pProtocol == Protocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
   mSocket = socket(AF_INET, socketType, 0);

   if(mSocket != INVALID_SOCKET)
   {
      // set non-blocking mode
      u_long mode = 1u;
      ioctlsocket(mSocket, FIONBIO, &mode);
   }
}

Host::~Host()
{
   if(mSocket != INVALID_SOCKET)
   {
      closesocket(mSocket);
   }

#if defined (GE_PLATFORM_WINDOWS)
   WSACleanup();
#endif
}


//
//  ServerSocket
//
ServerSocket::ServerSocket(Protocol pProtocol)
   : Server(pProtocol)
   , Host(pProtocol)
{
}

ServerSocket::~ServerSocket()
{
}

void ServerSocket::activateServer(uint16_t pPort)
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

void ServerSocket::acceptClientConnection()
{
   if(mProtocol == Protocol::TCP)
   {
      sockaddr_in remoteAddress;
      memset(&remoteAddress, 0, sizeof(sockaddr_in));
      int remoteAddressLength = (int)sizeof(remoteAddress);
      GESocket socket = accept(mSocket, (sockaddr*)&remoteAddress, &remoteAddressLength);

      if(socket != INVALID_SOCKET)
      {
         RemoteConnectionSocket connection;
         connection.mSocket = socket;
         connection.mIP = remoteAddress.sin_addr.s_addr;
         connection.mPort = htons(remoteAddress.sin_port);
         mConnectedClients.push_back(connection);

         Log::log(LogType::Info, "[Server] Established connection with client %s:%u",
            connection.getID(), connection.mPort);
      }
   }
}

size_t ServerSocket::getConnectedClientsCount() const
{
   return mConnectedClients.size();
}

const RemoteConnection* ServerSocket::getConnectedClient(size_t pIndex) const
{
   GEAssert(pIndex < mConnectedClients.size());
   return &mConnectedClients[pIndex];
}

void ServerSocket::sendMessage(const RemoteConnection* pClient, const char* pMessage, size_t pSize)
{
   const RemoteConnectionSocket* client = static_cast<const RemoteConnectionSocket*>(pClient);

   if(mProtocol == Protocol::TCP)
   {
      send(client->mSocket, pMessage, (int)pSize, 0);
   }
   else
   {
      sockaddr_in clientAddress;
      memset(&clientAddress, 0, sizeof(clientAddress));

      clientAddress.sin_family = AF_INET;
      clientAddress.sin_addr.s_addr = client->mIP;
      clientAddress.sin_port = htons(client->mPort);

      sendto(mSocket, pMessage, (int)pSize, 0, (sockaddr*)&clientAddress, sizeof(clientAddress));
   }
}

size_t ServerSocket::receiveMessage(RemoteConnection** pOutClient, char* pBuffer, size_t pMaxSize)
{
   if(mProtocol == Protocol::TCP)
   {
      for(size_t i = 0u; i < mConnectedClients.size(); i++)
      {
         const int bytes = recv(mConnectedClients[i].mSocket, pBuffer, (int)pMaxSize, 0);

         if(bytes > 0)
         {
            *pOutClient = &mConnectedClients[i];
            return (size_t)bytes;
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
         const uint32_t ip = clientAddress.sin_addr.s_addr;
         const uint16_t port = htons(clientAddress.sin_port);

         bool clientRegistered = false;

         for(size_t i = 0u; i < mConnectedClients.size(); i++)
         {
            if(mConnectedClients[i].mIP == ip)
            {
               clientRegistered = true;
               break;
            }
         }

         if(!clientRegistered)
         {
            RemoteConnectionSocket connection;
            connection.mIP = ip;
            connection.mPort = port;
            mConnectedClients.push_back(connection);

            *pOutClient = &mConnectedClients.back();

            Log::log(LogType::Info, "[Server] Established connection with client %s:%u", ip, port);
         }

         return (size_t)bytes;
      }
   }

   return 0u;
}


//
//  ClientSocket
//
ClientSocket::ClientSocket(Protocol pProtocol)
   : Client(pProtocol)
   , Host(pProtocol)
{
   memset(&mServerAddress, 0, sizeof(mServerAddress));
}

ClientSocket::~ClientSocket()
{
}

void ClientSocket::connectToServer(const char* pID, uint16_t pPort)
{
   uint32_t binaryIP;

   if(inet_pton(AF_INET, pID, &binaryIP))
   {
      mServerAddress.sin_family = AF_INET;
      mServerAddress.sin_addr.s_addr = binaryIP;
      mServerAddress.sin_port = htons(pPort);

      connect(mSocket, (sockaddr*)&mServerAddress, sizeof(mServerAddress));

      Log::log(LogType::Info, "[Client] Attempting to connect to %s:%u...", pID, pPort);
   }
}

bool ClientSocket::connected() const
{
   fd_set set;
   set.fd_array[0] = mSocket;
   set.fd_count = 1u;

   timeval timeout;
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;

   return select(1, nullptr, &set, nullptr, &timeout) == 1;
}

void ClientSocket::sendMessage(const char* pMessage, size_t pSize)
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

size_t ClientSocket::receiveMessage(char* pBuffer, size_t pMaxSize)
{
   if(mProtocol == Protocol::TCP)
   {
      const int bytes = recv(mSocket, pBuffer, (int)pMaxSize, 0);

      if(bytes > 0)
      {
         return (size_t)bytes;
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
         return (size_t)bytes;
      }
   }

   return 0u;
}

