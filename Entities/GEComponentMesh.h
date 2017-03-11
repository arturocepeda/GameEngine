
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentMesh.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEComponentRenderable.h"
#include "GEComponentSkeleton.h"
#include "Rendering/GEPrimitives.h"
#include "Core/GEObjectManager.h"
#include "Content/GEMesh.h"

namespace GE { namespace Entities
{
   class ComponentMesh : public ComponentRenderable
   {
   private:
      Content::Mesh* cMesh;
      bool bDynamicShadows;
      ComponentSkeleton* cSkeleton;

   public:
      ComponentMesh(Entity* Owner);
      ~ComponentMesh();

      const Core::ObjectName& getMeshName() const;
      void setMeshName(const Core::ObjectName& MeshName);

      bool getDynamicShadows() const;
      void setDynamicShadows(bool DynamicShadows);

      void loadMesh(Content::Mesh* M);
      void unload();

      Content::Mesh* getMesh() const;

      void updateSkinning();

      GEProperty(ObjectName, MeshName)
      GEProperty(Bool, DynamicShadows)
   };
}}
