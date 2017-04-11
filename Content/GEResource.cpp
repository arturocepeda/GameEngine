
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEResource.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEResource.h"

using namespace GE;
using namespace GE::Content;
using namespace GE::Core;


//
//  Resource
//
Resource::Resource(const ObjectName& Name, const ObjectName& GroupName)
   : Object(Name)
   , cGroupName(GroupName)
{
}

Resource::~Resource()
{
}

const Core::ObjectName& Resource::getGroupName() const
{
   return cGroupName;
}

uint Resource::getSizeInBytes() const
{
   return 0;
}
