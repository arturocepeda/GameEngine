
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEResourcesManager.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEResourcesManager.h"

#include "Rendering/GEPrimitives.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;
using namespace GE::Rendering;


//
//  ResourcesManager
//
ResourcesManager::ResourcesManager()
{
   mSimpleResourceManagersRegistry[Mesh::TypeName.getID()] = &mMeshes;
   mSimpleResourceManagersRegistry[Skeleton::TypeName.getID()] = &mSkeletons;
   mSimpleResourceManagersRegistry[AnimationSet::TypeName.getID()] = &mAnimationSets;

   registerObjectManager<Mesh>(Mesh::TypeName, &mMeshes);
   registerObjectManager<Skeleton>(Skeleton::TypeName, &mSkeletons);
   registerObjectManager<AnimationSet>(AnimationSet::TypeName, &mAnimationSets);

   registerSerializableResourceTypes();

   loadBuiltInMeshes();
}

ResourcesManager::~ResourcesManager()
{
   mMeshes.clear();
   mSkeletons.clear();
   mAnimationSets.clear();
}

void ResourcesManager::registerSerializableResourceTypes()
{
   registerObjectManager<Curve>(Curve::TypeName, &mCurves);
   registerObjectManager<BezierCurve>(BezierCurve::TypeName, &mBezierCurves);

   SerializableResourcesManager::getInstance()->registerSerializableResourceType<Curve>(&mCurves);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<BezierCurve>(&mBezierCurves);
}

void ResourcesManager::loadBuiltInMeshes()
{
   Sphere cSphere(1.0f, 48, 24);
   Mesh* cBuiltInMesh = Allocator::alloc<Mesh>();
   GEInvokeCtor(Mesh, cBuiltInMesh)(cSphere, ObjectName("Sphere"));
   add<Mesh>(cBuiltInMesh);

   Quad cQuad(1.0f);
   cBuiltInMesh = Allocator::alloc<Mesh>();
   GEInvokeCtor(Mesh, cBuiltInMesh)(cQuad, ObjectName("Quad"));
   add<Mesh>(cBuiltInMesh);

   Cube cCube(1.0f);
   cBuiltInMesh = Allocator::alloc<Mesh>();
   GEInvokeCtor(Mesh, cBuiltInMesh)(cCube, ObjectName("Cube"));
   add<Mesh>(cBuiltInMesh);
}


//
//  SerializableResourcesManager
//
SerializableResourcesManager::SerializableResourcesManager()
{
}

SerializableResourcesManager::~SerializableResourcesManager()
{
}

uint SerializableResourcesManager::getEntriesCount() const
{
   return (uint)vEntries.size();
}

SerializableResourceManagerObjects* SerializableResourcesManager::getEntry(uint Index)
{
   GEAssert(vEntries.size() > Index);
   return &vEntries[Index];
}

SerializableResourceManagerObjects* SerializableResourcesManager::getEntry(const ObjectName& ResourceTypeName)
{
   for(uint i = 0; i < vEntries.size(); i++)
   {
      if(vEntries[i].Factory->getResourceTypeName() == ResourceTypeName)
      {
         return &vEntries[i];
      }
   }

   return 0;
}
