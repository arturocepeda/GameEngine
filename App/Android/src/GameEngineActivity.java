
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda
//  Game Engine
//
//  Android
//
//  --- GameEngineActivity.java ---
//
//////////////////////////////////////////////////////////////////

package com.GameEngine.Main;

import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.view.KeyEvent;
import android.content.res.AssetManager;
import android.content.Context;
import android.media.AudioManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

public class GameEngineActivity extends Activity implements SensorEventListener
{
   private static final boolean UseAccelerometer = false;

   private static AssetManager mAssetManager;
   private static AudioManager mAudioManager;

   private GameEngineView mView;

   private SensorManager mSensorManager;
   private Sensor mAccelerometer;
   private Sensor mRotationVector;

   private boolean mKeyPressed[] = new boolean[256];

   @Override
   protected void onCreate(Bundle pSavedInstanceState)
   {
      super.onCreate(pSavedInstanceState);

      createMainView();
      createAssetManager();
      sendInternalStoragePath();
      setupAudioManagerValues();

      if(UseAccelerometer)
      {
         initializeAccelerometer();
      }

      GameEngineGPG.initialize(this);
   }

   private void createMainView()
   {
      mView = new GameEngineView(getApplication());
      setContentView(mView);
   }

   private void createAssetManager()
   {
      mAssetManager = getAssets();
      GameEngineLib.CreateAssetManager(mAssetManager);
   }

   private void sendInternalStoragePath()
   {
      String internalStoragePath = getFilesDir().getAbsolutePath();
      GameEngineLib.SendInternalStoragePath(internalStoragePath);
   }

   private void setupAudioManagerValues()
   {
      mAudioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
      GameEngineLib.SetAudioManagerValues
      (
         Integer.parseInt(mAudioManager.getProperty("android.media.property.OUTPUT_SAMPLE_RATE")),
         Integer.parseInt(mAudioManager.getProperty("android.media.property.OUTPUT_FRAMES_PER_BUFFER"))
      );
   }

   private void initializeAccelerometer()
   {
      mSensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
      mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
      mRotationVector = mSensorManager.getDefaultSensor(Sensor.TYPE_GAME_ROTATION_VECTOR);
      mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_GAME);
      mSensorManager.registerListener(this, mRotationVector, SensorManager.SENSOR_DELAY_GAME);
   }

   private void enableImmersiveMode()
   {
      runOnUiThread(new Runnable()
      {
         public void run()
         {
            mView.setSystemUiVisibility
            (
               View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_FULLSCREEN |
               View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
               View.SYSTEM_UI_FLAG_IMMERSIVE | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            );
         }
      });
   }

   @Override
   protected void onPause()
   {
      super.onPause();

      if(UseAccelerometer)
      {
         mSensorManager.unregisterListener(this);
      }

      GameEngineLib.Pause();
   }
   
   @Override
   protected void onResume()
   {
      super.onResume();

      enableImmersiveMode();

      if(UseAccelerometer)
      {
         mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_FASTEST);
      }

      GameEngineLib.Resume();
   }

   @Override
   public boolean onKeyDown(final int pKeyCode, KeyEvent pEvent)
   {
      if(!mKeyPressed[pKeyCode])
      {
         mKeyPressed[pKeyCode] = true;
         GameEngineLib.InputButtonDown(pKeyCode);
      }

      return super.onKeyDown(pKeyCode, pEvent);
   }

   @Override
   public boolean onKeyUp(final int pKeyCode, KeyEvent pEvent)
   {
      if(mKeyPressed[pKeyCode])
      {
         mKeyPressed[pKeyCode] = false;
         GameEngineLib.InputButtonUp(pKeyCode);
      }

      return super.onKeyUp(pKeyCode, pEvent);
   }

   @Override
   public void onAccuracyChanged(Sensor pSensor, int pAccuracy)
   {
   }

   @Override
   public final void onSensorChanged(SensorEvent pEvent)
   {
      // accelerometer
      if(pEvent.sensor == mAccelerometer)
      {
         GameEngineLib.UpdateAccelerometerStatus(pEvent.values[0], pEvent.values[1], pEvent.values[2]);
      }
      // rotation vector
      else
      {
         float[] qQuaternion = new float[4];
         SensorManager.getQuaternionFromVector(qQuaternion, pEvent.values);
         GameEngineLib.UpdateDeviceRotationVector(qQuaternion[1], qQuaternion[2], qQuaternion[3], qQuaternion[0]);
      }
   }

   public GameEngineActivity()
   {
      GameEngineLib.LoadSharedLibraries();		
   }	
}
