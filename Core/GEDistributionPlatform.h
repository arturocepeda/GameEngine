
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
#include "GEDevice.h"

#include <atomic>

namespace GE { namespace Core
{
   class DistributionPlatform : public Singleton<DistributionPlatform>
   {
   public:
      struct LeaderboardEntry
      {
         uint16_t mPosition;
         uint32_t mScore;
         uint32_t mScoreDetail;
         GESTLString mUserName;
         const char* getUserName() const { return mUserName.c_str(); }
      };
      struct Leaderboard
      {
         ObjectName mLeaderboardName;
         float mLeaderboardAge;
         GESTLVector(LeaderboardEntry) mLeaderboardEntries;

         Leaderboard() : mLeaderboardAge(0.0f) {}
      };

      struct Lobby
      {
         uint64_t mID;
         uint64_t mOwnerID;
         GESTLString mOwnerIP;
         GESTLString mName;
         uint16_t mOwnerPort;
         bool mMember;
      };
      struct LobbyMember
      {
         uint64_t mID;
         GESTLString mUserName;
      };

   private:
      GESTLVector(Leaderboard) mLeaderboards;
      GESTLVector(Lobby) mLobbies;

      std::atomic<bool> mUpdatingLeaderboardScore;
      std::atomic<bool> mRequestingLeaderboardScores;
      std::atomic<bool> mSearchingForLobbies;
      std::atomic<bool> mJoiningOrCreatingLobby;

      std::atomic<uint32_t> mErrorCode;

      void updateLeaderboardEntries(float pDeltaTime);
      void addLeaderboardEntry(size_t pLeaderboardIndex, const LeaderboardEntry& pEntry);

   public:
      DistributionPlatform();
      ~DistributionPlatform();

      bool init();
      void update();
      void shutdown();

      const char* getPlatformName() const;
      const char* getUserName() const;

      SystemLanguage getLanguage() const;

      // Network
      bool internetConnectionAvailable() const;
      bool loggedIn() const;

      void logIn(std::function<void()> onFinished);
      void logOut();

      // Data storage
      bool remoteFileExists(const char* pSubDir, const char* pName, const char* pExtension);
      bool readRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
        Content::ContentData* pContentData, std::function<void()> pOnFinished = nullptr);
      bool writeRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
        const Content::ContentData* pContentData, std::function<void(bool pSuccess)> pOnFinished = nullptr);
      bool deleteRemoteFile(const char* pSubDir, const char* pName, const char* pExtension);

      // Stats
      void setStat(const ObjectName& pStatName, float pValue);
      float getStat(const ObjectName& pStatName);

      // Achievements
      void unlockAchievement(const ObjectName& pAchievementName);

      // Leaderboards
      void updateLeaderboardScore(const ObjectName& pLeaderboardName, uint32_t pScore, uint32_t pScoreDetail);
      bool updatingLeaderboardScore() const;

      void resetLeaderboard(const ObjectName& pLeaderboardName);
      void requestLeaderboardScores(const ObjectName& pLeaderboardName, uint16_t pFirstPosition, uint16_t pLastPosition);
      void requestLeaderboardScoresAroundUser(const ObjectName& pLeaderboardName, uint16_t pPositionsCount);
      bool requestingLeaderboardScores() const;

      float getLeaderboardAge(const ObjectName& pLeaderboardName) const;
      size_t getLeaderboardEntriesCount(const ObjectName& pLeaderboardName) const;
      const LeaderboardEntry* getLeaderboardEntry(const ObjectName& pLeaderboardName, size_t pEntryIndex) const;

      // DLCs
      bool isDLCAvailable(const ObjectName& pDLCName) const;
      void requestDLCPurchase(const char* pURL) const;
      bool processingDLCPurchaseRequest() const;

      // Matchmaking
      void findLobbies();
      bool searchingForLobbies() const;

      void createLobby(const char* pName, uint32_t pMaxMembers);
      void joinLobby(const Lobby* pLobby);
      void leaveLobby(const Lobby* pLobby);

      bool isJoinOrCreateLobbyFeatureAvailable() const;
      void joinOrCreateLobby(const char* pName, uint32_t pMaxMembers);
      bool joiningOrCreatingLobby() const;

      size_t getLobbyMembersCount(const Lobby* pLobby) const;
      bool getLobbyMember(const Lobby* pLobby, size_t pIndex, LobbyMember* pOutMember);

      size_t getLobbiesCount() const;
      const Lobby* getLobby(size_t pIndex) const;

      // Error management
      void resetErrorCode();
      uint32_t getErrorCode() const;
   };
}}
