
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEAStar.cpp ---
//
////////////////////////////////////////////////////////////////////////

#include "GEAStar.h"

using namespace GE::Pathfinding;

AStar::AStar(float HeuristicWeight)
   : fHeuristicWeight(HeuristicWeight)
{
}

int AStar::estimateDistance(int nodeFrom, int nodeTo)
{
   return (int)((fabs(graph->Nodes[nodeTo].PosX - graph->Nodes[nodeFrom].PosX) +
      fabs(graph->Nodes[nodeTo].PosY - graph->Nodes[nodeFrom].PosY)) * fHeuristicWeight);
}
