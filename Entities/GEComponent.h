
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponent.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Core/GESerializable.h"

namespace GE { namespace Entities
{
   class Entity;

   class Component : public Core::Serializable
   {
   protected:
      Entity* cOwner;

      Component(Entity* Owner);

   public:
      virtual ~Component();

      Entity* getOwner() const;

      virtual void copy(Component* cSource);
   };
}}
