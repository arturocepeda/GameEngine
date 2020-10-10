
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

bool DistributionPlatform::init() const
{
   return true;
}

void DistributionPlatform::update() const
{
}

void DistributionPlatform::shutdown() const
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

bool DistributionPlatform::remoteFileExists(const char* pSubDir, const char* pName, const char* pExtension)
{
   return Device::userFileExists(pSubDir, pName, pExtension);
}

void DistributionPlatform::readRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   Content::ContentData* pContentData, std::function<void()> pOnFinished)
{
   Device::readUserFile(pSubDir, pName, pExtension, pContentData);

   if(pOnFinished)
   {
      pOnFinished();
   }
}

void DistributionPlatform::writeRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   const Content::ContentData* pContentData, std::function<void(bool pSuccess)> pOnFinished)
{
   Device::writeUserFile(pSubDir, pName, pExtension, pContentData);

   if(pOnFinished)
   {
      pOnFinished(true);
   }
}

void DistributionPlatform::deleteRemoteFile(const char* pSubDir, const char* pName, const char* pExtension)
{
   Device::deleteUserFile(pSubDir, pName, pExtension);
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

void DistributionPlatform::createLobby()
{
}
