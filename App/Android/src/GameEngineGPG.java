
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
import com.google.android.gms.games.LeaderboardsClient;
import com.google.android.gms.games.leaderboard.LeaderboardScoreBuffer;
import com.google.android.gms.games.leaderboard.LeaderboardVariant;

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
      gamesSignInClient.signIn().addOnCompleteListener(pTask ->
      {
         updateAuthenticationState();
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
               smPlayerID = pTask.getResult().getPlayerId();
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
