
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
      typedef float (*EstimateDistanceFunction)(const GraphNode& pNodeFrom, const GraphNode& pNodeTo);

      Graph* graph;
      GraphNodeIndex startNode;
      GraphNodeIndex targetNode;

      float* costSoFar;
      GraphNodeIndex* previousNode;
      std::vector<GraphNodeIndex> nodesToCheck;    
      std::vector<GraphNodeIndex> shortestPath;

      uint32_t visitedNodes;

      EstimateDistanceFunction mEstimateDistance;
      float mEstimateDistanceWeigth;

      GraphSearch();

      void initialize();
      void release();
      GraphNodeIndex getNextNode();

   public:
      ~GraphSearch();

      bool search(Graph* graph, GraphNodeIndex startNode, GraphNodeIndex targetNode);
      void getPath(std::vector<GraphNodeIndex>* pOutPath) const;
      uint32_t getNumberOfVisitedNodes() const;
   };


   class Dijkstra : public GraphSearch
   {
   private:
      static float estimateDistance(const GraphNode& pNodeFrom, const GraphNode& pNodeTo);

   public:
      Dijkstra();
      ~Dijkstra();
   };


   class AStar : public GraphSearch
   {
   private:
      static float estimateDistance(const GraphNode& pNodeFrom, const GraphNode& pNodeTo);

   public:
      AStar(float pHeuristicWeight);
      ~AStar();
   };
}}
