
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
         GESTLVector(LeaderboardEntry) mLeaderboardEntries;
      };

      struct Lobby
      {
         uint64_t mID;
         uint64_t mOwnerID;
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

      void addLeaderboardEntry(size_t pLeaderboardIndex, const LeaderboardEntry& pEntry);

   public:
      bool init() const;
      void update() const;
      void shutdown() const;

      const char* getPlatformName() const;
      const char* getUserName() const;

      SystemLanguage getLanguage() const;

      // Data storage
      bool remoteFileExists(const char* pSubDir, const char* pName, const char* pExtension);
      void readRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
        Content::ContentData* pContentData, std::function<void()> pOnFinished = nullptr);
      void writeRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
        const Content::ContentData* pContentData, std::function<void(bool pSuccess)> pOnFinished = nullptr);
      void deleteRemoteFile(const char* pSubDir, const char* pName, const char* pExtension);

      // Achievements
      void unlockAchievement(const ObjectName& pAchievementName);

      // Leaderboards
      void updateLeaderboardScore(const ObjectName& pLeaderboardName, uint32_t pScore, uint32_t pScoreDetail);
      void resetLeaderboard(const ObjectName& pLeaderboardName);
      void requestLeaderboardScores(const ObjectName& pLeaderboardName, uint16_t pFirstPosition, uint16_t pLastPosition);
      void requestLeaderboardScoresAroundUser(const ObjectName& pLeaderboardName, uint16_t pPositionsCount);

      size_t getLeaderboardEntriesCount(const ObjectName& pLeaderboardName) const;
      const LeaderboardEntry* getLeaderboardEntry(const ObjectName& pLeaderboardName, size_t pEntryIndex) const;

      // Matchmaking
      void findLobbies();
      void createLobby();
      void joinLobby(const Lobby* pLobby);
      void leaveLobby(const Lobby* pLobby);

      size_t getLobbyMembersCount(const Lobby* pLobby) const;
      void getLobbyMember(const Lobby* pLobby, size_t pIndex, LobbyMember* pOutMember);

      size_t getLobbiesCount() const;
      const Lobby* getLobby(size_t pIndex) const;
   };
}}
