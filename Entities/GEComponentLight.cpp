
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentLight.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentLight.h"
#include "GEEntity.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;

//
//  ComponentLight
//
ComponentLight::ComponentLight(Entity* Owner)
   : Component(Owner)
   , eLightType(LightType::Directional)
   , cColor(1.0f, 1.0f, 1.0f)
   , fLinearAttenuation(1.0f)
   , fSpotAngle(1.0f)
   , fShadowIntensity(0.0f)
{
   cClassName = ObjectName("Light");

   cTransform = cOwner->getComponent<ComponentTransform>();

   GERegisterPropertyEnum(LightType, LightType);
   GERegisterProperty(Color, Color);
   GERegisterProperty(Float, LinearAttenuation);
   GERegisterProperty(Float, SpotAngle);
   GERegisterProperty(Float, ShadowIntensity);
}

ComponentLight::~ComponentLight()
{
}

ComponentTransform* ComponentLight::getTransform() const
{
   return cTransform;
}

LightType ComponentLight::getLightType() const
{
   return eLightType;
}

const Color& ComponentLight::getColor() const
{
   return cColor;
}

float ComponentLight::getLinearAttenuation() const
{
   return fLinearAttenuation;
}

float ComponentLight::getSpotAngle() const
{
   return fSpotAngle;
}

float ComponentLight::getShadowIntensity() const
{
   return fShadowIntensity;
}

Vector3 ComponentLight::getDirection() const
{
   const Matrix4& mRotationMatrix = cTransform->getWorldRotation().getRotationMatrix();

   // apply the rotation matrix to the down vector (0, -1, 0)
   return Vector3(-mRotationMatrix.m[GE_M4_1_2], -mRotationMatrix.m[GE_M4_2_2], -mRotationMatrix.m[GE_M4_3_2]);
}

void ComponentLight::setLightType(LightType eLightType)
{
   this->eLightType = eLightType;
}

void ComponentLight::setColor(const Color& cColor)
{
   this->cColor = cColor;
}

void ComponentLight::setLinearAttenuation(float LinearAttenuation)
{
   fLinearAttenuation = LinearAttenuation;
}

void ComponentLight::setSpotAngle(float SpotAngle)
{
   fSpotAngle = SpotAngle;
}

void ComponentLight::setShadowIntensity(float Intensity)
{
   fShadowIntensity = Intensity;
}
