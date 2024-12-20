
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEEvents.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEObject.h"

namespace GE { namespace Core
{
   class Events
   {
   public:
      // Serializable
      static const ObjectName PropertiesUpdated;

      // Resources
      static const ObjectName ResourceCreated;
      static const ObjectName ResourceDestroyed;

      // Rendering
      static const ObjectName RenderingSurfaceChanged;
      static const ObjectName RenderableColorChanged;

      // Scene
      static const ObjectName ActiveSceneSet;
      static const ObjectName EntityAdded;
      static const ObjectName EntityRenamed;
      static const ObjectName EntityRemoved;
      static const ObjectName EntityParentChanged;

      // Localization
      static const ObjectName LocalizedStringsReloaded;
   };
}}
