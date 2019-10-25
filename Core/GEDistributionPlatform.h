
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Distribution Platform
//
//  --- GEDistributionPlatform.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GESingleton.h"

namespace GE { namespace Core
{
   class DistributionPlatform : public Singleton<DistributionPlatform>
   {
   public:
      bool init() const;
      void update() const;
      void shutdown() const;

      const char* getUserName() const;

      // Leaderboards
      void updateLeaderboardScore(const char* pLeaderboardName, uint32_t pScore);
   };
}}
