
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

void DistributionPlatform::updateLeaderboardScore(const char* pLeaderboardName, uint32_t pScore)
{
   SteamAPICall_t apiCall = SteamUserStats()->FindLeaderboard(pLeaderboardName);

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
