
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

      GraphConnection(GraphNodeIndex pDestinyNode, float pWeight) 
         : DestinyNode(pDestinyNode)
         , Weight(pWeight) 
         , Active(true)
      {
      }
   };

   class Graph
   {
   protected:
      bool* mUnreachableNodes;

      void setConnectionActive(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB, bool pActive);
      GraphConnection* getConnection(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB);

   public:
      GraphNode* Nodes;
      std::vector<GraphConnection>* AdjacencyList;
      uint32_t NumberOfNodes;

      Graph(uint32_t pNumberOfNodes);
      ~Graph();
    
      void setReachableNode(GraphNodeIndex pNodeIndex);
      void setUnreachableNode(GraphNodeIndex pNodeIndex);
      bool isUnreachableNode(GraphNodeIndex pNodeIndex) const;

      void connect(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB, float pWeight, bool pBidirectional = true);
      bool alreadyConnected(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB) const;

      void setConnectionWeight(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB, float pWeight);
      void activateConnection(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB);
      void deactivateConnection(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB);
   };
}}
