
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEGraph.h ---
//
////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include "Types/GETypes.h"

namespace GE { namespace Pathfinding
{
   typedef int GraphNodeIndex;

   static const GraphNodeIndex InvalidNodeIndex = -1;

   struct GraphNode
   {
      float PosX;
      float PosY;
   };

   struct GraphConnection
   {
      GraphNodeIndex DestinyNode;
      float Weight;
      bool Active;

      GraphConnection(GraphNodeIndex destinyNode, float weight) 
         : DestinyNode(destinyNode)
         , Weight(weight) 
         , Active(true)
      {
      }
   };

   class Graph
   {
   protected:
      bool* unreachableNodes;

      void setConnectionActive(GraphNodeIndex nodeA, GraphNodeIndex nodeB, bool active);
      GraphConnection* getConnection(GraphNodeIndex nodeA, GraphNodeIndex nodeB);

   public:
      GraphNode* Nodes;
      std::vector<GraphConnection>* AdjacencyList;
      uint32_t NumberOfNodes;

      Graph(uint32_t numberOfNodes);
      ~Graph();
    
      void setReachableNode(GraphNodeIndex nodeIndex);
      void setUnreachableNode(GraphNodeIndex nodeIndex);
      bool isUnreachableNode(GraphNodeIndex nodeIndex) const;

      void connect(GraphNodeIndex nodeA, GraphNodeIndex nodeB, float weight, bool bidirectional = true);
      bool alreadyConnected(GraphNodeIndex nodeA, GraphNodeIndex nodeB) const;

      void setConnectionWeight(GraphNodeIndex nodeA, GraphNodeIndex nodeB, float weight);
      void activateConnection(GraphNodeIndex nodeA, GraphNodeIndex nodeB);
      void deactivateConnection(GraphNodeIndex nodeA, GraphNodeIndex nodeB);
   };
}}
