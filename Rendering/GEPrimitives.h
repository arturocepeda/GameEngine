
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEPrimitives.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GESTLTypes.h"

namespace GE { namespace Rendering
{
   class Primitive
   {
   protected:
      GESTLVector(float) vVertices;
      GESTLVector(float) vNormals;
      GESTLVector(float) vTexCoords;
      GESTLVector(ushort) vIndices;

      Primitive();

   public:
      const uint getNumVertices() const;
      const uint getNumIndices() const;

      const float* getVertices() const;
      const float* getNormals() const;
      const float* getTexCoords() const;
      const ushort* getIndices() const;
   };

   class Quad : public Primitive
   {
   public:
      Quad(float size);
   };

   class Cube : public Primitive
   {
   public:
      Cube(float size);
   };

   class Sphere : public Primitive
   {
   public:
      Sphere(float radius, uint rings, uint sectors);
   };
}}
