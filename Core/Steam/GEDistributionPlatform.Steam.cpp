
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

static const size_t kPathBufferSize = 256u;

//
//  SteamCallback
//
template<typename T>
class SteamCallback : private CCallbackBase
{
public:
   typedef std::function<void(T*, bool)> func_t;

private:
   func_t mFunc;

   virtual void Run(void* pParam) override
   {
      mFunc((T*)pParam, false);
   }
   virtual void Run(void* pParam, bool pIOFailure, SteamAPICall_t) override
   {
      mFunc((T*)pParam, pIOFailure);
   }
   virtual int GetCallbackSizeBytes() override
   {
      return sizeof(T);
   }

public:
   SteamCallback()
      : mFunc(nullptr)
   {
      m_iCallback = T::k_iCallback;
   }
   ~SteamCallback()
   {
      Cancel();
   }
  
   void Set(func_t pFunc)
   {
      SteamAPI_UnregisterCallback(this);

      mFunc = pFunc;

      SteamAPI_RegisterCallback(this, m_iCallback);
   }
   void Cancel()
   {
      SteamAPI_UnregisterCallback(this);
      mFunc = nullptr;
   }
};

SteamCallback<UserStatsReceived_t> gCallbackRequestCurrentStats;


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

SteamCallResult<RemoteStorageFileReadAsyncComplete_t> gCallResultRemoteFileRead;
SteamCallResult<RemoteStorageFileWriteAsyncComplete_t> gCallResultRemoteFileWritten;
SteamCallResult<LeaderboardFindResult_t> gCallResultFindLeaderboard;
SteamCallResult<LeaderboardScoresDownloaded_t> gCallResultDownloadLeaderboardEntries;
SteamCallResult<LobbyCreated_t> gCallResultCreateLobby;
SteamCallResult<LobbyMatchList_t> gCallResultFindLobbies;
SteamCallResult<LobbyEnter_t> gCallResultJoinLobby;


//
//  DistributionPlatform
//
bool DistributionPlatform::init() const
{
   const bool success = SteamAPI_Init();

   if(success)
   {
      gCallbackRequestCurrentStats.Set([](UserStatsReceived_t* pResult, bool pIOFailure)
      {
         (void)pResult;
         (void)pIOFailure;
      });

      SteamUserStats()->RequestCurrentStats();
      SteamNetworkingUtils()->InitRelayNetworkAccess();
   }
   else
   {
      Log::log(LogType::Error, "The Steam API could not be initialized");
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

const char* DistributionPlatform::getPlatformName() const
{
   static const char* platformName = "Steam";
   return platformName;
}

const char* DistributionPlatform::getUserName() const
{
   return SteamFriends()->GetPersonaName();
}

SystemLanguage DistributionPlatform::getLanguage() const
{
   const char* gameLanguage = SteamApps()->GetCurrentGameLanguage();

   if(strcmp(gameLanguage, "english") == 0)
   {
      return SystemLanguage::English;
   }
   if(strcmp(gameLanguage, "spanish") == 0)
   {
      return SystemLanguage::Spanish;
   }
   if(strcmp(gameLanguage, "german") == 0)
   {
      return SystemLanguage::German;
   }

   return SystemLanguage::Count;
}

bool DistributionPlatform::remoteFileExists(const char* pSubDir, const char* pName, const char* pExtension)
{
   char fileName[kPathBufferSize];
   sprintf(fileName, "%s/%s.%s", pSubDir, pName, pExtension);

   return SteamRemoteStorage()->FileExists(fileName);
}

void DistributionPlatform::readRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   Content::ContentData* pContentData, std::function<void()> pOnFinished)
{
   char fileName[kPathBufferSize];
   sprintf(fileName, "%s/%s.%s", pSubDir, pName, pExtension);

   const int32_t fileSize = SteamRemoteStorage()->GetFileSize(fileName);
   SteamAPICall_t apiCall = SteamRemoteStorage()->FileReadAsync(fileName, 0u, fileSize);

   gCallResultRemoteFileRead.Set(apiCall, [fileSize, pContentData, pOnFinished](RemoteStorageFileReadAsyncComplete_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure)
      {
         char* buffer = Allocator::alloc<char>(fileSize);
         SteamRemoteStorage()->FileReadAsyncComplete(pResult->m_hFileReadAsync, buffer, fileSize);      
         pContentData->load(fileSize, buffer);
         Allocator::free(buffer);
      }

      if(pOnFinished)
      {
         pOnFinished();
      }
   });
}

void DistributionPlatform::writeRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   const Content::ContentData* pContentData, std::function<void(bool pSuccess)> pOnFinished)
{
   char fileName[kPathBufferSize];
   sprintf(fileName, "%s/%s.%s", pSubDir, pName, pExtension);

   const int32_t fileSize = SteamRemoteStorage()->GetFileSize(fileName);
   SteamAPICall_t apiCall = SteamRemoteStorage()->FileWriteAsync(fileName, pContentData->getData(), pContentData->getDataSize());

   gCallResultRemoteFileWritten.Set(apiCall, [pOnFinished](RemoteStorageFileWriteAsyncComplete_t* pResult, bool pIOFailure)
   {
      (void)pIOFailure;

      if(pOnFinished)
      {
         pOnFinished(pResult->m_eResult == k_EResultOK);
      }
   });
}

void DistributionPlatform::deleteRemoteFile(const char* pSubDir, const char* pName, const char* pExtension)
{
   char fileName[kPathBufferSize];
   sprintf(fileName, "%s/%s.%s", pSubDir, pName, pExtension);

   SteamRemoteStorage()->FileDelete(fileName);
}

void DistributionPlatform::unlockAchievement(const ObjectName& pAchievementName)
{
   bool achievementUnlocked = false;
   SteamUserStats()->GetAchievement(pAchievementName.getString(), &achievementUnlocked);

   if(!achievementUnlocked)
   {
      SteamUserStats()->SetAchievement(pAchievementName.getString());
      SteamUserStats()->StoreStats();
   }
}

void DistributionPlatform::updateLeaderboardScore(const ObjectName& pLeaderboardName, uint32_t pScore, uint32_t pScoreDetail)
{
   SteamAPICall_t apiCall = SteamUserStats()->FindLeaderboard(pLeaderboardName.getString());

   gCallResultFindLeaderboard.Set(apiCall, [pScore, pScoreDetail](LeaderboardFindResult_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure && pResult->m_bLeaderboardFound)
      {
         SteamUserStats()->UploadLeaderboardScore
         (
            pResult->m_hSteamLeaderboard,
            k_ELeaderboardUploadScoreMethodKeepBest,
            (int32_t)pScore, (const int32_t*)&pScoreDetail, 1
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
                  int32_t scoreDetail = 0;
                  SteamUserStats()->GetDownloadedLeaderboardEntry(pResult->m_hSteamLeaderboardEntries, i, &steamLeaderboardEntry, &scoreDetail, 1);
                  const char* userName = SteamFriends()->GetFriendPersonaName(steamLeaderboardEntry.m_steamIDUser);

                  LeaderboardEntry leaderboardEntry;
                  leaderboardEntry.mUserName.assign(userName);
                  leaderboardEntry.mPosition = (uint16_t)steamLeaderboardEntry.m_nGlobalRank;
                  leaderboardEntry.mScore = (uint32_t)steamLeaderboardEntry.m_nScore;
                  leaderboardEntry.mScoreDetail = (uint32_t)scoreDetail;
                  addLeaderboardEntry(leaderboardIndex, leaderboardEntry);
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
                  int32_t scoreDetail = 0;
                  SteamUserStats()->GetDownloadedLeaderboardEntry(pResult->m_hSteamLeaderboardEntries, i, &steamLeaderboardEntry, &scoreDetail, 1);
                  const char* userName = SteamFriends()->GetFriendPersonaName(steamLeaderboardEntry.m_steamIDUser);

                  LeaderboardEntry leaderboardEntry;
                  leaderboardEntry.mUserName.assign(userName);
                  leaderboardEntry.mPosition = (uint16_t)steamLeaderboardEntry.m_nGlobalRank;
                  leaderboardEntry.mScore = (uint32_t)steamLeaderboardEntry.m_nScore;
                  leaderboardEntry.mScoreDetail = (uint32_t)scoreDetail;
                  addLeaderboardEntry(leaderboardIndex, leaderboardEntry);
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

void DistributionPlatform::findLobbies()
{
   mLobbies.clear();
   mSearchingForLobbies = true;

   SteamAPICall_t apiCall = SteamMatchmaking()->RequestLobbyList();

   gCallResultFindLobbies.Set(apiCall, [this](LobbyMatchList_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure)
      {
         for(uint32_t i = 0u; i < pResult->m_nLobbiesMatching; i++)
         {
            CSteamID lobbyID = SteamMatchmaking()->GetLobbyByIndex((int)i);
            SteamMatchmaking()->RequestLobbyData(lobbyID);
            const char* lobbyName = SteamMatchmaking()->GetLobbyData(lobbyID, "name");

            Lobby lobby;
            lobby.mID = lobbyID.ConvertToUint64();
            lobby.mOwnerID = 0u;
            lobby.mName = lobbyName;
            lobby.mMember = false;
            mLobbies.push_back(lobby);
         }
      }

      mSearchingForLobbies = false;
   });
}

void DistributionPlatform::createLobby(uint32_t pMaxMembers)
{
   SteamAPICall_t apiCall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, (int)pMaxMembers);

   gCallResultCreateLobby.Set(apiCall, [this](LobbyCreated_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure && pResult->m_eResult == k_EResultOK)
      {
         const CSteamID lobbyID = pResult->m_ulSteamIDLobby;
         const char* lobbyName = getUserName();
         SteamMatchmaking()->SetLobbyData(lobbyID, "name", lobbyName);

         Lobby lobby;
         lobby.mID = lobbyID.ConvertToUint64();
         lobby.mOwnerID = SteamUser()->GetSteamID().ConvertToUint64();
         lobby.mName = lobbyName;
         lobby.mMember = true;
         mLobbies.push_back(lobby);
      }
   });
}

void DistributionPlatform::joinLobby(const Lobby* pLobby)
{
   CSteamID lobbyID;
   lobbyID.SetFromUint64(pLobby->mID);

   SteamAPICall_t apiCall = SteamMatchmaking()->JoinLobby(lobbyID);

   gCallResultJoinLobby.Set(apiCall, [this](LobbyEnter_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure)
      {
         for(size_t i = 0u; i < mLobbies.size(); i++)
         {
            if(mLobbies[i].mID == pResult->m_ulSteamIDLobby)
            {
               CSteamID lobbyID;
               lobbyID.SetFromUint64(pResult->m_ulSteamIDLobby);

               CSteamID lobbyOwnerID = SteamMatchmaking()->GetLobbyOwner(lobbyID);

               mLobbies[i].mOwnerID = lobbyOwnerID.ConvertToUint64();
               mLobbies[i].mMember = true;

               break;
            }
         }
      }
   });
}

void DistributionPlatform::leaveLobby(const Lobby* pLobby)
{
   if(pLobby->mMember)
   {
      CSteamID lobbyID;
      lobbyID.SetFromUint64(pLobby->mID);

      SteamMatchmaking()->LeaveLobby(lobbyID);
   }

   for(size_t i = 0u; i < mLobbies.size(); i++)
   {
      if(mLobbies[i].mID == pLobby->mID)
      {
         mLobbies.erase(mLobbies.begin() + i);
         break;
      }
   }
}

size_t DistributionPlatform::getLobbyMembersCount(const Lobby* pLobby) const
{
   CSteamID lobbyID;
   lobbyID.SetFromUint64(pLobby->mID);

   return (size_t)SteamMatchmaking()->GetNumLobbyMembers(lobbyID);
}

void DistributionPlatform::getLobbyMember(const Lobby* pLobby, size_t pIndex, LobbyMember* pOutMember)
{
   CSteamID lobbyID;
   lobbyID.SetFromUint64(pLobby->mID);

   const CSteamID memberID = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyID, (int)pIndex);
   pOutMember->mID = memberID.ConvertToUint64();
   pOutMember->mUserName.assign(SteamFriends()->GetFriendPersonaName(memberID));
}
