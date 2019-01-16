
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentTransform.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentTransform.h"

using namespace GE;
using namespace GE::Entities;
using namespace GE::Core;

const ObjectName ResetActionName = ObjectName("Reset");

ComponentTransform::ComponentTransform(Entity* Owner)
   : Component(Owner)
   , vPosition(Vector3::Zero)
   , vScale(Vector3::One)
   , vForward(Vector3::UnitZ)
   , vUp(Vector3::UnitY)
{
   mClassNames.push_back(ObjectName("Transform"));

   updateWorldMatrix();

   GERegisterProperty(Vector3, Position);
   GERegisterProperty(Vector3, Orientation);
   GERegisterProperty(Vector3, Scale);

   GERegisterPropertyReadonly(Vector3, WorldPosition);
   GERegisterPropertyReadonly(Vector3, WorldOrientation);
   GERegisterPropertyReadonly(Vector3, WorldScale);

   registerAction(ResetActionName, [this]{ reset(); });
}

ComponentTransform::~ComponentTransform()
{
}
