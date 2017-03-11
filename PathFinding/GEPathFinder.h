
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEPathFinder.h ---
//
////////////////////////////////////////////////////////////////////////

#pragma once

#include "GEGraph.h"
#include "GEGraphSearch.h"
#include <vector>

namespace GE { namespace Pathfinding
{
   class PathFinder
   {
   private:
      Graph* graph;
      GraphSearch* algorithm;

      int currentStartNode;
      int currentTargetNode;
      std::vector<int> currentPath;

      int getPositionOfNode(int node);

   public:
      PathFinder(Graph* graph, GraphSearch* algorithm);
      ~PathFinder();

      std::vector<int>& getCurrentPath();

      bool calculatePath(int nodeStart, int nodeTarget);
      bool updatePath(int currentNode, int updatedNode);
   };
}}
