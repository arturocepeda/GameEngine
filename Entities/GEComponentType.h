
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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
      Audio,
      Script,

      Count
   };
}}
