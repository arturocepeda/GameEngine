
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEManagedContent.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEObject.h"

namespace GE { namespace Content
{
   enum class ManagedContentType
   {
      Mesh,
      Skeleton,
      AnimationSet,
   };


   class ManagedContent : public Core::Object
   {
   protected:
      ManagedContent(const Core::ObjectName& Name);
      ~ManagedContent();
   };
}}
