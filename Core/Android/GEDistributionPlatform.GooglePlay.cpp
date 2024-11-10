
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
#include "Core/GEValue.h"

#include <sstream>

#include <jni.h>

static const char* kPurchasedProductsSubDir = "Save";
static const char* kPurchasedProductsFileName = "store";
static const char* kPurchasedProductsFileExtension = "ge";

typedef GESTLMap(uint32_t, GESTLString) IDsMap;
static IDsMap gLeaderboardIDsMap;

static GESTLSet(uint32_t) gPurchasedProducts;
static std::atomic<uint32_t> gPurchasingProductIDHash(0u);

static JavaVM* gJavaVM = nullptr;
static jclass gGPGClass = nullptr;
static jmethodID gMethodID_loggedIn = nullptr;
static jmethodID gMethodID_logIn = nullptr;
static jmethodID gMethodID_getUserName = nullptr;
static jmethodID gMethodID_updateLeaderboardScore = nullptr;
static jmethodID gMethodID_requestLeaderboardScores = nullptr;
static jmethodID gMethodID_requestLeaderboardScoresAroundUser = nullptr;
static jmethodID gMethodID_requestDLCPurchase = nullptr;
static jmethodID gMethodID_openWebPage = nullptr;
static jmethodID gMethodID_showFullscreenAd = nullptr;

static std::function<void()> gOnLogInFinished = nullptr;
static std::function<void(uint16_t, const char*, uint32_t)> gAddLeaderboardEntry = nullptr;
static std::function<void()> gOnLeaderboardQueryFinished = nullptr;

using namespace GE;
using namespace GE::Core;


//
//  Local static functions
//
static JNIEnv* getEnv()
{
   GEAssert(gJavaVM);
   JNIEnv* env = nullptr;
   return gJavaVM->AttachCurrentThread(&env, nullptr) == JNI_OK ? env : nullptr;
}

static void savePurchasedProducts()
{
   std::stringstream stream;

   Value valueReserved((uint32_t)0u);
   valueReserved.writeToStream(stream);

   Value valueCount((uint32_t)gPurchasedProducts.size());
   valueCount.writeToStream(stream);

   for(const uint32_t purchasedProductIDHash : gPurchasedProducts)
   {
      Value valuePurchasedProductIDHash(purchasedProductIDHash);
      valuePurchasedProductIDHash.writeToStream(stream);
   }

   std::stringstream::pos_type streamSize = stream.tellp();
   stream.seekp(0u, std::ios::beg);

   Content::ContentData storeData;
   storeData.load((uint32_t)streamSize, stream);

   Device::writeUserFile(kPurchasedProductsSubDir, kPurchasedProductsFileName, kPurchasedProductsFileExtension, &storeData);
}

static void loadPurchasedProducts()
{
   if(Device::userFileExists(kPurchasedProductsSubDir, kPurchasedProductsFileName, kPurchasedProductsFileExtension))
   {
      Content::ContentData storeData;
      Device::readUserFile(kPurchasedProductsSubDir, kPurchasedProductsFileName, kPurchasedProductsFileExtension, &storeData);

      Content::ContentDataMemoryBuffer memoryBuffer(storeData);
      std::istream stream(&memoryBuffer);

      Value valueReserved = Value::fromStream(ValueType::UInt, stream);
      Value valueCount = Value::fromStream(ValueType::UInt, stream);

      for(uint32_t i = 0u; i < valueCount.getAsUInt(); i++)
      {
         Value valuePurchasedProductIDHash = Value::fromStream(ValueType::UInt, stream);
         gPurchasedProducts.insert(valuePurchasedProductIDHash.getAsUInt());
      }
   }
}


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

      gMethodID_loggedIn =
         pEnv->GetStaticMethodID(gGPGClass, "loggedIn", "()Z");
      GEAssert(gMethodID_loggedIn);
      gMethodID_logIn =
         pEnv->GetStaticMethodID(gGPGClass, "logIn", "()V");
      GEAssert(gMethodID_logIn);
      gMethodID_getUserName =
         pEnv->GetStaticMethodID(gGPGClass, "getUserName", "()Ljava/lang/String;");
      GEAssert(gMethodID_getUserName);
      gMethodID_updateLeaderboardScore =
         pEnv->GetStaticMethodID(gGPGClass, "updateLeaderboardScore", "(Ljava/lang/String;J)V");
      GEAssert(gMethodID_updateLeaderboardScore);
      gMethodID_requestLeaderboardScores =
         pEnv->GetStaticMethodID(gGPGClass, "requestLeaderboardScores", "(Ljava/lang/String;II)V");
      GEAssert(gMethodID_requestLeaderboardScores);
      gMethodID_requestLeaderboardScoresAroundUser =
         pEnv->GetStaticMethodID(gGPGClass, "requestLeaderboardScoresAroundUser", "(Ljava/lang/String;I)V");
      GEAssert(gMethodID_requestLeaderboardScoresAroundUser);
      gMethodID_requestDLCPurchase =
         pEnv->GetStaticMethodID(gGPGClass, "requestDLCPurchase", "(Ljava/lang/String;)V");
      GEAssert(gMethodID_requestDLCPurchase);
      gMethodID_openWebPage =
         pEnv->GetStaticMethodID(gGPGClass, "openWebPage", "(Ljava/lang/String;)V");
      GEAssert(gMethodID_openWebPage);
      gMethodID_showFullscreenAd =
         pEnv->GetStaticMethodID(gGPGClass, "showFullscreenAd", "(Ljava/lang/String;)V");
      GEAssert(gMethodID_showFullscreenAd);
   }

   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_GPGOnLogInFinished(JNIEnv* pEnv, jclass pClass)
   {
      if(gOnLogInFinished)
      {
         gOnLogInFinished();
      }
   }

   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_GPGAddLeaderboardEntry(JNIEnv* pEnv, jclass pClass, jlong pRank, jstring pPlayerName, jlong pScore)
   {
      if(gAddLeaderboardEntry)
      {
         const uint16_t rank = (uint16_t)pRank;
         const char* playerName = pEnv->GetStringUTFChars(pPlayerName, nullptr);
         const uint32_t score = (uint32_t)pScore;

         gAddLeaderboardEntry(rank, playerName, score);
      }
   }

   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_GPGOnLeaderboardQueryFinished(JNIEnv* pEnv, jclass pClass)
   {
      if(gOnLeaderboardQueryFinished)
      {
         gOnLeaderboardQueryFinished();
      }
   }

   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_GPGNotifyPurchase(JNIEnv* pEnv, jclass pClass, jstring pProductName)
   {
      const ObjectName productName(pEnv->GetStringUTFChars(pProductName, nullptr));
      gPurchasedProducts.insert(productName.getID());
      savePurchasedProducts();
   }

   JNIEXPORT void JNICALL Java_com_GameEngine_Main_GameEngineLib_GPGOnPurchasesUpdateFinished(JNIEnv* pEnv, jclass pClass)
   {
      gPurchasingProductIDHash.store(0u);
   }
};


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

   loadPurchasedProducts();

   return true;
}

void DistributionPlatform::update()
{
}

void DistributionPlatform::shutdown()
{
   gOnLogInFinished = nullptr;
   gAddLeaderboardEntry = nullptr;
   gOnLeaderboardQueryFinished = nullptr;
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
   (void)pScoreDetail;

   IDsMap::const_iterator it = gLeaderboardIDsMap.find(pLeaderboardName.getID());
   GEAssert(it != gLeaderboardIDsMap.end());
   const char* leaderboardID = it->second.c_str();

   GEAssert(gGPGClass);
   GEAssert(gMethodID_updateLeaderboardScore);

   JNIEnv* env = getEnv();
   GEAssert(env);

   const jstring jargLeaderboardID = env->NewStringUTF(leaderboardID);
   const jlong jargScore = (jlong)pScore;

   env->CallStaticVoidMethod(gGPGClass, gMethodID_updateLeaderboardScore, jargLeaderboardID, jargScore);
}

void DistributionPlatform::requestLeaderboardScores(const ObjectName& pLeaderboardName, uint16_t pFirstPosition, uint16_t pLastPosition)
{
   mRequestingLeaderboardScores = true;

   IDsMap::const_iterator it = gLeaderboardIDsMap.find(pLeaderboardName.getID());
   GEAssert(it != gLeaderboardIDsMap.end());
   const char* leaderboardID = it->second.c_str();

   gAddLeaderboardEntry = [this, pLeaderboardName](uint16_t pRank, const char* pPlayerName, uint32_t pScore)
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

      LeaderboardEntry leaderboardEntry;
      leaderboardEntry.mUserName.assign(pPlayerName);
      leaderboardEntry.mPosition = pRank;
      leaderboardEntry.mScore = pScore;
      leaderboardEntry.mScoreDetail = 0u;
      addLeaderboardEntry(leaderboardIndex, leaderboardEntry);
   };

   gOnLeaderboardQueryFinished = [this]()
   {
      mRequestingLeaderboardScores = false;
   };

   GEAssert(gGPGClass);
   GEAssert(gMethodID_requestLeaderboardScores);

   JNIEnv* env = getEnv();
   GEAssert(env);

   const jstring jargLeaderboardID = env->NewStringUTF(leaderboardID);
   const jint jargFirstPosition = (jint)pFirstPosition;
   const jint jargLastPosition = (jint)pFirstPosition;

   env->CallStaticVoidMethod(gGPGClass, gMethodID_requestLeaderboardScores, jargLeaderboardID, jargFirstPosition, jargLastPosition);
}

void DistributionPlatform::requestLeaderboardScoresAroundUser(const ObjectName& pLeaderboardName, uint16_t pPositionsCount)
{
   mRequestingLeaderboardScores = true;

   IDsMap::const_iterator it = gLeaderboardIDsMap.find(pLeaderboardName.getID());
   GEAssert(it != gLeaderboardIDsMap.end());
   const char* leaderboardID = it->second.c_str();

   gAddLeaderboardEntry = [this, pLeaderboardName](uint16_t pRank, const char* pPlayerName, uint32_t pScore)
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

      LeaderboardEntry leaderboardEntry;
      leaderboardEntry.mUserName.assign(pPlayerName);
      leaderboardEntry.mPosition = pRank;
      leaderboardEntry.mScore = pScore;
      leaderboardEntry.mScoreDetail = 0u;
      addLeaderboardEntry(leaderboardIndex, leaderboardEntry);
   };

   gOnLeaderboardQueryFinished = [this]()
   {
      mRequestingLeaderboardScores = false;
   };

   GEAssert(gGPGClass);
   GEAssert(gMethodID_requestLeaderboardScoresAroundUser);

   JNIEnv* env = getEnv();
   GEAssert(env);

   const jstring jargLeaderboardID = env->NewStringUTF(leaderboardID);
   const jint jargPositionsCount = (jint)pPositionsCount;

   env->CallStaticVoidMethod(gGPGClass, gMethodID_requestLeaderboardScoresAroundUser, jargLeaderboardID, jargPositionsCount);
}

bool DistributionPlatform::isDLCAvailable(const ObjectName& pDLCName) const
{
   return gPurchasingProductIDHash.load() == 0u && gPurchasedProducts.find(pDLCName.getID()) != gPurchasedProducts.end();
}

void DistributionPlatform::requestDLCPurchase(const char* pURL) const
{
   const ObjectName productName(pURL);
   gPurchasingProductIDHash.store(productName.getID());

   GEAssert(gGPGClass);
   GEAssert(gMethodID_requestDLCPurchase);

   JNIEnv* env = getEnv();
   GEAssert(env);

   const jstring jargProductID = env->NewStringUTF(productName.getString());

   env->CallStaticVoidMethod(gGPGClass, gMethodID_requestDLCPurchase, jargProductID);
}

bool DistributionPlatform::processingDLCPurchaseRequest() const
{
   return gPurchasingProductIDHash.load() != 0u;
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
   GEAssert(gGPGClass);
   GEAssert(gMethodID_showFullscreenAd);

   JNIEnv* env = getEnv();
   GEAssert(env);

   const jstring jargID = env->NewStringUTF(pID);

   env->CallStaticVoidMethod(gGPGClass, gMethodID_showFullscreenAd, jargID);
}

void Device::openWebPage(const char* pURL)
{
   GEAssert(gGPGClass);
   GEAssert(gMethodID_openWebPage);

   JNIEnv* env = getEnv();
   GEAssert(env);

   const jstring jargURL = env->NewStringUTF(pURL);

   env->CallStaticVoidMethod(gGPGClass, gMethodID_openWebPage, jargURL);
}
