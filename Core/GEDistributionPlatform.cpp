
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

DistributionPlatform::DistributionPlatform()
   : mUpdatingLeaderboardScore(false)
   , mSearchingForLobbies(false)
   , mJoiningOrCreatingLobby(false)
   , mErrorCode(0u)
{
}

DistributionPlatform::~DistributionPlatform()
{
}

void DistributionPlatform::updateLeaderboardEntries(float pDeltaTime)
{
   for(size_t i = 0u; i < mLeaderboards.size(); i++)
   {
      mLeaderboards[i].mLeaderboardAge += pDeltaTime;
   }
}

void DistributionPlatform::addLeaderboardEntry(size_t pLeaderboardIndex, const LeaderboardEntry& pEntry)
{
   bool addNewEntry = true;

   for(size_t i = 0u; i < mLeaderboards[pLeaderboardIndex].mLeaderboardEntries.size(); i++)
   {
      if(mLeaderboards[pLeaderboardIndex].mLeaderboardEntries[i].mPosition == pEntry.mPosition)
      {
         addNewEntry = false;
         break;
      }
   }

   if(addNewEntry)
   {
      mLeaderboards[pLeaderboardIndex].mLeaderboardEntries.push_back(pEntry);
   }
}

bool DistributionPlatform::updatingLeaderboardScore() const
{
   return mUpdatingLeaderboardScore;
}

void DistributionPlatform::resetLeaderboard(const ObjectName& pLeaderboardName)
{
   for(size_t i = 0u; i < mLeaderboards.size(); i++)
   {
      if(mLeaderboards[i].mLeaderboardName == pLeaderboardName)
      {
         mLeaderboards[i].mLeaderboardAge = 0.0f;
         mLeaderboards[i].mLeaderboardEntries.clear();
         break;
      }
   }
}

float DistributionPlatform::getLeaderboardAge(const ObjectName& pLeaderboardName) const
{
   for(size_t i = 0u; i < mLeaderboards.size(); i++)
   {
      if(mLeaderboards[i].mLeaderboardName == pLeaderboardName)
      {
         return mLeaderboards[i].mLeaderboardAge;
      }
   }

   return FLT_MAX;
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

bool DistributionPlatform::searchingForLobbies() const
{
   return mSearchingForLobbies;
}

bool DistributionPlatform::joiningOrCreatingLobby() const
{
   return mJoiningOrCreatingLobby;
}

size_t DistributionPlatform::getLobbiesCount() const
{
   return mLobbies.size();
}

const DistributionPlatform::Lobby* DistributionPlatform::getLobby(size_t pIndex) const
{
   GEAssert(pIndex < mLobbies.size());
   return &mLobbies[pIndex];
}

void DistributionPlatform::resetErrorCode()
{
   mErrorCode = 0u;
}

uint32_t DistributionPlatform::getErrorCode() const
{
   return mErrorCode;
}
