
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
#include <sstream>

#pragma comment(lib, "../../../GameEngine/Externals/steamworks_sdk/redistributable_bin/win64/steam_api64.lib")

using namespace GE;
using namespace GE::Core;

static const size_t kPathBufferSize = 256u;

extern "C"
void __cdecl steamAPIDebugTextHook(int pSeverity, const char* pDebugText)
{
   const LogType logType = pSeverity == 1 ? LogType::Warning : LogType::Info;
   Log::log(logType, pDebugText);
}


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

static const int kStorageCallResults = 8;
SteamCallResult<RemoteStorageFileReadAsyncComplete_t> gCallResultRemoteFileRead[kStorageCallResults];
SteamCallResult<RemoteStorageFileWriteAsyncComplete_t> gCallResultRemoteFileWritten[kStorageCallResults];
int gCallResultRemoteFileReadIndex = 0;
int gCallResultRemoteFileWrittenIndex = 0;

SteamCallResult<LeaderboardFindResult_t> gCallResultFindLeaderboard;
SteamCallResult<LeaderboardScoreUploaded_t> gCallResultUpdateLeaderboardScore;
SteamCallResult<LeaderboardScoresDownloaded_t> gCallResultDownloadLeaderboardEntries;

SteamCallResult<LobbyCreated_t> gCallResultCreateLobby;
SteamCallResult<LobbyMatchList_t> gCallResultFindLobbies;
SteamCallResult<LobbyEnter_t> gCallResultJoinLobby;


//
//  DistributionPlatform
//
static bool gStoreStatsPending = false;

bool DistributionPlatform::init()
{
   const bool success = SteamAPI_Init();

   if(success)
   {
      SteamUtils()->SetWarningMessageHook(&steamAPIDebugTextHook);

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

void DistributionPlatform::update()
{
   if(gStoreStatsPending)
   {
      SteamUserStats()->StoreStats();
      gStoreStatsPending = false;
   }

   SteamAPI_RunCallbacks();
}

void DistributionPlatform::shutdown()
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
   if(strcmp(gameLanguage, "french") == 0)
   {
      return SystemLanguage::French;
   }
   if(strcmp(gameLanguage, "italian") == 0)
   {
      return SystemLanguage::Italian;
   }
   if(strcmp(gameLanguage, "portuguese") == 0)
   {
      return SystemLanguage::Portuguese;
   }

   if(strcmp(gameLanguage, "russian") == 0)
   {
      return SystemLanguage::Russian;
   }
   if(strcmp(gameLanguage, "schinese") == 0)
   {
      return SystemLanguage::ChineseSimplified;
   }
   if(strcmp(gameLanguage, "tchinese") == 0)
   {
      return SystemLanguage::ChineseTraditional;
   }
   if(strcmp(gameLanguage, "japanese") == 0)
   {
      return SystemLanguage::Japanese;
   }
   if(strcmp(gameLanguage, "koreana") == 0)
   {
      return SystemLanguage::Korean;
   }

   return SystemLanguage::Count;
}

bool DistributionPlatform::internetConnectionAvailable() const
{
   return true;
}

bool DistributionPlatform::loggedIn() const
{
   return true;
}

void DistributionPlatform::logIn(std::function<void()> onFinished)
{
   (void)onFinished;
}

void DistributionPlatform::logOut()
{
}

bool DistributionPlatform::remoteFileExists(const char* pSubDir, const char* pName, const char* pExtension)
{
   char fileName[kPathBufferSize];
   sprintf(fileName, "%s/%s.%s", pSubDir, pName, pExtension);

   return SteamRemoteStorage()->FileExists(fileName);
}

bool DistributionPlatform::readRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   Content::ContentData* pContentData, std::function<void()> pOnFinished)
{
   char fileName[kPathBufferSize];
   sprintf(fileName, "%s/%s.%s", pSubDir, pName, pExtension);

   bool success = false;
   const uint32 fileSize = (uint32)SteamRemoteStorage()->GetFileSize(fileName);
   SteamAPICall_t apiCall = SteamRemoteStorage()->FileReadAsync(fileName, 0u, fileSize);

   if(apiCall != k_uAPICallInvalid)
   {
      success = true;

      gCallResultRemoteFileRead[gCallResultRemoteFileReadIndex++].Set(
         apiCall, [fileSize, pContentData, pOnFinished](RemoteStorageFileReadAsyncComplete_t* pResult, bool pIOFailure)
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

      if(gCallResultRemoteFileReadIndex == kStorageCallResults)
      {
         gCallResultRemoteFileReadIndex = 0;
      }
   }
   else
   {
      Log::log(LogType::Warning, "[Steam] The '%s' file could not be read asynchronously", fileName);

      char* buffer = Allocator::alloc<char>(fileSize);
      success = SteamRemoteStorage()->FileRead(fileName, buffer, (int32)fileSize) == (int32)fileSize;
      pContentData->load(fileSize, buffer);
      Allocator::free(buffer);

      if(pOnFinished)
      {
         pOnFinished();
      }
   }

   return success;
}

bool DistributionPlatform::writeRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   const Content::ContentData* pContentData, std::function<void(bool pSuccess)> pOnFinished)
{
   char fileName[kPathBufferSize];
   sprintf(fileName, "%s/%s.%s", pSubDir, pName, pExtension);

   bool success = false;
   SteamAPICall_t apiCall = SteamRemoteStorage()->FileWriteAsync(fileName, pContentData->getData(), pContentData->getDataSize());

   if(apiCall != k_uAPICallInvalid)
   {
      success = true;

      gCallResultRemoteFileWritten[gCallResultRemoteFileWrittenIndex++].Set(
         apiCall, [pOnFinished](RemoteStorageFileWriteAsyncComplete_t* pResult, bool pIOFailure)
      {
         (void)pIOFailure;

         if(pOnFinished)
         {
            pOnFinished(pResult->m_eResult == k_EResultOK);
         }
      });

      if(gCallResultRemoteFileWrittenIndex == kStorageCallResults)
      {
         gCallResultRemoteFileWrittenIndex = 0;
      }
   }
   else
   {
      Log::log(LogType::Warning, "[Steam] The '%s' file could not be written asynchronously", fileName);

      success = SteamRemoteStorage()->FileWrite(fileName, pContentData->getData(), (int32)pContentData->getDataSize());
   }

   return success;
}

bool DistributionPlatform::deleteRemoteFile(const char* pSubDir, const char* pName, const char* pExtension)
{
   char fileName[kPathBufferSize];
   sprintf(fileName, "%s/%s.%s", pSubDir, pName, pExtension);

   return SteamRemoteStorage()->FileDelete(fileName);
}

void DistributionPlatform::setStat(const ObjectName& pStatName, float pValue)
{
   SteamUserStats()->SetStat(pStatName.getString(), pValue);
   gStoreStatsPending = true;
}

float DistributionPlatform::getStat(const ObjectName& pStatName)
{
   float value = 0.0f;
   SteamUserStats()->GetStat(pStatName.getString(), &value);
   return value;
}

void DistributionPlatform::unlockAchievement(const ObjectName& pAchievementName)
{
   bool achievementUnlocked = false;
   SteamUserStats()->GetAchievement(pAchievementName.getString(), &achievementUnlocked);

   if(!achievementUnlocked)
   {
      SteamUserStats()->SetAchievement(pAchievementName.getString());
      gStoreStatsPending = true;
   }
}

void DistributionPlatform::updateLeaderboardScore(const ObjectName& pLeaderboardName, uint32_t pScore, uint32_t pScoreDetail)
{
   SteamAPICall_t apiCall = SteamUserStats()->FindLeaderboard(pLeaderboardName.getString());
   mUpdatingLeaderboardScore = true;

   gCallResultFindLeaderboard.Set(apiCall, [this, pScore, pScoreDetail](LeaderboardFindResult_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure && pResult->m_bLeaderboardFound)
      {
         SteamAPICall_t apiCall = SteamUserStats()->UploadLeaderboardScore
         (
            pResult->m_hSteamLeaderboard,
            k_ELeaderboardUploadScoreMethodKeepBest,
            (int32_t)pScore, (const int32_t*)&pScoreDetail, 1
         );

         gCallResultUpdateLeaderboardScore.Set(apiCall, [this](LeaderboardScoreUploaded_t*, bool)
         {
            mUpdatingLeaderboardScore = false;
         });
      }
      else
      {
         mUpdatingLeaderboardScore = false;
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

bool DistributionPlatform::isDLCAvailable(const ObjectName& pDLCName) const
{
   const int32 dlcCount = SteamApps()->GetDLCCount();

   for(int i = 0; i < dlcCount; i++)
   {
      static const int kNameBufferSize = 128;

      AppId_t appId;
      bool available;
      char name[kNameBufferSize];

      if(SteamApps()->BGetDLCDataByIndex(i, &appId, &available, name, kNameBufferSize))
      {
         if(strcmp(pDLCName.getString(), name) == 0)
         {
            return available && SteamApps()->BIsDlcInstalled(appId);
         }
      }
   }

   return false;
}

void DistributionPlatform::requestDLCPurchase(const char* pURL) const
{
   SteamFriends()->ActivateGameOverlayToWebPage(pURL);
}

bool DistributionPlatform::processingDLCPurchaseRequest() const
{
   return false;
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
            lobby.mOwnerPort = 0u;
            lobby.mName = lobbyName;
            lobby.mMember = false;
            mLobbies.push_back(lobby);
         }
      }

      mSearchingForLobbies = false;
   });
}

void DistributionPlatform::createLobby(const char* pName, uint32_t pMaxMembers)
{
   static GESTLString lobbyName;

   SteamAPICall_t apiCall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, (int)pMaxMembers);
   lobbyName = pName;

   gCallResultCreateLobby.Set(apiCall, [this](LobbyCreated_t* pResult, bool pIOFailure)
   {
      if(!pIOFailure && pResult->m_eResult == k_EResultOK)
      {
         const CSteamID lobbyID = pResult->m_ulSteamIDLobby;
         SteamMatchmaking()->SetLobbyData(lobbyID, "name", lobbyName.c_str());

         Lobby lobby;
         lobby.mID = lobbyID.ConvertToUint64();
         lobby.mOwnerID = SteamUser()->GetSteamID().ConvertToUint64();
         lobby.mOwnerPort = 0u;
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
               mLobbies[i].mOwnerPort = 0u;
               mLobbies[i].mMember = true;

               std::ostringstream oss;
               oss << mLobbies[i].mOwnerID;
               mLobbies[i].mOwnerIP.assign(oss.str().c_str());

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

bool DistributionPlatform::isJoinOrCreateLobbyFeatureAvailable() const
{
   return false;
}

void DistributionPlatform::joinOrCreateLobby(const char* pName, uint32_t pMaxMembers)
{
   (void)pName;
   (void)pMaxMembers;
}

size_t DistributionPlatform::getLobbyMembersCount(const Lobby* pLobby) const
{
   CSteamID lobbyID;
   lobbyID.SetFromUint64(pLobby->mID);

   return (size_t)SteamMatchmaking()->GetNumLobbyMembers(lobbyID);
}

bool DistributionPlatform::getLobbyMember(const Lobby* pLobby, size_t pIndex, LobbyMember* pOutMember)
{
   CSteamID lobbyID;
   lobbyID.SetFromUint64(pLobby->mID);

   const CSteamID memberID = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyID, (int)pIndex);
   pOutMember->mID = memberID.ConvertToUint64();
   pOutMember->mUserName.assign(SteamFriends()->GetFriendPersonaName(memberID));

   return true;
}

void DistributionPlatform::showFullscreenAd(const char* pID)
{
}
