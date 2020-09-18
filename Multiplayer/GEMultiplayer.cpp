
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

using namespace GE::Multiplayer;


//
//  Server
//
Server::Server(Protocol)
   : mActive(false)
{
}

Server::~Server()
{
}

bool Server::active() const
{
   return mActive;
}


//
//  Client
//
Client::Client(Protocol)
   : mConnected(false)
{
}

Client::~Client()
{
}

bool Client::connected() const
{
   return mConnected;
}

