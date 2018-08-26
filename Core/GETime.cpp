
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GETime.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GETime.h"

#include "Content/GEResourcesManager.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;


//
//  Clock
//
const ObjectName Clock::TypeName = ObjectName("Clock");
const float Clock::MaxDelta = 0.1f;

Clock::Clock(const ObjectName& Name, const ObjectName& GroupName)
   : Resource(Name, GroupName, TypeName)
   , fDelta(0.0f)
   , fTimeFactor(1.0f)
{
   GERegisterProperty(Float, TimeFactor);

   registerAction("Run", [this] { fTimeFactor = 1.0f; });
   registerAction("Stop", [this] { fTimeFactor = 0.0f; });
}

Clock::~Clock()
{
}


//
//  Time
//
const ObjectName Time::DefaultClockName = ObjectName("Default");

float Time::fElapsed = 0.0f;
ObjectManager<Clock>* Time::mClocks = 0;

void Time::init()
{
   mClocks = Allocator::alloc<ObjectManager<Clock>>();
   GEInvokeCtor(ObjectManager<Clock>, mClocks);

   Clock* cDefaultClock = Allocator::alloc<Clock>();
   GEInvokeCtor(Clock, cDefaultClock)(DefaultClockName, ObjectName::Empty);
   mClocks->add(cDefaultClock);

   SerializableResourcesManager::getInstance()->registerSerializableResourceType<Clock>(mClocks);
}

void Time::release()
{
   GEInvokeDtor(ObjectManager<Clock>, mClocks);
   Allocator::free(mClocks);
}

float Time::getElapsed()
{
   return fElapsed;
}

Clock* Time::getDefaultClock()
{
   return mClocks->get(DefaultClockName);
}

Clock* Time::getClock(const ObjectName& pClockName)
{
   return mClocks->get(pClockName);
}

void Time::reset()
{
   fElapsed = 0.0f;
}

void Time::setDelta(float DeltaTime)
{
   fElapsed += DeltaTime;

   mClocks->iterate([DeltaTime](Clock* pClock) -> bool
   {
      pClock->setDelta(DeltaTime);
      return true;
   });
}
