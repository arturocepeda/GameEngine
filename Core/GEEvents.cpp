
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEEvents.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEEvents.h"

using namespace GE::Core;


//
//  Serializable
//
const ObjectName Events::PropertiesUpdated = ObjectName("PropertiesUpdated");


//
//  Resources
//
const ObjectName Events::ResourceCreated = ObjectName("ResourceCreated");
const ObjectName Events::ResourceDestroyed = ObjectName("ResourceDestroyed");


//
//  Rendering
//
const ObjectName Events::RenderingSurfaceChanged = ObjectName("RenderingSurfaceChanged");


//
//  Scene
//
const ObjectName Events::ActiveSceneSet = ObjectName("ActiveSceneSet");
const ObjectName Events::EntityAdded = ObjectName("EntityAdded");
const ObjectName Events::EntityRenamed = ObjectName("EntityRenamed");
const ObjectName Events::EntityRemoved = ObjectName("EntityRemoved");
const ObjectName Events::EntityParentChanged = ObjectName("EntityParentChanged");
