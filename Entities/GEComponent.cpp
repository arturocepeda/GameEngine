
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponent.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponent.h"

using namespace GE;
using namespace GE::Entities;
using namespace GE::Core;

//
//  Component
//
Component::Component(Entity* Owner)
   : cOwner(Owner)
   , Serializable("Component")
{
   GEAssert(Owner);
}

Component::~Component()
{
}

Entity* Component::getOwner() const
{
   return cOwner;
}
