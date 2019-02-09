
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEGraphSearch.cpp ---
//
////////////////////////////////////////////////////////////////////////

#include "GEGraphSearch.h"

#include <vector>

using namespace GE::Pathfinding;

//
//  GraphSearch
//
GraphSearch::GraphSearch()
   : costSoFar(nullptr)
   , previousNode(nullptr)
   , visitedNodes(0u)
   , mEstimateDistance(nullptr)
   , mEstimateDistanceWeigth(0.0f)
{
}

GraphSearch::~GraphSearch()
{
    release();
}

void GraphSearch::initialize()
{
    release();

    costSoFar = new float[graph->NumberOfNodes];
    previousNode = new GraphNodeIndex[graph->NumberOfNodes];
    nodesToCheck.clear();
    visitedNodes = 0u;

    if(shortestPath.capacity() < graph->NumberOfNodes)
        shortestPath.reserve(graph->NumberOfNodes);

    for(uint32_t i = 0; i < graph->NumberOfNodes; i++)
    {
        costSoFar[i] = FLT_MAX;
        previousNode[i] = InvalidNodeIndex;
        nodesToCheck.push_back(i);
    }

    costSoFar[startNode] = 0.0f;
}

void GraphSearch::release()
{
    if(costSoFar)
    {
        delete[] costSoFar;
        costSoFar = nullptr;
    }

    if(previousNode)
    {
        delete[] previousNode;
        previousNode = nullptr;
    }
}

GraphNodeIndex GraphSearch::getNextNode()
{
    GraphNodeIndex nextNode = InvalidNodeIndex;
    GraphNodeIndex nextNodeIndexInTheList = InvalidNodeIndex;

    float minCost = FLT_MAX;

    for(uint32_t i = 0; i < nodesToCheck.size(); i++)
    {
        GraphNodeIndex currentNode = nodesToCheck[i];
        float cost = costSoFar[currentNode];

        if(cost < minCost)
        {
            minCost = costSoFar[currentNode];
            nextNode = currentNode;
            nextNodeIndexInTheList = i;
        }
    }

    if(nextNode >= 0)
    {
        nodesToCheck.erase(nodesToCheck.begin() + nextNodeIndexInTheList);
    }

    return nextNode;
}

bool GraphSearch::search(Graph* graph, GraphNodeIndex startNode, GraphNodeIndex targetNode)
{
    this->graph = graph;
    this->startNode = startNode;
    this->targetNode = targetNode;

    initialize();

    while(!nodesToCheck.empty())
    {
        GraphNodeIndex currentNode = getNextNode();
        visitedNodes++;

        // there's no path
        if(currentNode == InvalidNodeIndex)
            return false;

        // path found
        if(currentNode == targetNode)
            return true;

        for(uint32_t connection = 0; connection < graph->AdjacencyList[currentNode].size(); connection++)
        {
            GraphConnection& currentNodeConnection = graph->AdjacencyList[currentNode][connection];

            if(!currentNodeConnection.Active || graph->isUnreachableNode(currentNodeConnection.DestinyNode))
                continue;

            const GraphNodeIndex connectedNode = currentNodeConnection.DestinyNode;

            const float costToThisNode =
               costSoFar[currentNode] +
               currentNodeConnection.Weight +
               (mEstimateDistance(graph->Nodes[connectedNode], graph->Nodes[targetNode]) * mEstimateDistanceWeigth);

            if(costSoFar[connectedNode] > costToThisNode)
            {
                costSoFar[connectedNode] = costToThisNode;
                previousNode[connectedNode] = currentNode;
            }
        }
    }

    return false;
}

void GraphSearch::getPath(std::vector<GraphNodeIndex>* pOutPath) const
{
    GraphNodeIndex currentNode = targetNode;
    pOutPath->clear();

    while(currentNode != startNode)
    {
        pOutPath->insert(pOutPath->begin(), currentNode);
        currentNode = previousNode[currentNode];
    }
}

uint32_t GraphSearch::getNumberOfVisitedNodes() const
{
    return visitedNodes;
}


//
//  Dijkstra
//
Dijkstra::Dijkstra()
{
   mEstimateDistance = estimateDistance;
}

Dijkstra::~Dijkstra()
{
}

float Dijkstra::estimateDistance(const GraphNode& pNodeFrom, const GraphNode& pNodeTo)
{
   return 0.0f;
}


//
//  AStar
//
AStar::AStar(float pHeuristicWeight)
{
   mEstimateDistance = estimateDistance;
   mEstimateDistanceWeigth = pHeuristicWeight;
}

AStar::~AStar()
{
}

float AStar::estimateDistance(const GraphNode& pNodeFrom, const GraphNode& pNodeTo)
{
   return fabsf(pNodeTo.PosX - pNodeFrom.PosX) + fabsf(pNodeTo.PosY - pNodeFrom.PosY);
}
