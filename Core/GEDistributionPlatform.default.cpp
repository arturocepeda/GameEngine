
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

const char* DistributionPlatform::getUserName() const
{
   static const char* defaultUserName = "Player";
   return defaultUserName;
}

void DistributionPlatform::unlockAchievement(const ObjectName&)
{
}

void DistributionPlatform::updateLeaderboardScore(const ObjectName&, uint32_t)
{
}

void DistributionPlatform::requestLeaderboardScores(const ObjectName&, uint16_t, uint16_t)
{
}

void DistributionPlatform::requestLeaderboardScoresAroundUser(const ObjectName&, uint16_t)
{
}
