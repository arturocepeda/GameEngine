
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

void Component::copy(Component* cSource)
{
   for(uint i = 0; i < cSource->getPropertiesCount(); i++)
   {
      const Property& sSourceProperty = cSource->getProperty(i);

      if(!sSourceProperty.Setter)
         continue;

      const Property& sTargetProperty = getProperty(i);
      Value cSourcePropertyValue = sSourceProperty.Getter();
      sTargetProperty.Setter(cSourcePropertyValue);
   }
}
