
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda
//  Game Engine
//
//  Android
//
//  --- GameEngineLib.java ---
//
//////////////////////////////////////////////////////////////////

package com.GameEngine.Main;

import android.app.Activity;
import android.content.res.AssetManager;

public class GameEngineLib
{
   public static void LoadSharedLibraries()
   {
      System.loadLibrary("GameEngineAndroid");
   }
      
   public static native void Initialize(int width, int height);
   public static native void UpdateFrame();
   public static native void Pause();
   public static native void Resume();

   public static native void CreateAssetManager(AssetManager assetManager);
   public static native void SendInternalStoragePath(String internalStoragePath);

   public static native void SetAudioManagerValues(int sampleRate, int framesPerBuffer);

   public static native void InputTouchDown(int index, float x, float y);
   public static native void InputTouchMove(int index, float x, float y);
   public static native void InputTouchUp(int index, float x, float y);
   public static native void InputButtonDown(int button);
   public static native void InputButtonUp(int button);
   public static native void UpdateAccelerometerStatus(float x, float y, float z);
   public static native void UpdateDeviceRotationVector(float x, float y, float z, float w);

   public static native void GPGInitialize(Activity mainActivity);
   public static native void GPGOnLogInFinished();
}
