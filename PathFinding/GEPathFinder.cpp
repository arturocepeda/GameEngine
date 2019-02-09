
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

PathFinder::PathFinder(Graph* graph, GraphSearch* algorithm)
   : graph(graph)
   , algorithm(algorithm)
{
}

PathFinder::~PathFinder()
{
}

const std::vector<GraphNodeIndex>& PathFinder::getCurrentPath() const
{
   return currentPath;
}

bool PathFinder::calculatePath(GraphNodeIndex nodeStart, GraphNodeIndex nodeTarget)
{
   currentStartNode = nodeStart;
   currentTargetNode = nodeTarget;

   bool found = algorithm->search(graph, nodeStart, nodeTarget);

   if(found)
   {
      algorithm->getPath(&currentPath);
   }
   else
   {
      currentPath.clear();
   }

   return found;
}

bool PathFinder::updatePath(GraphNodeIndex currentNode, GraphNodeIndex updatedNode)
{
   int updatedNodeIndex = getPositionOfNode(updatedNode);
   int currentIndex = getPositionOfNode(currentNode);

   // the updated node was closed and is already behind us
   if(updatedNodeIndex < currentIndex && graph->isUnreachableNode(updatedNode))
      return false;

   // remove path from our current position
   int newStartNode = InvalidNodeIndex;

   // we are at the beginning
   if(currentIndex == -1)
   {
      newStartNode = currentStartNode;
      currentPath.clear();
   }
   // we are somewhere in the middle
   else
   {
      newStartNode = currentPath[currentIndex];

      while(currentPath[currentPath.size() - 1u] != newStartNode)
      {
         currentPath.pop_back();
      }
   }

   bool found = algorithm->search(graph, newStartNode, currentTargetNode);

   if(found)
   {
      algorithm->getPath(&currentPath);
   }

   return true;
}

int PathFinder::getPositionOfNode(GraphNodeIndex node) const
{
   for(size_t i = 0; i < currentPath.size(); i++)
   {
      if(currentPath[i] == node)
         return (int)i;
   }

   return -1;
}
