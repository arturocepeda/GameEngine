
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

Graph::Graph(int numberOfNodes)
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

void Graph::setReachableNode(int nodeIndex)
{
   unreachableNodes[nodeIndex] = false;
}

void Graph::setUnreachableNode(int nodeIndex)
{
   unreachableNodes[nodeIndex] = true;
}

bool Graph::isUnreachableNode(int nodeIndex)
{
   return unreachableNodes[nodeIndex];
}

void Graph::connect(int nodeA, int nodeB, int weight, bool bidirectional)
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

bool Graph::alreadyConnected(int nodeA, int nodeB)
{
   for(unsigned int i = 0; i < AdjacencyList[nodeA].size(); i++)
   {
      if(AdjacencyList[nodeA][i].DestinyNode == nodeB)
         return true;
   }

   return false;
}

void Graph::setConnectionActive(int nodeA, int nodeB, bool active)
{
   GraphConnection* connection = getConnection(nodeA, nodeB);
   if(connection)
      connection->Active = active;
}

GraphConnection* Graph::getConnection(int nodeA, int nodeB)
{
   for(unsigned int i = 0; i < AdjacencyList[nodeA].size(); i++)
   {
      if(AdjacencyList[nodeA][i].DestinyNode == nodeB)
         return &AdjacencyList[nodeA][i];
   }

   return 0;
}

void Graph::setConnectionWeight(int nodeA, int nodeB, int weight)
{
   GraphConnection* connection = getConnection(nodeA, nodeB);

   if(connection)
      connection->Weight = weight;
}

void Graph::activateConnection(int nodeA, int nodeB)
{
   setConnectionActive(nodeA, nodeB, true);
   setConnectionActive(nodeB, nodeA, true);
}

void Graph::deactivateConnection(int nodeA, int nodeB)
{
   setConnectionActive(nodeA, nodeB, false);
   setConnectionActive(nodeB, nodeA, false);
}
