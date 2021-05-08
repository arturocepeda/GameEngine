
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

#include <sstream>


namespace GE { namespace Multiplayer
{
   class RemoteConnectionSteam : public RemoteConnection
   {
   public:
      HSteamNetConnection mConnection;

      RemoteConnectionSteam();
      virtual ~RemoteConnectionSteam();

      virtual bool valid() const override;
      virtual const char* getID() const override;
   };


   class ServerSteam : public Server
   {
   public:
      struct ConnectionRequest
      {
         HSteamNetConnection mConnection;
      };

   private:
      HSteamListenSocket mListenSocket;
      GESTLVector(ConnectionRequest) mConnectionRequests;
      GESTLVector(RemoteConnectionSteam) mConnectedClients;

      STEAM_CALLBACK(ServerSteam, onSteamNetConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t);

   public:
      ServerSteam(Protocol pProtocol);
      virtual ~ServerSteam();

      virtual void activateServer(uint16_t pPort) override;

      void addConnectionRequest(const ConnectionRequest& pConnectionRequest);

      virtual void acceptClientConnection() override;
      virtual size_t getConnectedClientsCount() const override;
      virtual const RemoteConnection* getConnectedClient(size_t pIndex) const override;

      virtual void sendMessage(const RemoteConnection* pClient, const char* pMessage, size_t pSize) override;
      virtual size_t receiveMessage(RemoteConnection** pOutClient, char* pBuffer, size_t pMaxSize) override;
   };


   class ClientSteam : public Client
   {
   private:
      HSteamNetConnection mServerConnection;

      STEAM_CALLBACK(ClientSteam, onSteamNetConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t);

   public:
      ClientSteam(Protocol pProtocol);
      virtual ~ClientSteam();

      virtual void connectToServer(const char* pID, uint16_t pPort) override;
      virtual bool connected() const override;

      virtual void sendMessage(const char* pMessage, size_t pSize) override;
      virtual size_t receiveMessage(char* pBuffer, size_t pMaxSize) override;
   };


   static size_t receiveMessageOnConnection(HSteamNetConnection pConnection, char* pBuffer, size_t pMaxSize)
   {
      (void)pMaxSize;

      static const int kMaxMessages = 64;
      SteamNetworkingMessage_t* messages[kMaxMessages];

      const int messagesCount = SteamNetworkingSockets()->ReceiveMessagesOnConnection(pConnection, messages, kMaxMessages);

      if(messagesCount > 0)
      {
         char* cursor = pBuffer;

         for(int i = 0; i < messagesCount; i++)
         {
            const void* messageData = messages[i]->GetData();
            const size_t messageSize = (size_t)messages[i]->GetSize();

            GEAssert(((size_t)(cursor - pBuffer) + messageSize) < pMaxSize);

            memcpy(cursor, messageData, messageSize);
            cursor += messageSize;

            messages[i]->Release();
         }

         return (size_t)(cursor - pBuffer);
      }

      return 0u;
   }
}}



using namespace GE::Core;
using namespace GE::Multiplayer;



//
//  Server
//
Server* Server::request(Mode pMode, Protocol pProtocol)
{
   (void)pMode;
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
Client* Client::request(Mode pMode, Protocol pProtocol)
{
   (void)pMode;
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
   : mConnection(k_HSteamNetConnection_Invalid)
{
}

RemoteConnectionSteam::~RemoteConnectionSteam()
{
}

bool RemoteConnectionSteam::valid() const
{
   return mConnection != k_HSteamNetConnection_Invalid;
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
   , mListenSocket(k_HSteamListenSocket_Invalid)
{
}

ServerSteam::~ServerSteam()
{
   if(mListenSocket)
   {
      SteamNetworkingSockets()->CloseListenSocket(mListenSocket);
   }
}

void ServerSteam::onSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pData)
{
   if(pData->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting)
   {
      ConnectionRequest connectionRequest;
      connectionRequest.mConnection = pData->m_hConn;
      addConnectionRequest(connectionRequest);
   }
   else if(pData->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer)
   {
      for(size_t i = 0u; i < mConnectedClients.size(); i++)
      {
         if(pData->m_hConn == mConnectedClients[i].mConnection)
         {
            if(onRemoteDisconnection)
            {
               onRemoteDisconnection(&mConnectedClients[i]);
            }

            SteamNetworkingSockets()->CloseConnection(mConnectedClients[i].mConnection, 0, nullptr, false);
            mConnectedClients.erase(mConnectedClients.begin() + i);
            break;
         }
      }
   }
   else if(pData->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
   {
      if(onLocalDisconnection)
      {
         onLocalDisconnection();
      }

      for(size_t i = 0u; i < mConnectedClients.size(); i++)
      {
         SteamNetworkingSockets()->CloseConnection(mConnectedClients[i].mConnection, 0, nullptr, false);
      }

      mConnectedClients.clear();
   }
}

void ServerSteam::activateServer(uint16_t)
{
   mListenSocket = SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, nullptr);
}

void ServerSteam::addConnectionRequest(const ConnectionRequest& pConnectionRequest)
{
   mConnectionRequests.push_back(pConnectionRequest);
}

void ServerSteam::acceptClientConnection()
{
   while(!mConnectionRequests.empty())
   {
      const ConnectionRequest& connectionRequest = mConnectionRequests.front();

      if(SteamNetworkingSockets()->AcceptConnection(connectionRequest.mConnection) == k_EResultOK)
      {
         RemoteConnectionSteam remoteConnection;
         remoteConnection.mConnection = connectionRequest.mConnection;
         mConnectedClients.push_back(remoteConnection);
      }

      mConnectionRequests.erase(mConnectionRequests.begin());
   }
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
   const RemoteConnectionSteam* client = static_cast<const RemoteConnectionSteam*>(pClient);
   const int sendFlags = mProtocol == Protocol::TCP ? k_nSteamNetworkingSend_ReliableNoNagle : k_nSteamNetworkingSend_Unreliable;
   SteamNetworkingSockets()->SendMessageToConnection(client->mConnection, pMessage, (uint32_t)pSize, sendFlags, nullptr);
}

size_t ServerSteam::receiveMessage(RemoteConnection** pOutClient, char* pBuffer, size_t pMaxSize)
{
   for(size_t i = 0u; i < mConnectedClients.size(); i++)
   {
      const RemoteConnectionSteam* client = static_cast<const RemoteConnectionSteam*>(&mConnectedClients[i]);
      const size_t bytes = receiveMessageOnConnection(client->mConnection, pBuffer, pMaxSize);

      if(bytes > 0u)
      {
         *pOutClient = &mConnectedClients[i];
         return bytes;
      }
   }

   return 0u;
}


//
//  ClientSteam
//
ClientSteam::ClientSteam(Protocol pProtocol)
   : Client(pProtocol)
   , mServerConnection(k_HSteamNetConnection_Invalid)
{
}

ClientSteam::~ClientSteam()
{
   if(connected())
   {
      SteamNetworkingSockets()->CloseConnection(mServerConnection, 0, nullptr, false);
   }
}

void ClientSteam::onSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pData)
{
   if(pData->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer)
   {
      if(onRemoteDisconnection)
      {
         onRemoteDisconnection();
      }

      SteamNetworkingSockets()->CloseConnection(mServerConnection, 0, nullptr, false);
      mServerConnection = k_HSteamNetConnection_Invalid;
   }
   else if(pData->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
   {
      if(onLocalDisconnection)
      {
         onLocalDisconnection();
      }

      SteamNetworkingSockets()->CloseConnection(mServerConnection, 0, nullptr, false);
      mServerConnection = k_HSteamNetConnection_Invalid;
   }
}

void ClientSteam::connectToServer(const char* pID, uint16_t)
{
   uint64_t id;
   std::istringstream iss(pID);
   iss >> id;

   CSteamID serverSteamID;
   serverSteamID.SetFromUint64(id);

   SteamNetworkingIdentity serverIdentity;
   serverIdentity.SetSteamID(serverSteamID);

   mServerConnection = SteamNetworkingSockets()->ConnectP2P(serverIdentity, 0, 0, nullptr);
}

bool ClientSteam::connected() const
{
   return mServerConnection != k_HSteamNetConnection_Invalid;
}

void ClientSteam::sendMessage(const char* pMessage, size_t pSize)
{
   const int sendFlags = mProtocol == Protocol::TCP ? k_nSteamNetworkingSend_ReliableNoNagle : k_nSteamNetworkingSend_Unreliable;
   SteamNetworkingSockets()->SendMessageToConnection(mServerConnection, pMessage, (uint32_t)pSize, sendFlags, nullptr);
}

size_t ClientSteam::receiveMessage(char* pBuffer, size_t pMaxSize)
{
   return receiveMessageOnConnection(mServerConnection, pBuffer, pMaxSize);
}

