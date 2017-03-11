
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
#include <climits>

using namespace GE::Pathfinding;

GraphSearch::GraphSearch()
{
    Infinity = std::numeric_limits<int>::max();
    InvalidNodeIndex = -1;
    costSoFar = NULL;
    previousNode = NULL;
}

GraphSearch::~GraphSearch()
{
    release();
}

void GraphSearch::initialize()
{
    release();
    costSoFar = new int[graph->NumberOfNodes];
    previousNode = new int[graph->NumberOfNodes];
    nodesToCheck.clear();
    visitedNodes = 0;

    if(shortestPath.capacity() < graph->NumberOfNodes)
        shortestPath.reserve(graph->NumberOfNodes);

    for(unsigned int i = 0; i < graph->NumberOfNodes; i++)
    {
        costSoFar[i] = Infinity;
        previousNode[i] = InvalidNodeIndex;
        nodesToCheck.push_back(i);
    }

    costSoFar[startNode] = 0;
}

void GraphSearch::release()
{
    if(costSoFar != NULL)
    {
        delete[] costSoFar;
        costSoFar = NULL;
    }

    if(previousNode != NULL)
    {
        delete[] previousNode;
        previousNode = NULL;
    }
}

int GraphSearch::getNextNode()
{
    int minCost = Infinity;
    int nextNode = InvalidNodeIndex;
    int nextNodeIndexInTheList;
    int currentNode;
    int cost;

    for(unsigned int i = 0; i < nodesToCheck.size(); i++)
    {
        currentNode = nodesToCheck[i];
        cost = costSoFar[currentNode];

        if(cost < minCost)
        {
            minCost = costSoFar[currentNode];
            nextNode = currentNode;
            nextNodeIndexInTheList = i;
        }
    }

    if(nextNode >= 0)
        nodesToCheck.erase(nodesToCheck.begin() + nextNodeIndexInTheList);

    return nextNode;
}

bool GraphSearch::search(Graph* graph, int startNode, int targetNode)
{
    this->graph = graph;
    this->startNode = startNode;
    this->targetNode = targetNode;

    int currentNode;

    initialize();

    while(nodesToCheck.size() > 0)
    {
        currentNode = getNextNode();
        visitedNodes++;

        // there's no path
        if(currentNode == InvalidNodeIndex)
            return false;

        // path found
        if(currentNode == targetNode)
            return true;

        for(unsigned int connection = 0; connection < graph->AdjacencyList[currentNode].size(); connection++)
        {
            GraphConnection& currentNodeConnection = graph->AdjacencyList[currentNode][connection];

            if (!currentNodeConnection.Active || graph->isUnreachableNode(currentNodeConnection.DestinyNode))
                continue;

            int connectedNode = currentNodeConnection.DestinyNode;
            int costToThisNode = costSoFar[currentNode] + currentNodeConnection.Weight +
                estimateDistance(connectedNode, targetNode);

            if(costSoFar[connectedNode] > costToThisNode)
            {
                costSoFar[connectedNode] = costToThisNode;
                previousNode[connectedNode] = currentNode;
            }
        }
    }

    return false;
}

std::vector<int> GraphSearch::getPath()
{
    int currentNode = targetNode;
    shortestPath.clear();

    while(currentNode != startNode)
    {
        shortestPath.insert(shortestPath.begin(), currentNode);
        currentNode = previousNode[currentNode];
    }

    return shortestPath;
}

int GraphSearch::getNumberOfVisitedNodes()
{
    return visitedNodes;
}
