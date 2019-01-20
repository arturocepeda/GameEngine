
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
   GESerializableEnum(MeshSettingsBitMask)
   {
      Transparency  = 1 << 0,
      Skinning      = 1 << 1,

      Count = 2
   };


   class ComponentMesh : public ComponentRenderable
   {
   private:
      Content::Mesh* cMesh;
      uint8_t eDynamicShadows;
      uint8_t eSettings;

   public:
      static const Core::ObjectName ClassName;

      ComponentMesh(Entity* Owner);
      ~ComponentMesh();

      const Core::ObjectName& getMeshName() const;
      void setMeshName(const Core::ObjectName& MeshName);

      uint8_t getDynamicShadows() const { return eDynamicShadows; }
      void setDynamicShadows(uint8_t BitMask) { eDynamicShadows = BitMask; }

      uint8_t getSettings() const { return eSettings; }
      void setSettings(uint8_t BitMask) { eSettings = BitMask; }

      void loadMesh(Content::Mesh* M);
      void unload();

      Content::Mesh* getMesh() const;

      void updateSkinning();
   };
}}
