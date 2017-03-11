
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Pathfinding
//
//  --- GEDijkstra.h ---
//
////////////////////////////////////////////////////////////////////////

#pragma once

#include "GEGraphSearch.h"

namespace GE { namespace Pathfinding
{
   class Dijkstra : public GraphSearch
   {
   protected:
      virtual int estimateDistance(int nodeFrom, int nodeTo);
   };
}}
