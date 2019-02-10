
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEPathFinder.cpp ---
//
////////////////////////////////////////////////////////////////////////

#include "GEPathFinder.h"

using namespace GE::Pathfinding;

PathFinder::PathFinder(Graph* pGraph, GraphSearch* pAlgorithm)
   : mGraph(pGraph)
   , mAlgorithm(pAlgorithm)
   , mCurrentStartNode(InvalidNodeIndex)
   , mCurrentTargetNode(InvalidNodeIndex)
{
}

PathFinder::~PathFinder()
{
}

const std::vector<GraphNodeIndex>& PathFinder::getCurrentPath() const
{
   return mCurrentPath;
}

bool PathFinder::calculatePath(GraphNodeIndex pNodeStart, GraphNodeIndex pNodeTarget)
{
   mCurrentStartNode = pNodeStart;
   mCurrentTargetNode = pNodeTarget;

   bool found = mAlgorithm->search(mGraph, pNodeStart, pNodeTarget);

   if(found)
   {
      mAlgorithm->getPath(&mCurrentPath);
   }
   else
   {
      mCurrentPath.clear();
   }

   return found;
}

bool PathFinder::updatePath(GraphNodeIndex pCurrentNode, GraphNodeIndex pUpdatedNode)
{
   int updatedNodeIndex = getPositionOfNode(pUpdatedNode);
   int currentIndex = getPositionOfNode(pCurrentNode);

   // the updated node was closed and is already behind us
   if(updatedNodeIndex < currentIndex && mGraph->isUnreachableNode(pUpdatedNode))
      return false;

   // remove path from our current position
   GraphNodeIndex newStartNode = InvalidNodeIndex;

   // we are at the beginning
   if(currentIndex == -1)
   {
      newStartNode = mCurrentStartNode;
      mCurrentPath.clear();
   }
   // we are somewhere in the middle
   else
   {
      newStartNode = mCurrentPath[currentIndex];

      while(mCurrentPath[mCurrentPath.size() - 1u] != newStartNode)
      {
         mCurrentPath.pop_back();
      }
   }

   const bool found = mAlgorithm->search(mGraph, newStartNode, mCurrentTargetNode);

   if(found)
   {
      mAlgorithm->getPath(&mCurrentPath);
   }

   return true;
}

int PathFinder::getPositionOfNode(GraphNodeIndex node) const
{
   for(size_t i = 0u; i < mCurrentPath.size(); i++)
   {
      if(mCurrentPath[i] == node)
      {
         return (int)i;
      }
   }

   return -1;
}
