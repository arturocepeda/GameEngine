
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Distribution Platform
//
//  --- GEDistributionPlatform.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDistributionPlatform.h"

using namespace GE;
using namespace GE::Core;

void DistributionPlatform::resetLeaderboard(const ObjectName& pLeaderboardName)
{
   for(size_t i = 0u; i < mLeaderboards.size(); i++)
   {
      if(mLeaderboards[i].mLeaderboardName == pLeaderboardName)
      {
         mLeaderboards[i].mLeaderboardEntries.clear();
         break;
      }
   }
}

size_t DistributionPlatform::getLeaderboardEntriesCount(const ObjectName& pLeaderboardName) const
{
   for(size_t i = 0u; i < mLeaderboards.size(); i++)
   {
      if(mLeaderboards[i].mLeaderboardName == pLeaderboardName)
      {
         return mLeaderboards[i].mLeaderboardEntries.size();
      }
   }

   return 0u;
}

const DistributionPlatform::LeaderboardEntry* DistributionPlatform::getLeaderboardEntry(const ObjectName& pLeaderboardName, size_t pEntryIndex) const
{
   for(size_t i = 0u; i < mLeaderboards.size(); i++)
   {
      if(mLeaderboards[i].mLeaderboardName == pLeaderboardName)
      {
         return mLeaderboards[i].mLeaderboardEntries.size() > pEntryIndex
            ? &mLeaderboards[i].mLeaderboardEntries[pEntryIndex]
            : nullptr;
      }
   }

   return nullptr;
}
