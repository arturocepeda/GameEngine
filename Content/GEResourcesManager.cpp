
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
   registerSimpleResourceTypes();
   registerSerializableResourceTypes();

   loadBuiltInMeshes();
}

ResourcesManager::~ResourcesManager()
{
   mSkeletons.clear();
   mAnimationSets.clear();
}

void ResourcesManager::registerSimpleResourceTypes()
{
   mSimpleResourceManagersRegistry[Skeleton::TypeName.getID()] = &mSkeletons;
   registerObjectManager<Skeleton>(Skeleton::TypeName, &mSkeletons);

   mSimpleResourceManagersRegistry[AnimationSet::TypeName.getID()] = &mAnimationSets;
   registerObjectManager<AnimationSet>(AnimationSet::TypeName, &mAnimationSets);
}

void ResourcesManager::registerSerializableResourceTypes()
{
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<Mesh>(&mMeshes);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<Curve>(&mCurves);
   SerializableResourcesManager::getInstance()->registerSerializableResourceType<BezierCurve>(&mBezierCurves);
}

void ResourcesManager::loadBuiltInMeshes()
{
   Sphere cSphere(1.0f, 48, 24);
   Mesh* cBuiltInMesh = Allocator::alloc<Mesh>();
   GEInvokeCtor(Mesh, cBuiltInMesh)(cSphere, ObjectName("Sphere"));
   SerializableResourcesManager::getInstance()->add<Mesh>(cBuiltInMesh);

   Quad cQuad(1.0f);
   cBuiltInMesh = Allocator::alloc<Mesh>();
   GEInvokeCtor(Mesh, cBuiltInMesh)(cQuad, ObjectName("Quad"));
   SerializableResourcesManager::getInstance()->add<Mesh>(cBuiltInMesh);

   Cube cCube(1.0f);
   cBuiltInMesh = Allocator::alloc<Mesh>();
   GEInvokeCtor(Mesh, cBuiltInMesh)(cCube, ObjectName("Cube"));
   SerializableResourcesManager::getInstance()->add<Mesh>(cBuiltInMesh);
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

   return nullptr;
}

#if defined (GE_EDITOR_SUPPORT)
void SerializableResourcesManager::registerSourceFile(const ObjectName& pResourceType, const ObjectName& pGroupName,
   const char* pSubDir, const char* pExtension)
{
   SourceFileRegistry::iterator it = mSourceFileRegistry.find(pResourceType.getID());

   if(it == mSourceFileRegistry.end())
   {
      mSourceFileRegistry[pResourceType.getID()] = ResourceGroupSourceFileRegistry();
      it = mSourceFileRegistry.find(pResourceType.getID());
   }

   GESTLString sourceFile;
   sourceFile.append(pSubDir);
   sourceFile.append("/");
   sourceFile.append(pGroupName.getString());
   sourceFile.append(".");
   sourceFile.append(pExtension);
   it->second[pGroupName.getID()] = sourceFile;
}

GESTLString* SerializableResourcesManager::retrieveSourceFile(const ObjectName& pResourceType, const ObjectName& pGroupName)
{
   SourceFileRegistry::iterator itResource = mSourceFileRegistry.find(pResourceType.getID());

   if(itResource != mSourceFileRegistry.end())
   {
      ResourceGroupSourceFileRegistry::iterator itGroup = itResource->second.find(pGroupName.getID());

      if(itGroup != itResource->second.end())
      {
         return &itGroup->second;
      }
   }

   return nullptr;
}
#endif
