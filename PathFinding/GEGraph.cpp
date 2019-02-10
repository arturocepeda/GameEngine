
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEGraph.cpp ---
//
////////////////////////////////////////////////////////////////////////

#include "GEGraph.h"

using namespace GE::Pathfinding;

Graph::Graph(uint32_t pNumberOfNodes)
   : NumberOfNodes(pNumberOfNodes)
{
   NumberOfNodes = pNumberOfNodes;
   Nodes = new GraphNode[pNumberOfNodes];
   AdjacencyList = new std::vector<GraphConnection>[pNumberOfNodes];
   mUnreachableNodes = new bool[pNumberOfNodes];
   memset(mUnreachableNodes, 0, pNumberOfNodes);
}

Graph::~Graph()
{
   delete[] Nodes;
   delete[] AdjacencyList;
   delete[] mUnreachableNodes;
}

void Graph::setReachableNode(GraphNodeIndex pNodeIndex)
{
   mUnreachableNodes[pNodeIndex] = false;
}

void Graph::setUnreachableNode(GraphNodeIndex pNodeIndex)
{
   mUnreachableNodes[pNodeIndex] = true;
}

bool Graph::isUnreachableNode(GraphNodeIndex pNodeIndex) const
{
   return mUnreachableNodes[pNodeIndex];
}

void Graph::connect(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB, float pWeight, bool pBidirectional)
{
   if(mUnreachableNodes[pNodeB])
      return;

   if(!alreadyConnected(pNodeA, pNodeB))
   {
      GraphConnection connection(pNodeB, pWeight);
      AdjacencyList[pNodeA].push_back(connection);
   }

   if(pBidirectional && !alreadyConnected(pNodeB, pNodeA))
   {
      GraphConnection connection(pNodeA, pWeight);
      AdjacencyList[pNodeB].push_back(connection);
   }
}

bool Graph::alreadyConnected(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB) const
{
   for(size_t i = 0u; i < AdjacencyList[pNodeA].size(); i++)
   {
      if(AdjacencyList[pNodeA][i].DestinyNode == pNodeB)
      {
         return true;
      }
   }

   return false;
}

void Graph::setConnectionActive(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB, bool pActive)
{
   GraphConnection* connection = getConnection(pNodeA, pNodeB);

   if(connection)
   {
      connection->Active = pActive;
   }
}

GraphConnection* Graph::getConnection(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB)
{
   for(size_t i = 0u; i < AdjacencyList[pNodeA].size(); i++)
   {
      if(AdjacencyList[pNodeA][i].DestinyNode == pNodeB)
      {
         return &AdjacencyList[pNodeA][i];
      }
   }

   return nullptr;
}

void Graph::setConnectionWeight(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB, float pWeight)
{
   GraphConnection* connection = getConnection(pNodeA, pNodeB);

   if(connection)
   {
      connection->Weight = pWeight;
   }
}

void Graph::activateConnection(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB)
{
   setConnectionActive(pNodeA, pNodeB, true);
   setConnectionActive(pNodeB, pNodeA, true);
}

void Graph::deactivateConnection(GraphNodeIndex pNodeA, GraphNodeIndex pNodeB)
{
   setConnectionActive(pNodeA, pNodeB, false);
   setConnectionActive(pNodeB, pNodeA, false);
}
