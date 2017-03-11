
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEGraphSearch.h ---
//
////////////////////////////////////////////////////////////////////////

#pragma once

#include "GEGraph.h"
#include <vector>

namespace GE { namespace Pathfinding
{
   class GraphSearch
   {
   protected:
      Graph* graph;
      int startNode;
      int targetNode;

      int* costSoFar;
      int* previousNode;
      std::vector<int> nodesToCheck;    
      std::vector<int> shortestPath;

      int Infinity;
      int InvalidNodeIndex;

      int visitedNodes;

      void initialize();
      void release();
      int getNextNode();

      virtual int estimateDistance(int nodeFrom, int nodeTo) = 0;

   public:
      GraphSearch();
      virtual ~GraphSearch();

      bool search(Graph* graph, int startNode, int targetNode);
      std::vector<int> getPath();
      int getNumberOfVisitedNodes();
   };
}}
