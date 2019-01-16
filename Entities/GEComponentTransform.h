
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Entities
//
//  --- GEComponentTransform.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponent.h"
#include "GEComponentType.h"
#include "GEEntity.h"
#include "Core/GEGeometry.h"
#include "Core/GESerializable.h"

namespace GE { namespace Entities
{
   class ComponentTransform : public Component
   {
   protected:
      friend class Scene;

      Vector3 vPosition;
      Rotation cRotation;
      Vector3 vEulerAngles;
      Vector3 vScale;

      Matrix4 mLocalWorldMatrix;
      Matrix4 mGlobalWorldMatrix;

      Vector3 vForward;
      Vector3 vUp;
      Vector3 vRight;

      inline void updateDirectionVectors()
      {
         Matrix4Transform(cRotation.getRotationMatrix(), Vector3::UnitZ, &vForward);
         Matrix4Transform(cRotation.getRotationMatrix(), Vector3::UnitY, &vUp);
         vRight = vForward.crossProduct(vUp);
      }

   public:
      static ComponentType getType() { return ComponentType::Transform; }

      ComponentTransform(Entity* Owner);
      virtual ~ComponentTransform();

      inline void move(const Vector3& Move)
      {
         setPosition(vPosition + Move);
      }
      inline void move(float DX, float DY, float DZ)
      {
         move(Vector3(DX, DY, DZ));
      }
      inline void rotate(const Rotation& Rotate)
      {
         setRotation(Rotate * cRotation);
      }
      inline void scale(const Vector3& Scale)
      {
         setScale(Vector3(vScale.X * Scale.X, vScale.Y * Scale.Y, vScale.Z * Scale.Z));
      }
      inline void scale(float SX, float SY, float SZ)
      {
         setScale(Vector3(vScale.X * SX, vScale.Y * SY, vScale.Z * SZ));
      }
      inline void scale(float ScaleFactor)
      {
         setScale(Vector3(vScale.X * ScaleFactor, vScale.Y * ScaleFactor, vScale.Z * ScaleFactor));
      }

      inline void reset()
      {
         vPosition = Vector3::Zero;
         cRotation = Rotation(Quaternion());
         vScale = Vector3::One;

         vRight = Vector3::UnitX;
         vUp = Vector3::UnitY;
         vForward = -Vector3::UnitZ;

         updateWorldMatrix();
      }
      inline void updateWorldMatrix()
      {
         Core::Geometry::createTRSMatrix(vPosition, cRotation, vScale, &mLocalWorldMatrix);
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

      inline Vector3& getPosition()
      {
         return vPosition;
      }
      inline Rotation& getRotation()
      {
         return cRotation;
      }
      inline Vector3& getOrientation()
      {
         vEulerAngles = cRotation.getEulerAngles() * GE_RAD2DEG;
         return vEulerAngles;
      }
      inline Vector3& getScale()
      {
         return vScale;
      }

      inline Vector3 getWorldPosition() const
      {
         return Vector3(
            mGlobalWorldMatrix.m[GE_M4_1_4],
            mGlobalWorldMatrix.m[GE_M4_2_4],
            mGlobalWorldMatrix.m[GE_M4_3_4]);
      }
      inline Rotation getWorldRotation() const
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
      inline Vector3 getWorldOrientation() const
      {
         Rotation worldRotation = getWorldRotation();
         return worldRotation.getEulerAngles() * GE_RAD2DEG;
      }
      inline Vector3 getWorldScale() const
      {
         Vector3 vScalingFactorX = Vector3(mGlobalWorldMatrix.m[GE_M4_1_1], mGlobalWorldMatrix.m[GE_M4_2_1], mGlobalWorldMatrix.m[GE_M4_3_1]);
         Vector3 vScalingFactorY = Vector3(mGlobalWorldMatrix.m[GE_M4_1_2], mGlobalWorldMatrix.m[GE_M4_2_2], mGlobalWorldMatrix.m[GE_M4_3_2]);
         Vector3 vScalingFactorZ = Vector3(mGlobalWorldMatrix.m[GE_M4_1_3], mGlobalWorldMatrix.m[GE_M4_2_3], mGlobalWorldMatrix.m[GE_M4_3_3]);

         return Vector3(vScalingFactorX.getLength(), vScalingFactorY.getLength(), vScalingFactorZ.getLength());
      }

      inline const Matrix4& getLocalWorldMatrix() const
      {
         return mLocalWorldMatrix;
      }
      inline const Matrix4& getGlobalWorldMatrix() const
      {
         return mGlobalWorldMatrix;
      }

      inline const Vector3& getForwardVector() const
      {
         return vForward;
      }
      inline const Vector3& getUpVector() const
      {
         return vUp;
      }
      inline const Vector3& getRightVector() const
      {
         return vRight;
      }

      inline void setPosition(const Vector3& Position)
      {
         vPosition = Position;
         updateWorldMatrix();
      }
      inline void setPosition(float X, float Y, float Z)
      {
         setPosition(Vector3(X, Y, Z));
      }
      inline void setRotation(const Rotation& R)
      {
         cRotation = R;
         updateDirectionVectors();
         updateWorldMatrix();
      }
      inline void setOrientation(const Vector3& EulerAnglesInDegrees)
      {
         setRotation(Rotation(EulerAnglesInDegrees * GE_DEG2RAD));
      }
      inline void setScale(const Vector3& Scale)
      {
         vScale = Scale;
         updateWorldMatrix();
      }
      inline void setScale(float X, float Y, float Z)
      {
         setScale(Vector3(X, Y, Z));
      }
      inline void setScale(float ScaleFactor)
      {
         setScale(Vector3(ScaleFactor, ScaleFactor, ScaleFactor));
      }
      inline void setUniformScale(float ScaleFactor)
      {
         setScale(Vector3(ScaleFactor, ScaleFactor, ScaleFactor));
      }

      inline void setLocalWorldMatrix(const Matrix4& LocalWorldMatrix)
      {
         Core::Geometry::extractTRSFromMatrix(LocalWorldMatrix, &vPosition, &cRotation, &vScale);
         updateDirectionVectors();
         updateWorldMatrix();
      }

      inline void setForwardVector(const Vector3& Forward)
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
   };
}}
