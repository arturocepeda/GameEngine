
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Multiplayer (Steam)
//
//  --- GEMultiplayer.Steam.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Multiplayer/GEMultiplayer.h"

#include "Types/GESTLTypes.h"
#include "Core/GELog.h"

#include "Externals/steamworks_sdk/public/steam/steam_api.h"


namespace GE { namespace Multiplayer
{
   class RemoteConnectionSteam : public RemoteConnection
   {
   public:
      RemoteConnectionSteam();
      virtual ~RemoteConnectionSteam();

      virtual bool valid() const override;
      virtual const char* getID() const override;
   };


   class ServerSteam : public Server
   {
   private:
      GESTLVector(RemoteConnectionSteam) mConnectedClients;

   public:
      ServerSteam(Protocol pProtocol);
      virtual ~ServerSteam();

      virtual void activateServer(uint16_t pPort) override;

      virtual void acceptClientConnection() override;
      virtual size_t getConnectedClientsCount() const override;
      virtual const RemoteConnection* getConnectedClient(size_t pIndex) const override;

      virtual void sendMessage(const RemoteConnection* pClient, const char* pMessage, size_t pSize) override;
      virtual size_t receiveMessage(RemoteConnection** pOutClient, char* pBuffer, size_t pMaxSize) override;
   };


   class ClientSteam : public Client
   {
   public:
      ClientSteam(Protocol pProtocol);
      virtual ~ClientSteam();

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
   ServerSteam* server = Allocator::alloc<ServerSteam>();
   GEInvokeCtor(ServerSteam, server)(pProtocol);
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
   ClientSteam* client = Allocator::alloc<ClientSteam>();
   GEInvokeCtor(ClientSteam, client)(pProtocol);
   return client;
}

void Client::release(Client* pClient)
{
   GEInvokeDtor(Client, pClient);
   Allocator::free(pClient);
}


//
//  RemoteConnectionSteam
//
RemoteConnectionSteam::RemoteConnectionSteam()
{
}

RemoteConnectionSteam::~RemoteConnectionSteam()
{
}

bool RemoteConnectionSteam::valid() const
{
   return false;
}

const char* RemoteConnectionSteam::getID() const
{
   return "";
}


//
//  ServerSteam
//
ServerSteam::ServerSteam(Protocol pProtocol)
   : Server(pProtocol)
{
}

ServerSteam::~ServerSteam()
{
}

void ServerSteam::activateServer(uint16_t pPort)
{
   (void)pPort;
}

void ServerSteam::acceptClientConnection()
{
}

size_t ServerSteam::getConnectedClientsCount() const
{
   return mConnectedClients.size();
}

const RemoteConnection* ServerSteam::getConnectedClient(size_t pIndex) const
{
   GEAssert(pIndex < mConnectedClients.size());
   return &mConnectedClients[pIndex];
}

void ServerSteam::sendMessage(const RemoteConnection* pClient, const char* pMessage, size_t pSize)
{
   (void)pClient;
   (void)pMessage;
   (void)pSize;
}

size_t ServerSteam::receiveMessage(RemoteConnection** pOutClient, char* pBuffer, size_t pMaxSize)
{
   (void)pOutClient;
   (void)pBuffer;
   (void)pMaxSize;

   return 0u;
}


//
//  ClientSteam
//
ClientSteam::ClientSteam(Protocol pProtocol)
   : Client(pProtocol)
{
}

ClientSteam::~ClientSteam()
{
}

void ClientSteam::connectToServer(const char* pID, uint16_t pPort)
{
   (void)pID;
   (void)pPort;
}

bool ClientSteam::connected() const
{
   return false;
}

void ClientSteam::sendMessage(const char* pMessage, size_t pSize)
{
   (void)pMessage;
   (void)pSize;
}

size_t ClientSteam::receiveMessage(char* pBuffer, size_t pMaxSize)
{
   (void)pBuffer;
   (void)pMaxSize;

   return 0u;
}

