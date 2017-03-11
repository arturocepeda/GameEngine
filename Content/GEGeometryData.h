
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEGeometryData.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypeDefinitions.h"

namespace GE { namespace Content
{
   struct GeometryData
   {
      uint NumVertices;
      float* VertexData;
      int VertexStride;
      uint NumIndices;
      ushort* Indices;

      GeometryData()
         : NumVertices(0)
         , VertexData(0)
         , VertexStride(0)
         , NumIndices(0)
         , Indices(0)
      {
      }
   };
}}
