
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEMeshGraph.h ---
//
////////////////////////////////////////////////////////////////////////

#pragma once

#include "GEGraph.h"

namespace GE { namespace Pathfinding
{
   class MeshGraph : public Graph
   {
   protected:
      int nodesColumns;
      int nodesRows;
    
      void setConnections(int nodeIndex, int column, int row);

   public:
      MeshGraph(int nodesColumns, int nodesRows);
      ~MeshGraph();

      int getNumberOfColumns();
      int getNumberOfRows();

      void setConnections();
      void resetConnections();

      void setNodePositions(float firstX, float firstY, float incrementX, float incrementY);
   };
}}
