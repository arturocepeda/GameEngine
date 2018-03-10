
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentTransform.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentTransform.h"
#include "GEEntity.h"
#include "Core/GEGeometry.h"

using namespace GE;
using namespace GE::Entities;
using namespace GE::Core;

const ObjectName ResetActionName = ObjectName("Reset");

ComponentTransform::ComponentTransform(Entity* Owner)
   : Component(Owner)
   , vPosition(Vector3::Zero)
   , vScale(Vector3::One)
   , vForward(Vector3::UnitZ)
   , vUp(Vector3::UnitY)
{
   cClassName = ObjectName("Transform");

   updateWorldMatrix();

   GERegisterProperty(Vector3, Position);
   GERegisterPropertySpecialEditor(Vector3, Orientation, PropertyEditor::Rotation);
   GERegisterProperty(Vector3, Scale);

   registerAction(ResetActionName, [this]{ reset(); });
}

ComponentTransform::~ComponentTransform()
{
}

void ComponentTransform::updateDirectionVectors()
{
   Matrix4Transform(cRotation.getRotationMatrix(), Vector3::UnitZ, &vForward);
   Matrix4Transform(cRotation.getRotationMatrix(), Vector3::UnitY, &vUp);
   vRight = vForward.crossProduct(vUp);
}

void ComponentTransform::reset()
{
   vPosition = Vector3::Zero;
   cRotation = Rotation(Quaternion());
   vScale = Vector3::One;

   vRight = Vector3::UnitX;
   vUp = Vector3::UnitY;
   vForward = -Vector3::UnitZ;

   updateWorldMatrix();
}

void ComponentTransform::updateWorldMatrix()
{
   Geometry::createTRSMatrix(vPosition, cRotation, vScale, &mLocalWorldMatrix);
   mGlobalWorldMatrix = mLocalWorldMatrix;

   Entity* cParent = cOwner->getParent();

   if(cParent)
   {
      Matrix4 mCurrentWorldMatrix = mGlobalWorldMatrix;
      ComponentTransform* cParentTransform = cParent->getComponent<ComponentTransform>();
      GEAssert(cParentTransform);
      Matrix4Multiply(cParentTransform->mGlobalWorldMatrix, mCurrentWorldMatrix, &mGlobalWorldMatrix);
   }

   for(uint i = 0; i < cOwner->getChildrenCount(); i++)
   {
      Entity* cChild = cOwner->getChildByIndex(i);
      ComponentTransform* cChildTransform = cChild->getComponent<ComponentTransform>();
      cChildTransform->updateWorldMatrix();
   }
}

void ComponentTransform::move(float DX, float DY, float DZ)
{
   move(Vector3(DX, DY, DZ));
}

void ComponentTransform::move(const Vector3& Move)
{
   setPosition(vPosition + Move);
}

void ComponentTransform::rotate(const Rotation& Rotate)
{
   setRotation(Rotate * cRotation);
}

void ComponentTransform::scale(float SX, float SY, float SZ)
{
   setScale(Vector3(vScale.X * SX, vScale.Y * SY, vScale.Z * SZ));
}

void ComponentTransform::scale(float ScaleFactor)
{
   setScale(Vector3(vScale.X * ScaleFactor, vScale.Y * ScaleFactor, vScale.Z * ScaleFactor));
}

void ComponentTransform::scale(const Vector3& Scale)
{
   setScale(Vector3(vScale.X * Scale.X, vScale.Y * Scale.Y, vScale.Z * Scale.Z));
}

Vector3& ComponentTransform::getPosition()
{
   return vPosition;
}

Rotation& ComponentTransform::getRotation()
{
   return cRotation;
}

Vector3& ComponentTransform::getOrientation()
{
   vEulerAngles = cRotation.getEulerAngles() * GE_RAD2DEG;
   return vEulerAngles;
}

Vector3& ComponentTransform::getScale()
{
   return vScale;
}

Vector3 ComponentTransform::getWorldPosition() const
{
   return Vector3(
      mGlobalWorldMatrix.m[GE_M4_1_4],
      mGlobalWorldMatrix.m[GE_M4_2_4],
      mGlobalWorldMatrix.m[GE_M4_3_4]);
}

Rotation ComponentTransform::getWorldRotation() const
{
   Vector3 vWorldInverseScale = getWorldScale();
   vWorldInverseScale.X = 1.0f / vWorldInverseScale.X;
   vWorldInverseScale.Y = 1.0f / vWorldInverseScale.Y;
   vWorldInverseScale.Z = 1.0f / vWorldInverseScale.Z;

   Matrix4 mWorldRotation = mGlobalWorldMatrix;
   mWorldRotation.m[GE_M4_1_4] = 0.0f;
   mWorldRotation.m[GE_M4_2_4] = 0.0f;
   mWorldRotation.m[GE_M4_3_4] = 0.0f;
   Matrix4Scale(&mWorldRotation, vWorldInverseScale);

   return Rotation(mWorldRotation);
}

Vector3 ComponentTransform::getWorldScale() const
{
   Vector3 vScalingFactorX = Vector3(mGlobalWorldMatrix.m[GE_M4_1_1], mGlobalWorldMatrix.m[GE_M4_2_1], mGlobalWorldMatrix.m[GE_M4_3_1]);
   Vector3 vScalingFactorY = Vector3(mGlobalWorldMatrix.m[GE_M4_1_2], mGlobalWorldMatrix.m[GE_M4_2_2], mGlobalWorldMatrix.m[GE_M4_3_2]);
   Vector3 vScalingFactorZ = Vector3(mGlobalWorldMatrix.m[GE_M4_1_3], mGlobalWorldMatrix.m[GE_M4_2_3], mGlobalWorldMatrix.m[GE_M4_3_3]);

   return Vector3(vScalingFactorX.getLength(), vScalingFactorY.getLength(), vScalingFactorZ.getLength());
}

const Matrix4& ComponentTransform::getLocalWorldMatrix() const
{
   return mLocalWorldMatrix;
}

const Matrix4& ComponentTransform::getGlobalWorldMatrix() const
{
   return mGlobalWorldMatrix;
}

const Vector3& ComponentTransform::getForwardVector() const
{
   return vForward;
}

const Vector3& ComponentTransform::getUpVector() const
{
   return vUp;
}

const Vector3& ComponentTransform::getRightVector() const
{
   return vRight;
}

void ComponentTransform::setPosition(float X, float Y, float Z)
{
   setPosition(Vector3(X, Y, Z));
}

void ComponentTransform::setPosition(const Vector3& Position)
{
   vPosition = Position;
   updateWorldMatrix();
}

void ComponentTransform::setRotation(const Rotation& R)
{
   cRotation = R;
   updateDirectionVectors();
   updateWorldMatrix();
}

void ComponentTransform::setOrientation(const Vector3& EulerAnglesInDegrees)
{
   setRotation(Rotation(EulerAnglesInDegrees * GE_DEG2RAD));
}

void ComponentTransform::setScale(float X, float Y, float Z)
{
   setScale(Vector3(X, Y, Z));
}

void ComponentTransform::setScale(float ScaleFactor)
{
   setScale(Vector3(ScaleFactor, ScaleFactor, ScaleFactor));
}

void ComponentTransform::setScale(const Vector3& Scale)
{
   vScale = Scale;
   updateWorldMatrix();
}

void ComponentTransform::setUniformScale(float ScaleFactor)
{
   setScale(Vector3(ScaleFactor, ScaleFactor, ScaleFactor));
}

void ComponentTransform::setLocalWorldMatrix(const Matrix4& LocalWorldMatrix)
{
   Geometry::extractTRSFromMatrix(LocalWorldMatrix, &vPosition, &cRotation, &vScale);
   updateDirectionVectors();
   updateWorldMatrix();
}

void ComponentTransform::setForwardVector(const Vector3& Forward)
{
   vRight = Vector3CrossProduct(Forward, Vector3::UnitY);
   vRight.normalize();
   vUp = Vector3CrossProduct(vRight, Forward);
   vUp.normalize();

   Matrix4 matRotation;

   matRotation.m[GE_M4_1_1] = -vRight.X;
   matRotation.m[GE_M4_2_1] = -vRight.Y;
   matRotation.m[GE_M4_3_1] = -vRight.Z;
   matRotation.m[GE_M4_4_1] = 0.0f;

   matRotation.m[GE_M4_1_2] = vUp.X;
   matRotation.m[GE_M4_2_2] = vUp.Y;
   matRotation.m[GE_M4_3_2] = vUp.Z;
   matRotation.m[GE_M4_4_2] = 0.0f;

   matRotation.m[GE_M4_1_3] = Forward.X;
   matRotation.m[GE_M4_2_3] = Forward.Y;
   matRotation.m[GE_M4_3_3] = Forward.Z;
   matRotation.m[GE_M4_4_3] = 0.0f;

   matRotation.m[GE_M4_1_4] = 0.0f;
   matRotation.m[GE_M4_2_4] = 0.0f;
   matRotation.m[GE_M4_3_4] = 0.0f;
   matRotation.m[GE_M4_4_4] = 1.0f;

   setRotation(Rotation(matRotation));
}
