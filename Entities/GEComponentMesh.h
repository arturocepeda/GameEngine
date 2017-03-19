
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
      uint8_t eDynamicShadows;
      ComponentSkeleton* cSkeleton;

   public:
      ComponentMesh(Entity* Owner);
      ~ComponentMesh();

      const Core::ObjectName& getMeshName() const;
      void setMeshName(const Core::ObjectName& MeshName);

      uint8_t getDynamicShadows() const;
      void setDynamicShadows(uint8_t BitMask);

      void loadMesh(Content::Mesh* M);
      void unload();

      Content::Mesh* getMesh() const;

      void updateSkinning();

      GEProperty(ObjectName, MeshName)
      GEPropertyBitMask(DynamicShadowsBitMask, DynamicShadows)
   };
}}
