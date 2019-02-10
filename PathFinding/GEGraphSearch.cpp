
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
   : mGraph(nullptr)
   , mStartNode(InvalidNodeIndex)
   , mTargetNode(InvalidNodeIndex)
   , mCostSoFar(nullptr)
   , mPreviousNode(nullptr)
   , mVisitedNodes(0u)
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

    mCostSoFar = new float[mGraph->NumberOfNodes];
    mPreviousNode = new GraphNodeIndex[mGraph->NumberOfNodes];
    mNodesToCheck.clear();
    mVisitedNodes = 0u;

    if(mShortestPath.capacity() < mGraph->NumberOfNodes)
    {
        mShortestPath.reserve(mGraph->NumberOfNodes);
    }

    for(uint32_t i = 0; i < mGraph->NumberOfNodes; i++)
    {
        mCostSoFar[i] = FLT_MAX;
        mPreviousNode[i] = InvalidNodeIndex;
        mNodesToCheck.push_back(i);
    }

    mCostSoFar[mStartNode] = 0.0f;
}

void GraphSearch::release()
{
    if(mCostSoFar)
    {
        delete[] mCostSoFar;
        mCostSoFar = nullptr;
    }

    if(mPreviousNode)
    {
        delete[] mPreviousNode;
        mPreviousNode = nullptr;
    }
}

GraphNodeIndex GraphSearch::getNextNode()
{
    GraphNodeIndex nextNode = InvalidNodeIndex;
    GraphNodeIndex nextNodeIndexInTheList = InvalidNodeIndex;

    float minCost = FLT_MAX;

    for(uint32_t i = 0; i < mNodesToCheck.size(); i++)
    {
        const GraphNodeIndex currentNode = mNodesToCheck[i];
        const float cost = mCostSoFar[currentNode];

        if(cost < minCost)
        {
            minCost = mCostSoFar[currentNode];
            nextNode = currentNode;
            nextNodeIndexInTheList = i;
        }
    }

    if(nextNode >= 0)
    {
        mNodesToCheck.erase(mNodesToCheck.begin() + nextNodeIndexInTheList);
    }

    return nextNode;
}

bool GraphSearch::search(Graph* pGraph, GraphNodeIndex pStartNode, GraphNodeIndex pTargetNode)
{
    mGraph = pGraph;
    mStartNode = pStartNode;
    mTargetNode = pTargetNode;

    initialize();

    while(!mNodesToCheck.empty())
    {
        GraphNodeIndex currentNode = getNextNode();
        mVisitedNodes++;

        // there's no path
        if(currentNode == InvalidNodeIndex)
            return false;

        // path found
        if(currentNode == pTargetNode)
            return true;

        for(uint32_t connection = 0; connection < mGraph->AdjacencyList[currentNode].size(); connection++)
        {
            GraphConnection& currentNodeConnection = mGraph->AdjacencyList[currentNode][connection];

            if(!currentNodeConnection.Active || mGraph->isUnreachableNode(currentNodeConnection.DestinyNode))
                continue;

            const GraphNodeIndex connectedNode = currentNodeConnection.DestinyNode;

            const float costToThisNode =
               mCostSoFar[currentNode] +
               currentNodeConnection.Weight +
               (mEstimateDistance(mGraph->Nodes[connectedNode], mGraph->Nodes[mTargetNode]) * mEstimateDistanceWeigth);

            if(mCostSoFar[connectedNode] > costToThisNode)
            {
                mCostSoFar[connectedNode] = costToThisNode;
                mPreviousNode[connectedNode] = currentNode;
            }
        }
    }

    return false;
}

void GraphSearch::getPath(std::vector<GraphNodeIndex>* pOutPath) const
{
    GraphNodeIndex currentNode = mTargetNode;
    pOutPath->clear();

    while(currentNode != mStartNode)
    {
        pOutPath->insert(pOutPath->begin(), currentNode);
        currentNode = mPreviousNode[currentNode];
    }
}

uint32_t GraphSearch::getNumberOfVisitedNodes() const
{
    return mVisitedNodes;
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
