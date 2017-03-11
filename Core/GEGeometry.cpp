
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEGeometry.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEGeometry.h"

using namespace GE;
using namespace GE::Core;

void Geometry::createTRSMatrix(const Vector3& T, const Rotation& R, const Vector3& S, Matrix4* Out)
{
   *Out = R.getRotationMatrix();
   Matrix4Scale(Out, S);
   Matrix4Translate(Out, T);
}

void Geometry::extractTRSFromMatrix(const Matrix4& M, Vector3* OutT, Rotation* OutR, Vector3* OutS)
{
   // scale
   Vector3 vScalingFactorX = Vector3(M.m[GE_M4_1_1], M.m[GE_M4_2_1], M.m[GE_M4_3_1]);
   Vector3 vScalingFactorY = Vector3(M.m[GE_M4_1_2], M.m[GE_M4_2_2], M.m[GE_M4_3_2]);
   Vector3 vScalingFactorZ = Vector3(M.m[GE_M4_1_3], M.m[GE_M4_2_3], M.m[GE_M4_3_3]);

   OutS->X = vScalingFactorX.getLength();
   OutS->Y = vScalingFactorY.getLength();
   OutS->Z = vScalingFactorZ.getLength();

   // rotation
   Vector3 vInverseScale = Vector3(1.0f / OutS->X, 1.0f / OutS->Y, 1.0f / OutS->Z);
   Matrix4 mRotation = M;

   mRotation.m[GE_M4_1_4] = 0.0f;
   mRotation.m[GE_M4_2_4] = 0.0f;
   mRotation.m[GE_M4_3_4] = 0.0f;
   Matrix4Scale(&mRotation, vInverseScale);

   *OutR = Rotation(mRotation);

   // translation
   OutT->X = M.m[GE_M4_1_4];
   OutT->Y = M.m[GE_M4_2_4];
   OutT->Z = M.m[GE_M4_3_4];
}
