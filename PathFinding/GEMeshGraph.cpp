
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

const int LineWeight = 100;
const int DiagonalWeight = 141;

MeshGraph::MeshGraph(int nodesColumns, int nodesRows) 
   : Graph(nodesColumns * nodesRows)
{
   this->nodesColumns = nodesColumns;
   this->nodesRows = nodesRows;
}

MeshGraph::~MeshGraph()
{
}

int MeshGraph::getNumberOfColumns()
{
   return nodesColumns;
}

int MeshGraph::getNumberOfRows()
{
   return nodesRows;
}

void MeshGraph::setNodePositions(float firstX, float firstY, float incrementX, float incrementY)
{
   int nodeIndex = 0;
   float nodePositionX;
   float nodePositionY = firstY;

   for(int row = 0; row < nodesRows; row++)
   {
      nodePositionX = firstX;

      for(int column = 0; column < nodesColumns; column++)
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
   int nodeIndex = 0;

   for(int row = 0; row < nodesRows; row++)
   {
      for(int column = 0; column < nodesColumns; column++)
      {
         setConnections(nodeIndex, column, row);
         nodeIndex++;
      }
   }
}

void MeshGraph::setConnections(int nodeIndex, int column, int row)
{
   if(row == 0)
   {
      if(column == 0)
      {
         connect(nodeIndex, nodeIndex + 1, LineWeight);                      // right
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
         connect(nodeIndex, nodeIndex + nodesColumns + 1, DiagonalWeight);   // down right
      }
      else if(column == (nodesColumns - 1))
      {
         connect(nodeIndex, nodeIndex - 1, LineWeight);                      // left
         connect(nodeIndex, nodeIndex + nodesColumns - 1, DiagonalWeight);   // down left
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down							
      }
      else
      {
         connect(nodeIndex, nodeIndex - 1, LineWeight);                      // left
         connect(nodeIndex, nodeIndex + 1, LineWeight);                      // right
         connect(nodeIndex, nodeIndex + nodesColumns - 1, DiagonalWeight);   // down left
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
         connect(nodeIndex, nodeIndex + nodesColumns + 1, DiagonalWeight);   // down right
      }
   }
   else if(row == (nodesRows - 1))
   {
      if(column == 0)
      {
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - nodesColumns + 1, DiagonalWeight);   // up right
         connect(nodeIndex, nodeIndex + 1, LineWeight);                      // right
      }
      else if(column == (nodesColumns - 1))
      {
         connect(nodeIndex, nodeIndex - nodesColumns - 1, DiagonalWeight);   // up left
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - 1, LineWeight);                      // left
      }
      else
      {
         connect(nodeIndex, nodeIndex - nodesColumns - 1, DiagonalWeight);   // up left
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - nodesColumns + 1, DiagonalWeight);   // up right
         connect(nodeIndex, nodeIndex - 1, LineWeight);                      // left
         connect(nodeIndex, nodeIndex + 1, LineWeight);                      // right
      }
   }
   else
   {
      if(column == 0)
      {
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - nodesColumns + 1, DiagonalWeight);   // up right
         connect(nodeIndex, nodeIndex + 1, LineWeight);                      // right
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
         connect(nodeIndex, nodeIndex + nodesColumns + 1, DiagonalWeight);   // down right
      }
      else if(column == (nodesColumns - 1))
      {
         connect(nodeIndex, nodeIndex - nodesColumns - 1, DiagonalWeight);   // up left
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - 1, LineWeight);                      // left
         connect(nodeIndex, nodeIndex + nodesColumns - 1, DiagonalWeight);   // down left
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
      }
      else
      {
         connect(nodeIndex, nodeIndex - nodesColumns - 1, DiagonalWeight);   // up left
         connect(nodeIndex, nodeIndex - nodesColumns, LineWeight);           // up
         connect(nodeIndex, nodeIndex - nodesColumns + 1, DiagonalWeight);   // up right
         connect(nodeIndex, nodeIndex - 1, LineWeight);                      // left
         connect(nodeIndex, nodeIndex + 1, LineWeight);                      // right
         connect(nodeIndex, nodeIndex + nodesColumns - 1, DiagonalWeight);   // down left
         connect(nodeIndex, nodeIndex + nodesColumns, LineWeight);           // down
         connect(nodeIndex, nodeIndex + nodesColumns + 1, DiagonalWeight);   // down right
      }
   }
}

void MeshGraph::resetConnections()
{
   for(unsigned int i = 0; i < NumberOfNodes; i++)
      AdjacencyList[i].clear();

   setConnections();
}
