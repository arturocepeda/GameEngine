
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
      FullScreenSizeMode eFullScreenSizeMode;
      bool bVertexDataDirty;

      void updateVertexData();

   public:
      ComponentSprite(Entity* Owner);
      ~ComponentSprite();

      void update();

      const Vector2& getCenter() const;
      const Vector2& getSize() const;
      SpriteLayer getLayer() const;
      UVMode getUVMode() const;
      FullScreenSizeMode getFullScreenSizeMode() const;
      const Core::ObjectName& getTextureAtlasName() const;

      void setCenter(const Vector2& Center);
      void setSize(const Vector2& Size);
      void setLayer(SpriteLayer Layer);
      void setUVMode(UVMode Mode);
      void setFullScreenSizeMode(FullScreenSizeMode Mode);
      void setTextureAtlasName(const Core::ObjectName& AtlasName);

      bool isOver(const Vector2& ScreenPosition) const;
   };
}}
