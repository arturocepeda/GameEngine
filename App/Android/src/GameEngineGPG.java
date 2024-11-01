
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
   private static Activity mActivity;
   private static boolean mAuthenticated = false;
   private static String mPlayerID;
   private static String mPlayerName;

   public static void initialize(Activity pActivity)
   {
      mActivity = pActivity;
      PlayGamesSdk.initialize(pActivity);
      updateAuthenticationState();
      GameEngineLib.GPGInitialize(pActivity);
   }

   public static String getUserName()
   {
      return mPlayerName;
   }

   public static boolean loggedIn()
   {
      return mAuthenticated;
   }

   public static void logIn()
   {
      GamesSignInClient gamesSignInClient = PlayGames.getGamesSignInClient(mActivity);
      gamesSignInClient.signIn().addOnCompleteListener(isAuthenticatedTask ->
      {
         updateAuthenticationState();
      });
   }

   private static void updateAuthenticationState()
   {
      GamesSignInClient gamesSignInClient = PlayGames.getGamesSignInClient(mActivity);
      gamesSignInClient.isAuthenticated().addOnCompleteListener(isAuthenticatedTask ->
      {
         mAuthenticated =
                 (isAuthenticatedTask.isSuccessful() && isAuthenticatedTask.getResult().isAuthenticated());

         if(mAuthenticated)
         {
            PlayGames.getPlayersClient(mActivity).getCurrentPlayer().addOnCompleteListener(mTask ->
            {
               mPlayerID = mTask.getResult().getPlayerId();
               mPlayerName = mTask.getResult().getDisplayName();

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
