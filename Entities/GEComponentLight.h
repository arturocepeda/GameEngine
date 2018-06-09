
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentLight.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentTransform.h"

namespace GE { namespace Entities
{
   GESerializableEnum(LightType)
   {
      Directional,
      Point,
      Spot,

      Count
   };


   class ComponentLight : public Component
   {
   private:
      ComponentTransform* cTransform;

      LightType eLightType;
      Color cColor;
      float fLinearAttenuation;
      float fSpotAngle;
      float fShadowIntensity;

   public:
      static ComponentType getType() { return ComponentType::Light; }

      ComponentLight(Entity* Owner);
      ~ComponentLight();

      ComponentTransform* getTransform() const;
      LightType getLightType() const;
      const Color& getColor() const;
      float getLinearAttenuation() const;
      float getSpotAngle() const;
      float getShadowIntensity() const;
      Vector3 getDirection() const;

      void setLightType(LightType eLightType);
      void setColor(const Color& cColor);
      void setLinearAttenuation(float LinearAttenuation);
      void setSpotAngle(float SpotAngle);
      void setShadowIntensity(float Intensity);
   };
}}
