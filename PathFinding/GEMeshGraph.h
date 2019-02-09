
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
      uint32_t nodesColumns;
      uint32_t nodesRows;
    
      void setConnections(GraphNodeIndex nodeIndex, uint32_t column, uint32_t row);

   public:
      MeshGraph(uint32_t nodesColumns, uint32_t nodesRows);
      ~MeshGraph();

      uint32_t getNumberOfColumns() const;
      uint32_t getNumberOfRows() const;

      void setConnections();
      void resetConnections();

      void setNodePositions(float firstX, float firstY, float incrementX, float incrementY);
   };
}}
