
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
      uint32_t mNodesColumns;
      uint32_t mNodesRows;
    
      void setConnections(GraphNodeIndex pNodeIndex, uint32_t pColumn, uint32_t pRow);

   public:
      MeshGraph(uint32_t pNodesColumns, uint32_t pNodesRows);
      ~MeshGraph();

      uint32_t getNumberOfColumns() const;
      uint32_t getNumberOfRows() const;

      void setConnections();
      void resetConnections();

      void setNodePositions(float pFirstX, float pFirstY, float pIncrementX, float pIncrementY);
   };
}}
