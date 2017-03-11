
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Entities
//
//  --- GEComponentScript.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEComponentScript.h"
#include "Content/GEContentData.h"
#include "Core/GETime.h"
#include "Entities/GEScene.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Entities;
using namespace GE::Content;


ComponentScript::ComponentScript(Entity* Owner)
   : Component(Owner)
   , bInitialized(false)
{
   cClassName = ObjectName("Script");

   cScript = Allocator::alloc<Script>();
   GEInvokeCtor(Script, cScript);

   GERegisterProperty(ComponentScript, String, ScriptInit);
   GERegisterProperty(ComponentScript, String, ScriptUpdate);
}

ComponentScript::~ComponentScript()
{
   GEInvokeDtor(Script, cScript);
   Allocator::free(cScript);
}

void ComponentScript::update()
{
   if(!bInitialized)
   {
      cScript->setVariableObject<Entity>("entity", cOwner);

      if(!sScriptInit.empty())
         cScript->runFunction("init");

      bInitialized = true;
   }

   if(sScriptUpdate.empty())
      return;

   cScript->setVariableObject<Scene>("scene", Scene::getActiveScene());
   cScript->setVariableFloat("deltaTime", Time::getClock(0).getDelta());
   
   cScript->runFunction("update");
}

void ComponentScript::setScriptInit(const char* FileName)
{
   if(!FileName || strlen(FileName) == 0)
      return;

   sScriptInit = FileName;
   cScript->loadFromFile(FileName);
}

const char* ComponentScript::getScriptInit() const
{
   return sScriptInit.c_str();
}

void ComponentScript::setScriptUpdate(const char* FileName)
{
   if(!FileName || strlen(FileName) == 0)
      return;

   sScriptUpdate = FileName;
   cScript->loadFromFile(FileName);
}

const char* ComponentScript::getScriptUpdate() const
{
   return sScriptUpdate.c_str();
}
