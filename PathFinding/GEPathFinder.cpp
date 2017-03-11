
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

std::vector<int>& PathFinder::getCurrentPath()
{
   return currentPath;
}

bool PathFinder::calculatePath(int nodeStart, int nodeTarget)
{
   currentStartNode = nodeStart;
   currentTargetNode = nodeTarget;

   bool found = algorithm->search(graph, nodeStart, nodeTarget);

   if(found)
      currentPath = algorithm->getPath();
   else
      currentPath.clear();

   return found;
}

bool PathFinder::updatePath(int currentNode, int updatedNode)
{
   int updatedNodeIndex = getPositionOfNode(updatedNode);
   int currentIndex = getPositionOfNode(currentNode);

   // the updated node was closed and is already behind us
   if(updatedNodeIndex < currentIndex && graph->isUnreachableNode(updatedNode))
      return false;

   // remove path from our current position
   int newStartNode;

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

      while(currentPath[currentPath.size() - 1] != newStartNode)
         currentPath.pop_back();
   }

   bool found = algorithm->search(graph, newStartNode, currentTargetNode);

   if(!found)
      return true;

   std::vector<int> newSubPath = algorithm->getPath();

   for(unsigned int i = 0; i < newSubPath.size(); i++)
      currentPath.push_back(newSubPath[i]);

   return true;
}

int PathFinder::getPositionOfNode(int node)
{
   for(unsigned int i = 0; i < currentPath.size(); i++)
   {
      if(currentPath[i] == node)
         return i;
   }

   return -1;
}
