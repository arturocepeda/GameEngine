
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

#include <functional>

#pragma comment(lib, "../../../GameEngine/Externals/steamworks_sdk/redistributable_bin/win64/steam_api64.lib")

using namespace GE;
using namespace GE::Core;

//
//  SteamCallResult
//
template<typename T>
class SteamCallResult : private CCallbackBase
{
public:
	typedef std::function<void(T*, bool)> func_t;

private:
	SteamAPICall_t mAPICall;
	func_t mFunc;

	virtual void Run(void* pParam) override
   {
	   mAPICall = k_uAPICallInvalid;
	   mFunc((T*)pParam, false);
   }
	virtual void Run(void* pParam, bool pIOFailure, SteamAPICall_t pAPICall) override
   {
	   if(mAPICall == pAPICall)
	   {
		   mAPICall = k_uAPICallInvalid;
		   mFunc((T*)pParam, pIOFailure);
	   }
   }
	virtual int GetCallbackSizeBytes() override
   {
      return sizeof(T);
   }

public:
	SteamCallResult()
      : mAPICall(k_uAPICallInvalid)
      , mFunc(nullptr)
   {
	   m_iCallback = T::k_iCallback;
   }
	~SteamCallResult()
   {
      Cancel();
   }
	
	void Set(SteamAPICall_t pAPICall, func_t pFunc)
   {
	   if(mAPICall)
      {
		   SteamAPI_UnregisterCallResult(this, mAPICall);
      }

	   mAPICall = pAPICall;
	   mFunc = pFunc;

	   if(mAPICall)
      {
		   SteamAPI_RegisterCallResult(this, mAPICall);
      }
   }
	bool IsActive() const
   {
      return mAPICall != k_uAPICallInvalid;
   }
	void Cancel()
   {
	   if(mAPICall != k_uAPICallInvalid)
	   {
		   SteamAPI_UnregisterCallResult(this, mAPICall);
		   mAPICall = k_uAPICallInvalid;
         mFunc = nullptr;
	   }
   }
	void SetGameserverFlag()
   {
      m_nCallbackFlags |= k_ECallbackFlagsGameServer;
   }
};

SteamCallResult<LeaderboardFindResult_t> gCallResultFindLeaderboard;
SteamCallResult<LeaderboardScoresDownloaded_t> gCallResultDownloadLeaderboardEntries;


//
//  DistributionPlatform
//
bool DistributionPlatform::init() const
{
   const bool success = SteamAPI_Init();

   if(!success)
   {
      Log::log(LogType::Error, "The Steam API could no be initialized");
   }

   return success;
}

void DistributionPlatform::update() const
{
   SteamAPI_RunCallbacks();
}

void DistributionPlatform::shutdown() const
{
   SteamAPI_Shutdown();
}

const char* DistributionPlatform::getUserName() const
{
   return SteamFriends()->GetPersonaName();
}

void DistributionPlatform::updateLeaderboardScore(const ObjectName& pLeaderboardName, uint32_t pScore)
{
   SteamAPICall_t apiCall = SteamUserStats()->FindLeaderboard(pLeaderboardName.getString());

   gCallResultFindLeaderboard.Set(apiCall, [pScore](LeaderboardFindResult_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure && pResult->m_bLeaderboardFound)
      {
         SteamUserStats()->UploadLeaderboardScore
         (
            pResult->m_hSteamLeaderboard,
            k_ELeaderboardUploadScoreMethodKeepBest,
            (int32_t)pScore, nullptr, 0
         );
      }
   });
}

void DistributionPlatform::requestLeaderboardScores(const ObjectName& pLeaderboardName, uint16_t pFirstPosition, uint16_t pLastPosition)
{
   size_t leaderboardIndex = 0u;
   bool leaderboardFound = false;

   for(size_t i = 0u; i < mLeaderboards.size(); i++)
   {
      if(mLeaderboards[i].mLeaderboardName == pLeaderboardName)
      {
         leaderboardIndex = i;
         leaderboardFound = true;
         break;
      }
   }

   if(!leaderboardFound)
   {
      mLeaderboards.emplace_back();
      leaderboardIndex = mLeaderboards.size() - 1u;
      mLeaderboards[leaderboardIndex].mLeaderboardName = pLeaderboardName;
   }

   SteamAPICall_t apiCall = SteamUserStats()->FindLeaderboard(pLeaderboardName.getString());

   gCallResultFindLeaderboard.Set(apiCall, [this, leaderboardIndex, pFirstPosition, pLastPosition](LeaderboardFindResult_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure && pResult->m_bLeaderboardFound)
      {
         SteamAPICall_t apiCall =
            SteamUserStats()->DownloadLeaderboardEntries(pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobal, (int)pFirstPosition, (int)pLastPosition);

         gCallResultDownloadLeaderboardEntries.Set(apiCall, [this, leaderboardIndex](LeaderboardScoresDownloaded_t* pResult, bool pIOFailure)
         {
            if(!pIOFailure && pResult->m_cEntryCount > 0)
            {
               for(int i = 0; i < pResult->m_cEntryCount; i++)
               {
                  LeaderboardEntry_t steamLeaderboardEntry;
                  SteamUserStats()->GetDownloadedLeaderboardEntry(pResult->m_hSteamLeaderboardEntries, i, &steamLeaderboardEntry, nullptr, 0);                  
                  const char* userName = SteamFriends()->GetFriendPersonaName(steamLeaderboardEntry.m_steamIDUser);

                  LeaderboardEntry leaderboardEntry;
                  leaderboardEntry.mUserName.assign(userName);
                  leaderboardEntry.mPosition = (uint16_t)steamLeaderboardEntry.m_nGlobalRank;
                  leaderboardEntry.mScore = (uint32_t)steamLeaderboardEntry.m_nScore;
                  mLeaderboards[leaderboardIndex].mLeaderboardEntries.push_back(leaderboardEntry);
               }

               std::sort(mLeaderboards[leaderboardIndex].mLeaderboardEntries.begin(), mLeaderboards[leaderboardIndex].mLeaderboardEntries.end(),
               [](const LeaderboardEntry& pEntry1, const LeaderboardEntry& pEntry2) -> bool
               {
                  return pEntry1.mPosition < pEntry2.mPosition;
               });
            }
         });
      }
   });
}

void DistributionPlatform::requestLeaderboardScoresAroundUser(const ObjectName& pLeaderboardName, uint16_t pPositionsCount)
{
   size_t leaderboardIndex = 0u;
   bool leaderboardFound = false;

   for(size_t i = 0u; i < mLeaderboards.size(); i++)
   {
      if(mLeaderboards[i].mLeaderboardName == pLeaderboardName)
      {
         leaderboardIndex = i;
         leaderboardFound = true;
         break;
      }
   }

   if(!leaderboardFound)
   {
      mLeaderboards.emplace_back();
      leaderboardIndex = mLeaderboards.size() - 1u;
      mLeaderboards[leaderboardIndex].mLeaderboardName = pLeaderboardName;
   }

   SteamAPICall_t apiCall = SteamUserStats()->FindLeaderboard(pLeaderboardName.getString());

   gCallResultFindLeaderboard.Set(apiCall, [this, leaderboardIndex, pPositionsCount](LeaderboardFindResult_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure && pResult->m_bLeaderboardFound)
      {
         const int first = -((int)pPositionsCount);
         const int last = (int)pPositionsCount;

         SteamAPICall_t apiCall =
            SteamUserStats()->DownloadLeaderboardEntries(pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, first, last);

         gCallResultDownloadLeaderboardEntries.Set(apiCall, [this, leaderboardIndex](LeaderboardScoresDownloaded_t* pResult, bool pIOFailure)
         {
            if(!pIOFailure && pResult->m_cEntryCount > 0)
            {
               for(int i = 0; i < pResult->m_cEntryCount; i++)
               {
                  LeaderboardEntry_t steamLeaderboardEntry;
                  SteamUserStats()->GetDownloadedLeaderboardEntry(pResult->m_hSteamLeaderboardEntries, i, &steamLeaderboardEntry, nullptr, 0);                  
                  const char* userName = SteamFriends()->GetFriendPersonaName(steamLeaderboardEntry.m_steamIDUser);

                  LeaderboardEntry leaderboardEntry;
                  leaderboardEntry.mUserName.assign(userName);
                  leaderboardEntry.mPosition = (uint16_t)steamLeaderboardEntry.m_nGlobalRank;
                  leaderboardEntry.mScore = (uint32_t)steamLeaderboardEntry.m_nScore;
                  mLeaderboards[leaderboardIndex].mLeaderboardEntries.push_back(leaderboardEntry);
               }

               std::sort(mLeaderboards[leaderboardIndex].mLeaderboardEntries.begin(), mLeaderboards[leaderboardIndex].mLeaderboardEntries.end(),
               [](const LeaderboardEntry& pEntry1, const LeaderboardEntry& pEntry2) -> bool
               {
                  return pEntry1.mPosition < pEntry2.mPosition;
               });
            }
         });
      }
   });
}
