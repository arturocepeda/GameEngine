
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

void DistributionPlatform::updateLeaderboardScore(const char* pLeaderboardName, uint32_t pScore)
{
}
