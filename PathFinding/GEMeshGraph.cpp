
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

MeshGraph::MeshGraph(uint32_t nodesColumns, uint32_t nodesRows)
   : Graph(nodesColumns * nodesRows)
{
   this->nodesColumns = nodesColumns;
   this->nodesRows = nodesRows;
}

MeshGraph::~MeshGraph()
{
}

uint32_t MeshGraph::getNumberOfColumns() const
{
   return nodesColumns;
}

uint32_t MeshGraph::getNumberOfRows() const
{
   return nodesRows;
}

void MeshGraph::setNodePositions(float firstX, float firstY, float incrementX, float incrementY)
{
   GraphNodeIndex nodeIndex = 0;

   float nodePositionY = firstY;

   for(uint32_t row = 0; row < nodesRows; row++)
   {
      float nodePositionX = firstX;

      for(uint32_t column = 0; column < nodesColumns; column++)
      {
         Nodes[nodeIndex].PosX = nodePositionX;
         Nodes[nodeIndex].PosY = nodePositionY;
         nodePositionX += incrementX;
         nodeIndex++;
      }

      nodePositionY += incrementY;
   }
}

void MeshGraph::setConnections()
{
   GraphNodeIndex nodeIndex = 0;

   for(uint32_t row = 0; row < nodesRows; row++)
   {
      for(uint32_t column = 0; column < nodesColumns; column++)
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
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
         connect(nodeIndex, nodeIndex + nodesColumns + 1u, DiagonalWeight);  // down right
      }
      else if(column == (nodesColumns - 1u))
      {
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + nodesColumns - 1u, DiagonalWeight);  // down left
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down							
      }
      else
      {
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
         connect(nodeIndex, nodeIndex + nodesColumns - 1u, DiagonalWeight);  // down left
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
         connect(nodeIndex, nodeIndex + nodesColumns + 1u, DiagonalWeight);  // down right
      }
   }
   else if(row == (nodesRows - 1u))
   {
      if(column == 0u)
      {
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - nodesColumns + 1u, DiagonalWeight);  // up right
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
      }
      else if(column == (nodesColumns - 1u))
      {
         connect(nodeIndex, nodeIndex - nodesColumns - 1u, DiagonalWeight);  // up left
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
      }
      else
      {
         connect(nodeIndex, nodeIndex - nodesColumns - 1u, DiagonalWeight);  // up left
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - nodesColumns + 1u, DiagonalWeight);  // up right
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
      }
   }
   else
   {
      if(column == 0u)
      {
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - nodesColumns + 1u, DiagonalWeight);  // up right
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
         connect(nodeIndex, nodeIndex + nodesColumns + 1u, DiagonalWeight);  // down right
      }
      else if(column == (nodesColumns - 1u))
      {
         connect(nodeIndex, nodeIndex - nodesColumns - 1u, DiagonalWeight);  // up left
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + nodesColumns - 1u, DiagonalWeight);  // down left
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
      }
      else
      {
         connect(nodeIndex, nodeIndex - nodesColumns - 1u, DiagonalWeight);  // up left
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - nodesColumns + 1u, DiagonalWeight);  // up right
         connect(nodeIndex, nodeIndex - 1u, LineWeight);                     // left
         connect(nodeIndex, nodeIndex + 1u, LineWeight);                     // right
         connect(nodeIndex, nodeIndex + nodesColumns - 1u, DiagonalWeight);  // down left
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
         connect(nodeIndex, nodeIndex + nodesColumns + 1u, DiagonalWeight);  // down right
      }
   }
}

void MeshGraph::resetConnections()
{
   for(uint32_t i = 0; i < NumberOfNodes; i++)
   {
      AdjacencyList[i].clear();
   }

   setConnections();
}
