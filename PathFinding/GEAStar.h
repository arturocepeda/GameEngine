
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEAStar.h ---
//
////////////////////////////////////////////////////////////////////////

#pragma once

#include "GEGraphSearch.h"
#include <cmath>

namespace GE { namespace Pathfinding
{
   class AStar : public GraphSearch
   {
   protected:
      float fHeuristicWeight;

      virtual int estimateDistance(int nodeFrom, int nodeTo);

   public:
      AStar(float HeuristicWeight);
   };
}}
