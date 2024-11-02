
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda
//  Game Engine
//
//  Android
//
//  --- GameEnginePlayGames.java ---
//
//////////////////////////////////////////////////////////////////

package com.GameEngine.Main;

import android.app.Activity;

import com.google.android.gms.games.PlayGamesSdk;
import com.google.android.gms.games.PlayGames;
import com.google.android.gms.games.GamesSignInClient;

public class GameEngineGPG
{
   private static Activity smActivity;
   private static boolean smAuthenticated = false;
   private static String smPlayerID;
   private static String smPlayerName;

   public static void initialize(Activity pActivity)
   {
      smActivity = pActivity;
      PlayGamesSdk.initialize(pActivity);
      updateAuthenticationState();
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
      gamesSignInClient.signIn().addOnCompleteListener(isAuthenticatedTask ->
      {
         updateAuthenticationState();
      });
   }

   public static void updateLeaderboardScore(String pLeaderboardID, long pScore)
   {
      PlayGames.getLeaderboardsClient(smActivity)
         .submitScore(pLeaderboardID, pScore);
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
            PlayGames.getPlayersClient(smActivity).getCurrentPlayer().addOnCompleteListener(mTask ->
            {
               smPlayerID = mTask.getResult().getPlayerId();
               smPlayerName = mTask.getResult().getDisplayName();

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
