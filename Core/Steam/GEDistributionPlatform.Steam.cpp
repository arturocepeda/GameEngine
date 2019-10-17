
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Distribution Platform (Steam)
//
//  --- GEDistributionPlatform.Steam.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDistributionPlatform.h"
#include "Core/GELog.h"

#include "Externals/steamworks_sdk/public/steam/steam_api.h"

#pragma comment(lib, "../../../GameEngine/Externals/steamworks_sdk/redistributable_bin/win64/steam_api64.lib")

using namespace GE;
using namespace GE::Core;

bool DistributionPlatform::init() const
{
   const bool success = SteamAPI_Init();

   if(!success)
   {
      Log::log(LogType::Error, "The Steam API could no be initialized");
   }

   return success;
}

void DistributionPlatform::shutdown() const
{
   SteamAPI_Shutdown();
}

const char* DistributionPlatform::getUserName() const
{
   return SteamFriends()->GetPersonaName();
}
