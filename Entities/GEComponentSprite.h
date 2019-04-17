
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentSprite.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentRenderable.h"

namespace GE { namespace Entities
{
   GESerializableEnum(SpriteLayer)
   {
      GUI,
      Pre3D,
      PostGUI,

      Count
   };


   GESerializableEnum(UVMode)
   {
      Normal,
      FlipU,
      FlipV,
      FlipUV,

      Count
   };


   GESerializableEnum(CenterMode)
   {
      Absolute,
      Relative,

      Count
   };


   GESerializableEnum(FullScreenSizeMode)
   {
      None,
      Stretch,
      MatchWidth,
      MatchHeight,
      
      Count
   };

   
   class ComponentSprite : public ComponentRenderable
   {
   private:
      Vector2 vCenter;
      Vector2 vSize;
      float mScaledYSize;
      Core::ObjectName cTextureAtlasName;
      SpriteLayer iLayer;
      UVMode eUVMode;
      CenterMode mCenterMode;
      FullScreenSizeMode eFullScreenSizeMode;
      bool bVertexDataDirty;

      void updateVertexData();

   public:
      static const Core::ObjectName ClassName;

      ComponentSprite(Entity* Owner);
      ~ComponentSprite();

      void update();

      GEDefaultGetter(const Vector2&, Center, v)
      GEDefaultGetter(const Vector2&, Size, v)
      GEDefaultGetter(float, ScaledYSize, m)
      GEDefaultGetter(SpriteLayer, Layer, i)
      GEDefaultGetter(UVMode, UVMode, e)
      GEDefaultGetter(CenterMode, CenterMode, m)
      GEDefaultGetter(FullScreenSizeMode, FullScreenSizeMode, e)
      GEDefaultGetter(const Core::ObjectName&, TextureAtlasName, c)

      void setCenter(const Vector2& Center);
      void setSize(const Vector2& Size);
      void setScaledYSize(float pSize);
      void setLayer(SpriteLayer Layer);
      void setUVMode(UVMode Mode);
      void setCenterMode(CenterMode pMode);
      void setFullScreenSizeMode(FullScreenSizeMode Mode);
      void setTextureAtlasName(const Core::ObjectName& AtlasName);

      bool isOver(const Vector2& ScreenPosition) const;
   };
}}
