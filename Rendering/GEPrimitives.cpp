
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEPrimitives.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEPrimitives.h"
#include "Core/GEConstants.h"

using namespace GE;
using namespace GE::Rendering;

//
//  Primitive
//
Primitive::Primitive()
{
}

const uint Primitive::getNumVertices() const
{
   return (uint)(vVertices.size() / 3);
}

const uint Primitive::getNumIndices() const
{
   return (uint)vIndices.size();
}

const float* Primitive::getVertices() const
{
   return &vVertices[0];
}

const float* Primitive::getNormals() const
{
   return &vNormals[0];
}

const float* Primitive::getTexCoords() const
{
   return &vTexCoords[0];
}

const ushort* Primitive::getIndices() const
{
   return &vIndices[0];
}


//
//  Quad
//
Quad::Quad(float Size)
{
   vVertices.resize(4 * 3);
   vNormals.resize(4 * 3);
   vTexCoords.resize(4 * 2);

   GESTLVector(float)::iterator v = vVertices.begin();
   GESTLVector(float)::iterator n = vNormals.begin();
   GESTLVector(float)::iterator t = vTexCoords.begin();

   float fHalfSize = Size * 0.5f;

   *v++ = -fHalfSize; *v++ = 0.0f; *v++ = -fHalfSize;
   *v++ = -fHalfSize; *v++ = 0.0f; *v++ =  fHalfSize;
   *v++ =  fHalfSize; *v++ = 0.0f; *v++ =  fHalfSize;
   *v++ =  fHalfSize; *v++ = 0.0f; *v++ = -fHalfSize;

   *n++ = 0.0f; *n++ = 1.0f; *n++ = 0.0f;
   *n++ = 0.0f; *n++ = 1.0f; *n++ = 0.0f;
   *n++ = 0.0f; *n++ = 1.0f; *n++ = 0.0f;
   *n++ = 0.0f; *n++ = 1.0f; *n++ = 0.0f;

   *t++ = 0.0f; *t++ = 0.0f;
   *t++ = 0.0f; *t++ = 1.0f;
   *t++ = 1.0f; *t++ = 1.0f;
   *t++ = 1.0f; *t++ = 0.0f;

   vIndices.resize(6);
   GESTLVector(ushort)::iterator i = vIndices.begin();

   *i++ = 0; *i++ = 1; *i++ = 2;
   *i++ = 0; *i++ = 2; *i++ = 3;
}



//
//  Cube
//
Cube::Cube(float Size)
{
   vVertices.resize(24 * 3);
   vNormals.resize(24 * 3);
   vTexCoords.resize(24 * 2);
   vIndices.resize(24);

   const int FaceCount = 6;

   const Vector3 Normals[FaceCount] =
   {
      Vector3( 0.0f,  0.0f, -1.0f),
      Vector3( 0.0f,  0.0f,  1.0f),
      Vector3( 1.0f,  0.0f,  0.0f),
      Vector3(-1.0f,  0.0f,  0.0f),
      Vector3( 0.0f,  1.0f,  0.0f),
      Vector3( 0.0f, -1.0f,  0.0f)
   };

   const Vector2 TextureCoordinates[4] =
   {
      Vector2(1.0f, 0.0f),
      Vector2(1.0f, 1.0f),
      Vector2(0.0f, 1.0f),
      Vector2(0.0f, 0.0f)
   };

   for(int i = 0; i < FaceCount; i++)
   {
      ushort iFirstIndex = (ushort)(vVertices.size() / 3);

      vIndices.push_back(iFirstIndex + 0);
      vIndices.push_back(iFirstIndex + 1);
      vIndices.push_back(iFirstIndex + 2);
      vIndices.push_back(iFirstIndex + 0);
      vIndices.push_back(iFirstIndex + 2);
      vIndices.push_back(iFirstIndex + 3);

      const Vector3& vNormal = Normals[i];

      Vector3 vBasis = (i >= 4) ? Vector3::UnitZ : Vector3::UnitY;
      Vector3 vSide1 = vNormal.crossProduct(vBasis);
      Vector3 vSide2 = vNormal.crossProduct(vSide1);

      float fHalfSize = Size * 0.5f;

      Vector3 v0 = (vNormal - vSide1 + vSide2) * fHalfSize;
      Vector3 v1 = (vNormal - vSide1 - vSide2) * fHalfSize;
      Vector3 v2 = (vNormal + vSide1 - vSide2) * fHalfSize;
      Vector3 v3 = (vNormal + vSide1 + vSide2) * fHalfSize;

      vVertices.push_back(v0.X); vVertices.push_back(v0.Y); vVertices.push_back(v0.Z);
      vVertices.push_back(v1.X); vVertices.push_back(v1.Y); vVertices.push_back(v1.Z);
      vVertices.push_back(v2.X); vVertices.push_back(v2.Y); vVertices.push_back(v2.Z);
      vVertices.push_back(v3.X); vVertices.push_back(v3.Y); vVertices.push_back(v3.Z);

      vNormals.push_back(vNormal.X); vNormals.push_back(vNormal.Y); vNormals.push_back(vNormal.Z);
      vNormals.push_back(vNormal.X); vNormals.push_back(vNormal.Y); vNormals.push_back(vNormal.Z);
      vNormals.push_back(vNormal.X); vNormals.push_back(vNormal.Y); vNormals.push_back(vNormal.Z);
      vNormals.push_back(vNormal.X); vNormals.push_back(vNormal.Y); vNormals.push_back(vNormal.Z);

      vTexCoords.push_back(TextureCoordinates[0].X); vTexCoords.push_back(TextureCoordinates[0].Y);
      vTexCoords.push_back(TextureCoordinates[1].X); vTexCoords.push_back(TextureCoordinates[1].Y);
      vTexCoords.push_back(TextureCoordinates[2].X); vTexCoords.push_back(TextureCoordinates[2].Y);
      vTexCoords.push_back(TextureCoordinates[3].X); vTexCoords.push_back(TextureCoordinates[3].Y);
   }
}



//
//  Sphere
//
Sphere::Sphere(float Radius, uint Rings, uint Sectors)
{
   float const R = 1.0f / (Rings - 1);
   float const S = 1.0f / (Sectors - 1);

   vVertices.resize(Rings * Sectors * 3);
   vNormals.resize(Rings * Sectors * 3);
   vTexCoords.resize(Rings * Sectors * 2);

   GESTLVector(float)::iterator v = vVertices.begin();
   GESTLVector(float)::iterator n = vNormals.begin();
   GESTLVector(float)::iterator t = vTexCoords.begin();

   for(uint r = 0; r < Rings; r++)
   {
      for(uint s = 0; s < Sectors; s++)
      {
         float fSin = sinf(GE_PI * r * R);
         Vector3 vPoint(cosf(GE_DOUBLEPI * s * S) * fSin, sinf(-GE_HALFPI + GE_PI * r * R), sinf(GE_DOUBLEPI * s * S) * -fSin);

         *t++ = s * S;
         *t++ = r * R;

         *v++ = vPoint.X * Radius;
         *v++ = vPoint.Y * Radius;
         *v++ = vPoint.Z * Radius;

         *n++ = vPoint.X;
         *n++ = vPoint.Y;
         *n++ = vPoint.Z;
      }
   }

   vIndices.resize(Rings * Sectors * 6);
   GESTLVector(ushort)::iterator i = vIndices.begin();

   for(uint r = 0; r < (Rings - 1); r++)
   {
      for(uint s = 0; s < (Sectors - 1); s++)
      {
         *i++ = r * Sectors + s;
         *i++ = r * Sectors + (s + 1);
         *i++ = (r + 1) * Sectors + s;

         *i++ = r * Sectors + (s + 1);
         *i++ = (r + 1) * Sectors + (s + 1);
         *i++ = (r + 1) * Sectors + s;
      }
   }
}
