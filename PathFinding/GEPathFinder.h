
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

      GraphNodeIndex currentStartNode;
      GraphNodeIndex currentTargetNode;
      std::vector<GraphNodeIndex> currentPath;

      int getPositionOfNode(GraphNodeIndex node) const;

   public:
      PathFinder(Graph* graph, GraphSearch* algorithm);
      ~PathFinder();

      const std::vector<GraphNodeIndex>& getCurrentPath() const;

      bool calculatePath(GraphNodeIndex nodeStart, GraphNodeIndex nodeTarget);
      bool updatePath(GraphNodeIndex currentNode, GraphNodeIndex updatedNode);
   };
}}
