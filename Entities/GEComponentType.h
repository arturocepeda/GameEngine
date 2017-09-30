
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentType.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

namespace GE { namespace Entities
{
   enum class ComponentType
   {
      Transform,
      Camera,
      Light,
      Renderable,
      UIElement,
      Skeleton,
      DataContainer,
      Collider,
      Script,

      Count
   };
}}
