
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda
//  Game Engine
//
//  Android
//
//  --- GameEngineGPG.java ---
//
//////////////////////////////////////////////////////////////////

package com.GameEngine.Main;

import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.android.gms.games.PlayGamesSdk;
import com.google.android.gms.games.PlayGames;
import com.google.android.gms.games.GamesSignInClient;
import com.google.android.gms.games.LeaderboardsClient;
import com.google.android.gms.games.leaderboard.LeaderboardScoreBuffer;
import com.google.android.gms.games.leaderboard.LeaderboardVariant;

import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;
import com.google.android.gms.ads.MobileAds;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.QueryPurchasesParams;
import com.android.billingclient.api.QueryProductDetailsParams;

public class GameEngineGPG
{
   private static Activity smActivity;
   private static boolean smAuthenticated = false;
   private static String smPlayerName;

   private static BillingClient smBillingClient;
   private static GameEngineBillingClientStateListener smBillingClientStateListener;
   private static GameEngineProductDetailsResponseListener smProductDetailsResponseListener;
   private static GameEnginePurchasesResponseListener smPurchasesResponseListener;
   private static GameEnginePurchasesUpdatedListener smPurchasesUpdatedListener;

   private static InterstitialAd smFullscreenAd;

   private static class GameEngineBillingClientStateListener implements BillingClientStateListener
   {
      @Override
      public void onBillingSetupFinished(BillingResult pBillingResult)
      {
         if(pBillingResult.getResponseCode() ==  BillingClient.BillingResponseCode.OK)
         {
            smBillingClient.queryPurchasesAsync
            (
               QueryPurchasesParams.newBuilder()
                  .setProductType(BillingClient.ProductType.INAPP)
                  .build(),
               smPurchasesResponseListener
            );
         }
         else
         {
            smAuthenticated = false;
         }
      }

      @Override
      public void onBillingServiceDisconnected()
      {
         smAuthenticated = false;
      }
   }

   private static class GameEngineProductDetailsResponseListener implements ProductDetailsResponseListener
   {
      @Override
      public void onProductDetailsResponse(BillingResult pBillingResult, List<ProductDetails> pProductDetailsList)
      {
         if(pProductDetailsList.isEmpty())
         {
            GameEngineLib.GPGOnPurchasesUpdateFinished();
         }
         else
         {
            List<BillingFlowParams.ProductDetailsParams> productDetailsParamsList =
               List.of
               (
                  BillingFlowParams.ProductDetailsParams.newBuilder()
                     .setProductDetails(pProductDetailsList.get(0))
                     .build()
               );
            BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                    .setProductDetailsParamsList(productDetailsParamsList)
                    .build();
            BillingResult billingResult = smBillingClient.launchBillingFlow(smActivity, billingFlowParams);

            if(billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK)
            {
               GameEngineLib.GPGOnPurchasesUpdateFinished();
            }
         }
      }
   }

   private static class GameEnginePurchasesResponseListener implements PurchasesResponseListener
   {
      @Override
      public void onQueryPurchasesResponse(BillingResult pBillingResult, List<Purchase> pPurchases)
      {
         if(pBillingResult != null && pBillingResult.getResponseCode() == BillingClient.BillingResponseCode.OK)
         {
            if(pPurchases != null)
            {
               for(Purchase purchase : pPurchases)
               {
                  List<String> products = purchase.getProducts();

                  for(String product : products)
                  {
                     GameEngineLib.GPGNotifyPurchase(product);
                  }
               }
            }
         }

         GameEngineLib.GPGOnPurchasesUpdateFinished();
      }
   }

   private static class GameEnginePurchasesUpdatedListener implements PurchasesUpdatedListener
   {
      @Override
      public void onPurchasesUpdated(@NonNull BillingResult pBillingResult, @Nullable List<Purchase> pPurchases)
      {
         if(pBillingResult.getResponseCode() == BillingClient.BillingResponseCode.OK)
         {
            if(pPurchases != null)
            {
               for(Purchase purchase : pPurchases)
               {
                  List<String> products = purchase.getProducts();

                  for(String product : products)
                  {
                     GameEngineLib.GPGNotifyPurchase(product);
                  }
               }
            }
         }

         GameEngineLib.GPGOnPurchasesUpdateFinished();
      }
   }

   public static void initialize(Activity pActivity)
   {
      smActivity = pActivity;

      PlayGamesSdk.initialize(pActivity);
      updateAuthenticationState();

      smBillingClientStateListener = new GameEngineBillingClientStateListener();
      smProductDetailsResponseListener = new GameEngineProductDetailsResponseListener();
      smPurchasesResponseListener = new GameEnginePurchasesResponseListener();
      smPurchasesUpdatedListener = new GameEnginePurchasesUpdatedListener();
      smBillingClient = BillingClient.newBuilder(pActivity)
              .enablePendingPurchases()
              .setListener(smPurchasesUpdatedListener)
              .build();
      smBillingClient.startConnection(smBillingClientStateListener);

      MobileAds.initialize(pActivity);

      GameEngineLib.GPGInitialize(pActivity);
   }

   public static String getUserName()
   {
      return smPlayerName;
   }

   public static boolean loggedIn()
   {
      return smAuthenticated;
   }

   public static void logIn()
   {
      GamesSignInClient gamesSignInClient = PlayGames.getGamesSignInClient(smActivity);
      gamesSignInClient.signIn().addOnCompleteListener(pTask ->
      {
         updateAuthenticationState();
         smBillingClient.startConnection(smBillingClientStateListener);
      });
   }

   public static void updateLeaderboardScore(String pLeaderboardID, long pScore)
   {
      PlayGames.getLeaderboardsClient(smActivity).submitScore(pLeaderboardID, pScore);
   }

   public static void requestLeaderboardScores(String pLeaderboardID, int pFirstPosition, int pLastPosition)
   {
      PlayGames.getLeaderboardsClient(smActivity).loadTopScores
      (
         pLeaderboardID,
         LeaderboardVariant.TIME_SPAN_ALL_TIME,
         LeaderboardVariant.COLLECTION_PUBLIC,
         pLastPosition - pFirstPosition + 1
      )
      .addOnCompleteListener(pTask ->
      {
         if(pTask.isSuccessful())
         {
            LeaderboardsClient.LeaderboardScores scores = pTask.getResult().get();

            if(scores != null)
            {
               LeaderboardScoreBuffer scoreBuffer = scores.getScores();

               for(int i = 0; i < scoreBuffer.getCount(); i++)
               {
                  long rank = scoreBuffer.get(i).getRank();
                  String playerName = scoreBuffer.get(i).getScoreHolderDisplayName();
                  long score = scoreBuffer.get(i).getRawScore();

                  GameEngineLib.GPGAddLeaderboardEntry(rank, playerName, score);
               }

               scoreBuffer.release();
            }
         }

         GameEngineLib.GPGOnLeaderboardQueryFinished();
      });
   }

   public static void requestLeaderboardScoresAroundUser(String pLeaderboardID, int pPositionsCount)
   {
      PlayGames.getLeaderboardsClient(smActivity).loadPlayerCenteredScores
      (
         pLeaderboardID,
         LeaderboardVariant.TIME_SPAN_ALL_TIME,
         LeaderboardVariant.COLLECTION_PUBLIC,
         pPositionsCount
      )
      .addOnCompleteListener(pTask ->
      {
         if(pTask.isSuccessful())
         {
            LeaderboardsClient.LeaderboardScores scores = pTask.getResult().get();

            if(scores != null)
            {
               LeaderboardScoreBuffer scoreBuffer = scores.getScores();

               for(int i = 0; i < scoreBuffer.getCount(); i++)
               {
                  long rank = scoreBuffer.get(i).getRank();
                  String playerName = scoreBuffer.get(i).getScoreHolderDisplayName();
                  long score = scoreBuffer.get(i).getRawScore();

                  GameEngineLib.GPGAddLeaderboardEntry(rank, playerName, score);
               }

               scoreBuffer.release();
            }
         }

         GameEngineLib.GPGOnLeaderboardQueryFinished();
      });
   }

   public static void requestDLCPurchase(String pProductID)
   {
      QueryProductDetailsParams queryProductDetailsParams = QueryProductDetailsParams.newBuilder()
         .setProductList(List.of(QueryProductDetailsParams.Product.newBuilder()
            .setProductId(pProductID)
            .setProductType(BillingClient.ProductType.INAPP)
            .build()))
         .build();

      smBillingClient.queryProductDetailsAsync(queryProductDetailsParams, smProductDetailsResponseListener);
   }

   public static void openWebPage(String pURL)
   {
      Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(pURL));
      smActivity.startActivity(browserIntent);
   }

   public static void showFullscreenAd(String pID)
   {
      smActivity.runOnUiThread(new Runnable()
      {
         @Override
         public void run()
         {
            AdRequest adRequest = new AdRequest.Builder().build();

            InterstitialAd.load(smActivity, pID, adRequest, new InterstitialAdLoadCallback()
            {
               @Override
               public void onAdLoaded(@NonNull InterstitialAd pAd)
               {
                  smFullscreenAd = pAd;

                  smFullscreenAd.setFullScreenContentCallback(new FullScreenContentCallback()
                  {
                     @Override
                     public void onAdDismissedFullScreenContent()
                     {
                        smFullscreenAd = null;
                     }

                     @Override
                     public void onAdShowedFullScreenContent()
                     {
                        smFullscreenAd = null;
                     }
                  });

                  if(smFullscreenAd != null)
                  {
                     smFullscreenAd.show(smActivity);
                  }
               }
            });
         }
      });
   }

   private static void updateAuthenticationState()
   {
      GamesSignInClient gamesSignInClient = PlayGames.getGamesSignInClient(smActivity);
      gamesSignInClient.isAuthenticated().addOnCompleteListener(isAuthenticatedTask ->
      {
         smAuthenticated =
                 (isAuthenticatedTask.isSuccessful() && isAuthenticatedTask.getResult().isAuthenticated());

         if(smAuthenticated)
         {
            PlayGames.getPlayersClient(smActivity).getCurrentPlayer().addOnCompleteListener(pTask ->
            {
               smPlayerName = pTask.getResult().getDisplayName();

               GameEngineLib.GPGOnLogInFinished();
            });
         }
         else
         {
            GameEngineLib.GPGOnLogInFinished();
         }
      });
   }
}
