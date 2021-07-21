
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Distribution Platform (Default)
//
//  --- GEDistributionPlatform.default.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDistributionPlatform.h"

using namespace GE;
using namespace GE::Core;

bool DistributionPlatform::init()
{
   return true;
}

void DistributionPlatform::update()
{
}

void DistributionPlatform::shutdown()
{
}

const char* DistributionPlatform::getPlatformName() const
{
   static const char* platformName = "Default";
   return platformName;
}

const char* DistributionPlatform::getUserName() const
{
   static const char* defaultUserName = "";
   return defaultUserName;
}

SystemLanguage DistributionPlatform::getLanguage() const
{
   return SystemLanguage::Count;
}

bool DistributionPlatform::internetConnectionAvailable() const
{
   return true;
}

bool DistributionPlatform::loggedIn() const
{
   return true;
}

void DistributionPlatform::logIn(std::function<void()>)
{
}

bool DistributionPlatform::remoteFileExists(const char* pSubDir, const char* pName, const char* pExtension)
{
   return Device::userFileExists(pSubDir, pName, pExtension);
}

bool DistributionPlatform::readRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   Content::ContentData* pContentData, std::function<void()> pOnFinished)
{
   Device::readUserFile(pSubDir, pName, pExtension, pContentData);

   if(pOnFinished)
   {
      pOnFinished();
   }

   return true;
}

bool DistributionPlatform::writeRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   const Content::ContentData* pContentData, std::function<void(bool pSuccess)> pOnFinished)
{
   Device::writeUserFile(pSubDir, pName, pExtension, pContentData);

   if(pOnFinished)
   {
      pOnFinished(true);
   }

   return true;
}

bool DistributionPlatform::deleteRemoteFile(const char* pSubDir, const char* pName, const char* pExtension)
{
   Device::deleteUserFile(pSubDir, pName, pExtension);

   return true;
}

void DistributionPlatform::setStat(const ObjectName&, float)
{
}

float DistributionPlatform::getStat(const ObjectName&)
{
   return 0.0f;
}

void DistributionPlatform::unlockAchievement(const ObjectName&)
{
}

void DistributionPlatform::updateLeaderboardScore(const ObjectName&, uint32_t, uint32_t)
{
}

void DistributionPlatform::requestLeaderboardScores(const ObjectName&, uint16_t, uint16_t)
{
}

void DistributionPlatform::requestLeaderboardScoresAroundUser(const ObjectName&, uint16_t)
{
}

void DistributionPlatform::findLobbies()
{
}

void DistributionPlatform::createLobby(const char*, uint32_t)
{
}

void DistributionPlatform::joinLobby(const Lobby*)
{
}

void DistributionPlatform::leaveLobby(const Lobby*)
{
}

size_t DistributionPlatform::getLobbyMembersCount(const Lobby*) const
{
   return 0u;
}

void DistributionPlatform::getLobbyMember(const Lobby*, size_t, LobbyMember*)
{
}
