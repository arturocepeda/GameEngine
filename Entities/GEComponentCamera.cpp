
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentCamera.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentCamera.h"
#include "GEEntity.h"
#include "Core/GEEvents.h"
#include "Core/GEDevice.h"
#include "Rendering/GERenderSystem.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;

const float ComponentCamera::DefaultFOV = 65.0f;
const float ComponentCamera::DefaultNearZ = 0.25f;
const float ComponentCamera::DefaultFarZ = 500.0f;

//
//  ComponentCamera
//
ComponentCamera::ComponentCamera(Entity* Owner)
   : Component(Owner)
   , fFOV(DefaultFOV)
   , fNearZ(DefaultNearZ)
   , fFarZ(DefaultFarZ)
{
   cClassName = ObjectName("Camera");
   cTransform = cOwner->getComponent<ComponentTransform>();
   calculateProjectionMatrix();

   if(!RenderSystem::getInstance()->getActiveCamera())
   {
      RenderSystem::getInstance()->setActiveCamera(this);
   }

#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::connectStaticEventCallback(Events::RenderingSurfaceChanged, this, [this](const EventArgs* args) -> bool
   {
      calculateProjectionMatrix();
      return false;
   });
#endif

   GERegisterProperty(Float, FOV);
   GERegisterProperty(Float, NearZ);
   GERegisterProperty(Float, FarZ);
}

ComponentCamera::~ComponentCamera()
{
   if(RenderSystem::getInstance()->getActiveCamera() == this)
   {
      RenderSystem::getInstance()->setActiveCamera(0);
   }

#if defined (GE_EDITOR_SUPPORT)
   EventHandlingObject::disconnectStaticEventCallback(Events::RenderingSurfaceChanged, this);
#endif
}

void ComponentCamera::update()
{
   calculateViewMatrix();
   Matrix4Multiply(matProjection, matView, &matViewProjection);
}

void ComponentCamera::calculateProjectionMatrix()
{
#if defined (GE_PLATFORM_WP8)
   float fAspect = Device::Orientation == DeviceOrientation::Portrait
      ? 1.0f / Device::getAspectRatio()
      : Device::getAspectRatio();
#else
   float fAspect = 1.0f / Device::getAspectRatio();
#endif

   Matrix4MakePerspective(fFOV * GE_DEG2RAD, fAspect, fNearZ, fFarZ, &matProjection);

#if defined (GE_RENDERING_API_DIRECTX)
   if(Device::Orientation == DeviceOrientation::Landscape)
      Matrix4RotateZ(&matProjection, -GE_HALFPI);
#endif
}

void ComponentCamera::calculateViewMatrix()
{
   Vector3 pLookingAt = cTransform->getPosition() + cTransform->getForwardVector();
   Matrix4MakeLookAt(cTransform->getPosition(), pLookingAt, cTransform->getUpVector(), &matView);
}

ComponentTransform* ComponentCamera::getTransform() const
{
   return cTransform;
}

float ComponentCamera::getFOV() const
{
   return fFOV;
}

float ComponentCamera::getNearZ() const
{
   return fNearZ;
}

float ComponentCamera::getFarZ() const
{
   return fFarZ;
}

const Matrix4 ComponentCamera::getViewMatrix() const
{
   return matView;
}

const Matrix4 ComponentCamera::getProjectionMatrix() const
{
   return matProjection;
}

const Matrix4 ComponentCamera::getViewProjectionMatrix() const
{
   return matViewProjection;
}

void ComponentCamera::setFOV(float FOV)
{
   fFOV = FOV;
   calculateProjectionMatrix();
}

void ComponentCamera::setNearZ(float NearZ)
{
   fNearZ = NearZ;
   calculateProjectionMatrix();
}

void ComponentCamera::setFarZ(float FarZ)
{
   fFarZ = FarZ;
   calculateProjectionMatrix();
}

void ComponentCamera::lookAt(float X, float Y, float Z)
{
   lookAt(Vector3(X, Y, Z));
}

void ComponentCamera::lookAt(const Vector3& LookAt)
{
   Vector3 vDirection = LookAt - cTransform->getPosition();
   vDirection.normalize();
   cTransform->setForwardVector(vDirection);
}

void ComponentCamera::orbit(const Vector3& ReferencePoint, float Distance, float Theta, float Phi)
{
   float fSinPhi = sinf(Phi);

   cTransform->setPosition(ReferencePoint.X + (Distance * sinf(Theta) * fSinPhi),
                           ReferencePoint.Y + (Distance * -cosf(Phi)), 
                           ReferencePoint.Z + (Distance * cosf(Theta) * fSinPhi));
   lookAt(ReferencePoint);
}

void ComponentCamera::worldToScreen(const Vector3& PositionWorld, Vector2* OutPositionScreen) const
{
   Vector3 vPositionScreen;
   Matrix4Transform(matViewProjection, PositionWorld, &vPositionScreen);

   OutPositionScreen->X = vPositionScreen.X;
   OutPositionScreen->Y = vPositionScreen.Y * Device::getAspectRatio();
}

void ComponentCamera::screenToWorld(const Vector2& PositionScreen, Vector3* OutWorldPointNear, Vector3* OutWorldPointFar) const
{
   Matrix4 matInverseViewProjection = matViewProjection;
   Matrix4Invert(&matInverseViewProjection);

   Vector3 vPositionScreen = Vector3(PositionScreen.X, PositionScreen.Y / Device::getAspectRatio(), 0.0f);
   Matrix4Transform(matInverseViewProjection, vPositionScreen, OutWorldPointNear);
   vPositionScreen.Z = 1.0f;
   Matrix4Transform(matInverseViewProjection, vPositionScreen, OutWorldPointFar);
}

Physics::Ray ComponentCamera::getScreenRay(const Vector2& PositionScreen) const
{
   Vector3 vWorldPositionNearPlane;
   Vector3 vWorldPositionFarPlane;
   screenToWorld(PositionScreen, &vWorldPositionNearPlane, &vWorldPositionFarPlane);

   Vector3 vDir = vWorldPositionFarPlane - vWorldPositionNearPlane;
   vDir.normalize();
   return Physics::Ray(vWorldPositionNearPlane, vDir);
}
