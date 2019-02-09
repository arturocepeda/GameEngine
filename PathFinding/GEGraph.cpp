
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

Graph::Graph(uint32_t numberOfNodes)
   : NumberOfNodes(numberOfNodes)
{
   NumberOfNodes = numberOfNodes;
   Nodes = new GraphNode[numberOfNodes];
   AdjacencyList = new std::vector<GraphConnection>[numberOfNodes];
   unreachableNodes = new bool[numberOfNodes];
   memset(unreachableNodes, 0, numberOfNodes);
}

Graph::~Graph()
{
   delete[] Nodes;
   delete[] AdjacencyList;
   delete[] unreachableNodes;
}

void Graph::setReachableNode(GraphNodeIndex nodeIndex)
{
   unreachableNodes[nodeIndex] = false;
}

void Graph::setUnreachableNode(GraphNodeIndex nodeIndex)
{
   unreachableNodes[nodeIndex] = true;
}

bool Graph::isUnreachableNode(GraphNodeIndex nodeIndex) const
{
   return unreachableNodes[nodeIndex];
}

void Graph::connect(GraphNodeIndex nodeA, GraphNodeIndex nodeB, float weight, bool bidirectional)
{
   if(unreachableNodes[nodeB])
      return;

   if(!alreadyConnected(nodeA, nodeB))
   {
      GraphConnection connection(nodeB, weight);
      AdjacencyList[nodeA].push_back(connection);
   }

   if(bidirectional && !alreadyConnected(nodeB, nodeA))
   {
      GraphConnection connection(nodeA, weight);
      AdjacencyList[nodeB].push_back(connection);
   }
}

bool Graph::alreadyConnected(GraphNodeIndex nodeA, GraphNodeIndex nodeB) const
{
   for(uint32_t i = 0u; i < AdjacencyList[nodeA].size(); i++)
   {
      if(AdjacencyList[nodeA][i].DestinyNode == nodeB)
         return true;
   }

   return false;
}

void Graph::setConnectionActive(GraphNodeIndex nodeA, GraphNodeIndex nodeB, bool active)
{
   GraphConnection* connection = getConnection(nodeA, nodeB);

   if(connection)
   {
      connection->Active = active;
   }
}

GraphConnection* Graph::getConnection(GraphNodeIndex nodeA, GraphNodeIndex nodeB)
{
   for(uint32_t i = 0u; i < AdjacencyList[nodeA].size(); i++)
   {
      if(AdjacencyList[nodeA][i].DestinyNode == nodeB)
         return &AdjacencyList[nodeA][i];
   }

   return nullptr;
}

void Graph::setConnectionWeight(GraphNodeIndex nodeA, GraphNodeIndex nodeB, float weight)
{
   GraphConnection* connection = getConnection(nodeA, nodeB);

   if(connection)
   {
      connection->Weight = weight;
   }
}

void Graph::activateConnection(GraphNodeIndex nodeA, GraphNodeIndex nodeB)
{
   setConnectionActive(nodeA, nodeB, true);
   setConnectionActive(nodeB, nodeA, true);
}

void Graph::deactivateConnection(GraphNodeIndex nodeA, GraphNodeIndex nodeB)
{
   setConnectionActive(nodeA, nodeB, false);
   setConnectionActive(nodeB, nodeA, false);
}
