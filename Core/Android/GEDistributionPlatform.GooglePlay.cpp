
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Distribution Platform (Google Play)
//
//  --- GEDistributionPlatform.GooglePlay.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDistributionPlatform.h"
#include "Core/GEDevice.h"

#include <jni.h>

typedef GESTLMap(uint32_t, GESTLString) IDsMap;
static IDsMap gLeaderboardIDsMap;

static JavaVM* gJavaVM = nullptr;
static jclass gGPGClass = nullptr;
static jmethodID gMethodID_loggedIn = nullptr;
static jmethodID gMethodID_logIn = nullptr;
static jmethodID gMethodID_getUserName = nullptr;

static std::function<void()> gOnLogInFinished = nullptr;

using namespace GE;
using namespace GE::Core;


//
//  JNI functions
//
extern "C"
{
   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_GPGInitialize(JNIEnv* pEnv, jclass pClass, jobject pMainActivity)
   {
      (void)pClass;

      pEnv->GetJavaVM(&gJavaVM);
      GEAssert(gJavaVM);

      jclass gpgClass = pEnv->FindClass("com/GameEngine/Main/GameEngineGPG");
      GEAssert(gpgClass);
      gGPGClass = (jclass)pEnv->NewGlobalRef(gpgClass);
      GEAssert(gGPGClass);

      gMethodID_loggedIn = pEnv->GetStaticMethodID(gGPGClass, "loggedIn", "()Z");
      GEAssert(gMethodID_loggedIn);
      gMethodID_logIn = pEnv->GetStaticMethodID(gGPGClass, "logIn", "()V");
      GEAssert(gMethodID_logIn);
      gMethodID_getUserName = pEnv->GetStaticMethodID(gGPGClass, "getUserName", "()Ljava/lang/String;");
      GEAssert(gMethodID_getUserName);
   }

   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_GPGOnLogInFinished(JNIEnv* pEnv, jclass pClass)
   {
      if(gOnLogInFinished)
      {
         gOnLogInFinished();
      }
   }
};


//
//  Local static functions
//
static JNIEnv* getEnv()
{
   GEAssert(gJavaVM);
   JNIEnv* env = nullptr;
   return gJavaVM->AttachCurrentThread(&env, nullptr) == JNI_OK ? env : nullptr;
}


//
//  DistributionPlatform
//
bool DistributionPlatform::init()
{
   Content::ContentData contentData;
   Device::readContentFile(Content::ContentType::GenericTextData, ".", "android.metadata", "xml", &contentData);

   pugi::xml_document xml;
   xml.load_buffer(contentData.getData(), contentData.getDataSize());

   const pugi::xml_node& xmlLeaderboards = xml.child("Leaderboards");

   for(const pugi::xml_node& xmlLeaderboard : xmlLeaderboards.children("Leaderboard"))
   {
      const ObjectName leaderboardName(xmlLeaderboard.attribute("name").as_string());
      gLeaderboardIDsMap[leaderboardName.getID()] = GESTLString(xmlLeaderboard.attribute("id").as_string());
   }

   return true;
}

void DistributionPlatform::update()
{
}

void DistributionPlatform::shutdown()
{
   gOnLogInFinished = nullptr;
}

const char* DistributionPlatform::getPlatformName() const
{
   static const char* platformName = "Google Play";
   return platformName;
}

const char* DistributionPlatform::getUserName() const
{
   if(loggedIn())
   {
      GEAssert(gGPGClass);
      GEAssert(gMethodID_getUserName);

      JNIEnv* env = getEnv();
      GEAssert(env);

      jstring userNameJString = (jstring)env->CallStaticObjectMethod(gGPGClass, gMethodID_getUserName);
      const char* userNameCString = env->GetStringUTFChars(userNameJString, nullptr);

      static char userNameBuffer[256];
      strcpy(userNameBuffer, userNameCString);

      return userNameBuffer;
   }

   static const char* defaultUserName = "";
   return defaultUserName;
}

SystemLanguage DistributionPlatform::getLanguage() const
{
   return Device::requestOSLanguage();
}

bool DistributionPlatform::internetConnectionAvailable() const
{
   //TODO
   return true;
}

bool DistributionPlatform::loggedIn() const
{
   GEAssert(gGPGClass);
   GEAssert(gMethodID_loggedIn);

   JNIEnv* env = getEnv();
   GEAssert(env);

   return (bool)env->CallStaticBooleanMethod(gGPGClass, gMethodID_loggedIn);
}

void DistributionPlatform::logIn(std::function<void()> onFinished)
{
   gOnLogInFinished = onFinished;

   GEAssert(gGPGClass);
   GEAssert(gMethodID_loggedIn);

   JNIEnv* env = getEnv();
   GEAssert(env);

   env->CallStaticVoidMethod(gGPGClass, gMethodID_logIn);
}

void DistributionPlatform::logOut()
{
}

bool DistributionPlatform::remoteFileExists(const char* pSubDir, const char* pName, const char* pExtension)
{
   return Device::userFileExists(pSubDir, pName, pExtension);
}

bool DistributionPlatform::readRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   Content::ContentData* pContentData, std::function<void()> pOnFinished)
{
   Device::readUserFile(pSubDir, pName, pExtension, pContentData);

   if(pOnFinished)
   {
      pOnFinished();
   }

   return true;
}

bool DistributionPlatform::writeRemoteFile(const char* pSubDir, const char* pName, const char* pExtension,
   const Content::ContentData* pContentData, std::function<void(bool pSuccess)> pOnFinished)
{
   Device::writeUserFile(pSubDir, pName, pExtension, pContentData);

   if(pOnFinished)
   {
      pOnFinished(true);
   }

   return true;
}

bool DistributionPlatform::deleteRemoteFile(const char* pSubDir, const char* pName, const char* pExtension)
{
   Device::deleteUserFile(pSubDir, pName, pExtension);

   return true;
}

void DistributionPlatform::setStat(const ObjectName&, float)
{
}

float DistributionPlatform::getStat(const ObjectName&)
{
   return 0.0f;
}

void DistributionPlatform::unlockAchievement(const ObjectName&)
{
}

void DistributionPlatform::updateLeaderboardScore(const ObjectName& pLeaderboardName, uint32_t pScore, uint32_t pScoreDetail)
{
}

void DistributionPlatform::requestLeaderboardScores(const ObjectName& pLeaderboardName, uint16_t pFirstPosition, uint16_t pLastPosition)
{
}

void DistributionPlatform::requestLeaderboardScoresAroundUser(const ObjectName& pLeaderboardName, uint16_t pPositionsCount)
{
}

bool DistributionPlatform::isDLCAvailable(const ObjectName& pDLCName) const
{
   return false;
}

void DistributionPlatform::requestDLCPurchase(const char* pURL) const
{
}

bool DistributionPlatform::processingDLCPurchaseRequest() const
{
   return false;
}

void DistributionPlatform::findLobbies()
{
}

void DistributionPlatform::createLobby(const char*, uint32_t)
{
}

void DistributionPlatform::joinLobby(const Lobby*)
{
}

void DistributionPlatform::leaveLobby(const Lobby*)
{
}

bool DistributionPlatform::isJoinOrCreateLobbyFeatureAvailable() const
{
   return false;
}

void DistributionPlatform::joinOrCreateLobby(const char*, uint32_t)
{
}

size_t DistributionPlatform::getLobbyMembersCount(const Lobby*) const
{
   return 0u;
}

bool DistributionPlatform::getLobbyMember(const Lobby*, size_t, LobbyMember*)
{
   return false;
}

void DistributionPlatform::showFullscreenAd(const char* pID)
{
}
