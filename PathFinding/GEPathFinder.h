
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
      Graph* mGraph;
      GraphSearch* mAlgorithm;

      GraphNodeIndex mCurrentStartNode;
      GraphNodeIndex mCurrentTargetNode;
      std::vector<GraphNodeIndex> mCurrentPath;

      int getPositionOfNode(GraphNodeIndex node) const;

   public:
      PathFinder(Graph* pGraph, GraphSearch* pAlgorithm);
      ~PathFinder();

      const std::vector<GraphNodeIndex>& getCurrentPath() const;

      bool calculatePath(GraphNodeIndex pNodeStart, GraphNodeIndex pNodeTarget);
      bool updatePath(GraphNodeIndex pCurrentNode, GraphNodeIndex pUpdatedNode);
   };
}}
