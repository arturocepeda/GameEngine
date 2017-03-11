
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEContentManager.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEContentManager.h"
#include "Core/GEAllocator.h"
#include "Rendering/GEPrimitives.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;
using namespace GE::Rendering;

ContentManager::ContentManager()
{
   mManagersRegistry[Mesh::ContentType] = &mMeshes;
   mManagersRegistry[Skeleton::ContentType] = &mSkeletons;
   mManagersRegistry[AnimationSet::ContentType] = &mAnimationSets;

   ObjectManagers::getInstance()->registerObjectManager<Mesh>("Mesh", &mMeshes);
   ObjectManagers::getInstance()->registerObjectManager<Skeleton>("Skeleton", &mSkeletons);
   ObjectManagers::getInstance()->registerObjectManager<AnimationSet>("AnimationSet", &mAnimationSets);

   loadBuiltInMeshes();
}

ContentManager::~ContentManager()
{
   mMeshes.clear();
   mSkeletons.clear();
   mAnimationSets.clear();
}

void ContentManager::loadBuiltInMeshes()
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
