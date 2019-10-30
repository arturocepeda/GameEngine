
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
#include "GEObject.h"

namespace GE { namespace Core
{
   class DistributionPlatform : public Singleton<DistributionPlatform>
   {
   public:
      struct LeaderboardEntry
      {
         uint16_t mPosition;
         uint32_t mScore;
         GESTLString mUserName;
         const char* getUserName() const { return mUserName.c_str(); }
      };
      struct Leaderboard
      {
         ObjectName mLeaderboardName;
         GESTLVector(LeaderboardEntry) mLeaderboardEntries;
      };

   private:
      GESTLVector(Leaderboard) mLeaderboards;

   public:
      bool init() const;
      void update() const;
      void shutdown() const;

      const char* getUserName() const;

      // Leaderboards
      void updateLeaderboardScore(const ObjectName& pLeaderboardName, uint32_t pScore);
      void resetLeaderboard(const ObjectName& pLeaderboardName);
      void requestLeaderboardScores(const ObjectName& pLeaderboardName, uint16_t pFirstPosition, uint16_t pLastPosition);
      void requestLeaderboardScoresAroundUser(const ObjectName& pLeaderboardName, uint16_t pPositionsCount);

      size_t getLeaderboardEntriesCount(const ObjectName& pLeaderboardName) const;
      const LeaderboardEntry* getLeaderboardEntry(const ObjectName& pLeaderboardName, size_t pEntryIndex) const;
   };
}}
