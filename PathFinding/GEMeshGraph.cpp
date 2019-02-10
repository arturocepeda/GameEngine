
////////////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Pathfinding
//
//  --- GEMeshGraph.cpp ---
//
////////////////////////////////////////////////////////////////////////

#include "GEMeshGraph.h"

using namespace GE::Pathfinding;

const float LineWeight = 1.0f;
const float DiagonalWeight = 1.41421356f;

MeshGraph::MeshGraph(uint32_t pNodesColumns, uint32_t pNodesRows)
   : Graph(pNodesColumns * pNodesRows)
{
   mNodesColumns = pNodesColumns;
   mNodesRows = pNodesRows;
}

MeshGraph::~MeshGraph()
{
}

uint32_t MeshGraph::getNumberOfColumns() const
{
   return mNodesColumns;
}

uint32_t MeshGraph::getNumberOfRows() const
{
   return mNodesRows;
}

void MeshGraph::setNodePositions(float pFirstX, float pFirstY, float pIncrementX, float pIncrementY)
{
   GraphNodeIndex nodeIndex = 0;

   float nodePositionY = pFirstY;

   for(uint32_t row = 0u; row < mNodesRows; row++)
   {
      float nodePositionX = pFirstX;

      for(uint32_t column = 0u; column < mNodesColumns; column++)
      {
         Nodes[nodeIndex].PosX = nodePositionX;
         Nodes[nodeIndex].PosY = nodePositionY;
         nodePositionX += pIncrementX;
         nodeIndex++;
      }

      nodePositionY += pIncrementY;
   }
}

void MeshGraph::setConnections()
{
   GraphNodeIndex nodeIndex = 0;

   for(uint32_t row = 0u; row < mNodesRows; row++)
   {
      for(uint32_t column = 0u; column < mNodesColumns; column++)
      {
         setConnections(nodeIndex, column, row);
         nodeIndex++;
      }
   }
}

void MeshGraph::setConnections(GraphNodeIndex nodeIndex, uint32_t column, uint32_t row)
{
   if(row == 0u)
   {
      if(column == 0u)
      {
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
         connect(nodeIndex, nodeIndex + mNodesColumns, LineWeight);          // down
         connect(nodeIndex, nodeIndex + mNodesColumns + 1u, DiagonalWeight); // down right
      }
      else if(column == (mNodesColumns - 1u))
      {
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + mNodesColumns - 1u, DiagonalWeight); // down left
         connect(nodeIndex, nodeIndex + mNodesColumns, LineWeight);          // down
      }
      else
      {
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
         connect(nodeIndex, nodeIndex + mNodesColumns - 1u, DiagonalWeight); // down left
         connect(nodeIndex, nodeIndex + mNodesColumns, LineWeight);          // down
         connect(nodeIndex, nodeIndex + mNodesColumns + 1u, DiagonalWeight); // down right
      }
   }
   else if(row == (mNodesRows - 1u))
   {
      if(column == 0u)
      {
         connect(nodeIndex, nodeIndex - mNodesColumns, LineWeight);          // up
         connect(nodeIndex, nodeIndex - mNodesColumns + 1u, DiagonalWeight); // up right
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
      }
      else if(column == (mNodesColumns - 1u))
      {
         connect(nodeIndex, nodeIndex - mNodesColumns - 1u, DiagonalWeight); // up left
         connect(nodeIndex, nodeIndex - mNodesColumns, LineWeight);          // up
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
      }
      else
      {
         connect(nodeIndex, nodeIndex - mNodesColumns - 1u, DiagonalWeight); // up left
         connect(nodeIndex, nodeIndex - mNodesColumns, LineWeight);          // up
         connect(nodeIndex, nodeIndex - mNodesColumns + 1u, DiagonalWeight); // up right
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
      }
   }
   else
   {
      if(column == 0u)
      {
         connect(nodeIndex, nodeIndex - mNodesColumns, LineWeight);          // up
         connect(nodeIndex, nodeIndex - mNodesColumns + 1u, DiagonalWeight); // up right
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
         connect(nodeIndex, nodeIndex + mNodesColumns, LineWeight);          // down
         connect(nodeIndex, nodeIndex + mNodesColumns + 1u, DiagonalWeight); // down right
      }
      else if(column == (mNodesColumns - 1u))
      {
         connect(nodeIndex, nodeIndex - mNodesColumns - 1u, DiagonalWeight); // up left
         connect(nodeIndex, nodeIndex - mNodesColumns, LineWeight);          // up
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + mNodesColumns - 1u, DiagonalWeight); // down left
         connect(nodeIndex, nodeIndex + mNodesColumns, LineWeight);          // down
      }
      else
      {
         connect(nodeIndex, nodeIndex - mNodesColumns - 1u, DiagonalWeight); // up left
         connect(nodeIndex, nodeIndex - mNodesColumns, LineWeight);          // up
         connect(nodeIndex, nodeIndex - mNodesColumns + 1u, DiagonalWeight); // up right
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
         connect(nodeIndex, nodeIndex + mNodesColumns - 1u, DiagonalWeight); // down left
         connect(nodeIndex, nodeIndex + mNodesColumns, LineWeight);          // down
         connect(nodeIndex, nodeIndex + mNodesColumns + 1u, DiagonalWeight); // down right
      }
   }
}

void MeshGraph::resetConnections()
{
   for(uint32_t i = 0u; i < NumberOfNodes; i++)
   {
      AdjacencyList[i].clear();
   }

   setConnections();
}
