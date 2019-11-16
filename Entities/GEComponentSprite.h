
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


   GESerializableEnum(SizeMode)
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
      Core::ObjectName cTextureAtlasName;
      SpriteLayer iLayer;
      UVMode eUVMode;
      CenterMode mCenterMode;
      SizeMode mSizeMode;
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
      GEDefaultGetter(SpriteLayer, Layer, i)
      GEDefaultGetter(UVMode, UVMode, e)
      GEDefaultGetter(CenterMode, CenterMode, m)
      GEDefaultGetter(SizeMode, SizeMode, m)
      GEDefaultGetter(FullScreenSizeMode, FullScreenSizeMode, e)
      GEDefaultGetter(const Core::ObjectName&, TextureAtlasName, c)

      void setCenter(const Vector2& Center);
      void setSize(const Vector2& Size);
      void setLayer(SpriteLayer Layer);
      void setUVMode(UVMode Mode);
      void setCenterMode(CenterMode pMode);
      void setSizeMode(SizeMode pMode);
      void setFullScreenSizeMode(FullScreenSizeMode Mode);
      void setTextureAtlasName(const Core::ObjectName& AtlasName);

      bool isOver(const Vector2& ScreenPosition) const;
   };
}}
