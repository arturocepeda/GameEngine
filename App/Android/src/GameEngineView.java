
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda
//  Game Engine
//
//  Android
//
//  --- GameEngineView.java ---
//
//////////////////////////////////////////////////////////////////

package com.GameEngine.Main;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class GameEngineView extends GLSurfaceView
{
   public GameEngineView(Context context)
   {
      super(context);

      setEGLContextClientVersion(3);
      setRenderer(new Renderer());
   }
   private static class Renderer implements GLSurfaceView.Renderer
   {
      public void onSurfaceCreated(GL10 gl, EGLConfig config)
      {
      }

      public void onDrawFrame(GL10 gl)
      {
         GameEngineLib.UpdateFrame();
      }

      public void onSurfaceChanged(GL10 gl, int width, int height)
      {
         GLES20.glViewport(0, 0, width, height);
         GameEngineLib.Initialize(width, height);
      }
   }

   @Override
   public boolean onTouchEvent(MotionEvent event)
   {
      int index = event.getActionIndex();

      switch(event.getActionMasked())
      {
      case MotionEvent.ACTION_DOWN:
      case MotionEvent.ACTION_POINTER_DOWN:
         GameEngineLib.InputTouchDown(event.getPointerId(index), event.getX(index), event.getY(index));
         break;
      case MotionEvent.ACTION_MOVE:
         for(int pointerIndex = 0; pointerIndex < event.getPointerCount(); ++pointerIndex)
            GameEngineLib.InputTouchMove(event.getPointerId(pointerIndex), event.getX(pointerIndex), event.getY(pointerIndex));
         break;
      case MotionEvent.ACTION_UP:
      case MotionEvent.ACTION_POINTER_UP:
      case MotionEvent.ACTION_CANCEL:
         GameEngineLib.InputTouchUp(event.getPointerId(index), event.getX(index), event.getY(index));
         break;
      }

      return true;
   }
}
