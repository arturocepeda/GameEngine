
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Distribution Platform (Google Play)
//
//  --- GEDistributionPlatform.GooglePlay.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDistributionPlatform.h"
#include "Core/GEDevice.h"

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
   static const char* platformName = "Google Play";
   return platformName;
}

const char* DistributionPlatform::getUserName() const
{
   static const char* defaultUserName = "";
   return defaultUserName;
}

SystemLanguage DistributionPlatform::getLanguage() const
{
   return Device::requestOSLanguage();
}

bool DistributionPlatform::internetConnectionAvailable() const
{
   //TODO
   return true;
}

bool DistributionPlatform::loggedIn() const
{
   return false;
}

void DistributionPlatform::logIn(std::function<void()> onFinished)
{
}

void DistributionPlatform::logOut()
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

void DistributionPlatform::updateLeaderboardScore(const ObjectName& pLeaderboardName, uint32_t pScore, uint32_t pScoreDetail)
{
}

void DistributionPlatform::requestLeaderboardScores(const ObjectName& pLeaderboardName, uint16_t pFirstPosition, uint16_t pLastPosition)
{
}

void DistributionPlatform::requestLeaderboardScoresAroundUser(const ObjectName& pLeaderboardName, uint16_t pPositionsCount)
{
}

bool DistributionPlatform::isDLCAvailable(const ObjectName& pDLCName) const
{
   return false;
}

void DistributionPlatform::requestDLCPurchase(const char* pURL) const
{
}

bool DistributionPlatform::processingDLCPurchaseRequest() const
{
   return false;
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

bool DistributionPlatform::isJoinOrCreateLobbyFeatureAvailable() const
{
   return false;
}

void DistributionPlatform::joinOrCreateLobby(const char*, uint32_t)
{
}

size_t DistributionPlatform::getLobbyMembersCount(const Lobby*) const
{
   return 0u;
}

bool DistributionPlatform::getLobbyMember(const Lobby*, size_t, LobbyMember*)
{
   return false;
}

void DistributionPlatform::showFullscreenAd(const char* pID)
{
}
