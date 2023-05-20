
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Distribution Platform (Game Center)
//
//  --- GEDistributionPlatform.GameCenter.mm ---
//
//////////////////////////////////////////////////////////////////

#include "Core/GEDistributionPlatform.h"
#include "Core/GEDevice.h"
#include "Core/GEValue.h"
#include "Input/GEInputSystem.h"
#include "Audio/GEAudioSystem.h"

#import <GameKit/GameKit.h>
#import <StoreKit/StoreKit.h>
#import <GoogleMobileAds/GoogleMobileAds.h>

#include <sstream>

using namespace GE;
using namespace GE::Core;
using namespace GE::Input;
using namespace GE::Audio;


static const char* kPurchasedProductsSubDir = "Save";
static const char* kPurchasedProductsFileName = "store";
static const char* kPurchasedProductsFileExtension = "ge";

static GESTLSet(uint32_t) gPurchasedProducts;
static std::atomic<uint32_t> gPurchasingProductIDHash(0u);

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
//  PaymentTransactionObserver
//
@interface PaymentTransactionObserver : NSObject<SKPaymentTransactionObserver>
@end

@implementation PaymentTransactionObserver
- (void)paymentQueue:(SKPaymentQueue*)pQueue updatedTransactions:(NSArray<SKPaymentTransaction*>*)pTransactions
{
   const size_t cachedPurchasedProductCount = gPurchasedProducts.size();
   
   for(SKPaymentTransaction* transaction in pTransactions)
   {
      NSString* productID = transaction.payment.productIdentifier;
      const uint32_t productIDHash = GE::Core::hash([productID UTF8String]);
      
      if(transaction.transactionState == SKPaymentTransactionStateRestored ||
         transaction.transactionState == SKPaymentTransactionStatePurchased)
      {
         gPurchasedProducts.insert(productIDHash);
      }
      
      if(gPurchasingProductIDHash.load() == productIDHash &&
         transaction.transactionState != SKPaymentTransactionStatePurchasing)
      {
         gPurchasingProductIDHash.store(0u);
      }
   }
   
   if(gPurchasedProducts.size() > cachedPurchasedProductCount)
   {
      savePurchasedProducts();
   }
}
@end

static PaymentTransactionObserver* gPaymentTransactionObserver = nil;


//
//  ProductsRequestDelegate
//
typedef GESTLMap(uint32_t, SKProduct*) ProductMap;
static ProductMap gProductMap;

@interface ProductsRequestDelegate : NSObject<SKProductsRequestDelegate>
@end

@implementation ProductsRequestDelegate
- (void)fetchProductInformation:(NSSet*)pIdentifiers
{
   SKProductsRequest* request = [[SKProductsRequest alloc] initWithProductIdentifiers:pIdentifiers];
   request.delegate = self;
   [request start];
}

- (void)productsRequest:(SKProductsRequest*)pRequest didReceiveResponse:(SKProductsResponse*)pResponse
{
   NSArray<SKProduct*>* products = pResponse.products;
   
   for(SKProduct* product : products)
   {
      const uint32_t productIDHash = GE::Core::hash([product.productIdentifier UTF8String]);
      gProductMap[productIDHash] = [product retain];
   }
}

- (void)request:(SKRequest *)request didFailWithError:(NSError*)pError
{
}
@end

static ProductsRequestDelegate* gProductsRequestDelegate = nil;


//
//  FullscreenAdDelegate
//
GADInterstitialAd* gFullscreenAd = nil;

@interface FullscreenAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@implementation FullscreenAdDelegate
- (void)adWillPresentFullScreenContent:(id<GADFullScreenPresentingAd>)pAd
{
   InputSystem::getInstance()->setInputEnabled(false);
   AudioSystem::getInstance()->pauseAll();
}

- (void)ad:(id<GADFullScreenPresentingAd>)pAd didFailToPresentFullScreenContentWithError:(NSError*)pError
{
   NSLog(@"Failed to present interstitial ad with error: %@", [pError localizedDescription]);

   InputSystem::getInstance()->setInputEnabled(true);
   AudioSystem::getInstance()->resumeAll();
   
   gFullscreenAd = nil;
}

- (void)adDidDismissFullScreenContent:(id<GADFullScreenPresentingAd>)pAd
{
   InputSystem::getInstance()->setInputEnabled(true);
   AudioSystem::getInstance()->resumeAll();
   
   gFullscreenAd = nil;
}
@end

static FullscreenAdDelegate* gFullscreenAdDelegate = nil;


//
//  DistributionPlatform
//
bool DistributionPlatform::init()
{
   NSMutableSet* productIdentifiers = [NSMutableSet set];
   
   Content::ContentData contentData;
   Device::readContentFile(Content::ContentType::GenericTextData, ".", "ios.metadata", "xml", &contentData);

   pugi::xml_document xml;
   xml.load_buffer(contentData.getData(), contentData.getDataSize());
   
   const pugi::xml_node& xmlInAppPurchases = xml.child("InAppPurchases");
   
   for(const pugi::xml_node& xmlProduct : xmlInAppPurchases.children("Product"))
   {
      NSString* productID = [NSString stringWithUTF8String:xmlProduct.attribute("id").as_string()];
      [productIdentifiers addObject:productID];
   }
   
   gProductsRequestDelegate = [[ProductsRequestDelegate alloc] init];
   [gProductsRequestDelegate fetchProductInformation:productIdentifiers];
   
   loadPurchasedProducts();
   
   if([SKPaymentQueue canMakePayments])
   {
      gPaymentTransactionObserver = [[PaymentTransactionObserver alloc] init];
      [[SKPaymentQueue defaultQueue] addTransactionObserver:gPaymentTransactionObserver];
      [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
   }
   
   gFullscreenAdDelegate = [[FullscreenAdDelegate alloc] init];
   [GADMobileAds.sharedInstance startWithCompletionHandler:nil];
      
   return true;
}

void DistributionPlatform::update()
{
}

void DistributionPlatform::shutdown()
{
   for(const auto& product : gProductMap)
   {
      GE::Core::Allocator::free(product.second);
   }
   
   gProductMap.clear();
}

const char* DistributionPlatform::getPlatformName() const
{
   static const char* platformName = "Game Center";
   return platformName;
}

const char* DistributionPlatform::getUserName() const
{
   if(loggedIn())
   {
      NSString* localPlayerAlias = [GKLocalPlayer localPlayer].alias;
      return [localPlayerAlias UTF8String];
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
   return [[GKLocalPlayer localPlayer] isAuthenticated];
}

void DistributionPlatform::logIn(std::function<void()> onFinished)
{
   [[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:^(NSError* _Nullable pError)
   {
      (void)pError;
      onFinished();
   }];
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
   mUpdatingLeaderboardScore = true;
   
   NSString* leaderboardIdentifier = [NSString stringWithUTF8String:pLeaderboardName.getString()];
   
   GKScore* gkScore = [[GKScore alloc] initWithLeaderboardIdentifier:leaderboardIdentifier];
   gkScore.value = (int64_t)pScore;
   [GKScore reportScores:@[gkScore] withCompletionHandler:^(NSError* pError)
   {
      (void)pError;
      mUpdatingLeaderboardScore = false;
   }];
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
   
   NSString* leaderboardIdentifier = [NSString stringWithUTF8String:pLeaderboardName.getString()];
   
   NSRange leaderboardRange;
   leaderboardRange.location = (NSUInteger)pFirstPosition;
   leaderboardRange.length = (NSUInteger)(pLastPosition - pFirstPosition + 1u);
   
   GKLeaderboard* gkLeaderboard = [[GKLeaderboard alloc] init];
   gkLeaderboard.identifier = leaderboardIdentifier;
   gkLeaderboard.playerScope = GKLeaderboardPlayerScope::GKLeaderboardPlayerScopeGlobal;
   gkLeaderboard.timeScope = GKLeaderboardTimeScope::GKLeaderboardTimeScopeAllTime;
   gkLeaderboard.range = leaderboardRange;
   
   [gkLeaderboard loadScoresWithCompletionHandler:^(NSArray* pScores, NSError* pError)
   {
      if(pScores != nil && pError == nil)
      {
         for(NSUInteger i = 0u; i < pScores.count; i++)
         {
            GKScore* gkScore = pScores[i];
            
            LeaderboardEntry leaderboardEntry;
            leaderboardEntry.mUserName.assign([gkScore.player.alias UTF8String]);
            leaderboardEntry.mPosition = (uint16_t)gkScore.rank;
            leaderboardEntry.mScore = (uint32_t)gkScore.value;
            leaderboardEntry.mScoreDetail = 0u;
            addLeaderboardEntry(leaderboardIndex, leaderboardEntry);
         }
      }
   }];
}

void DistributionPlatform::requestLeaderboardScoresAroundUser(const ObjectName& pLeaderboardName, uint16_t pPositionsCount)
{
   NSString* leaderboardIdentifier = [NSString stringWithUTF8String:pLeaderboardName.getString()];
   
   GKLeaderboard* gkLeaderboard = [[GKLeaderboard alloc] init];
   gkLeaderboard.identifier = leaderboardIdentifier;
   [gkLeaderboard loadScoresWithCompletionHandler:^(NSArray* pScores, NSError* pError)
   {
      if(pScores != nil && pError == nil)
      {
         GKScore* localPlayerScore = gkLeaderboard.localPlayerScore;
         
         const int firstPosition = std::max((int)localPlayerScore.rank - (int)pPositionsCount, 1);
         const int lastPosition = (int)localPlayerScore.rank + (int)pPositionsCount;
         
         requestLeaderboardScores(pLeaderboardName, (uint16_t)firstPosition, (uint16_t)lastPosition);
      }
   }];
}

bool DistributionPlatform::isDLCAvailable(const ObjectName& pDLCName) const
{
   return gPurchasedProducts.find(pDLCName.getID()) != gPurchasedProducts.end();
}

void DistributionPlatform::requestDLCPurchase(const char* pURL) const
{
   const uint32_t productIDHash = GE::Core::hash(pURL);
   ProductMap::const_iterator it = gProductMap.find(productIDHash);
   
   if(it != gProductMap.end())
   {
      SKPayment* payment = [SKPayment paymentWithProduct:it->second];
      [[SKPaymentQueue defaultQueue] addPayment:payment];
      
      if([SKPaymentQueue canMakePayments])
      {
         gPurchasingProductIDHash.store(productIDHash);
      }
   }
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
   NSString* gadID = [NSString stringWithUTF8String:pID];
   GADRequest* gadRequest = [GADRequest request];
   
   [GADInterstitialAd loadWithAdUnitID:gadID
      request:gadRequest
      completionHandler:^(GADInterstitialAd* pAd, NSError* pError)
   {
      if(pError)
      {
         NSLog(@"Failed to load interstitial ad with error: %@", [pError localizedDescription]);
         return;
      }
      
      gFullscreenAd = [pAd retain];
      pAd.fullScreenContentDelegate = gFullscreenAdDelegate;
      
      UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
      [pAd presentFromRootViewController:rootViewController];
   }];
}
