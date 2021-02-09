
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
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
static const ObjectName kClassName("Component");

Component::Component(Entity* Owner)
   : Serializable(kClassName)
   , cOwner(Owner)
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
