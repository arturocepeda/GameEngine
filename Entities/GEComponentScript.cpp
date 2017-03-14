
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

   GERegisterProperty(ComponentScript, String, ScriptName);
}

ComponentScript::~ComponentScript()
{
   GEInvokeDtor(Script, cScript);
   Allocator::free(cScript);
}

void ComponentScript::update()
{
   if(sScriptName.empty())
      return;

   if(!bInitialized)
   {
      cScript->setVariableObject<Entity>("entity", cOwner);

      if(cScript->isFunctionDefined("init"))
      {
         cScript->runFunction("init");
      }

      bInitialized = true;
   }

   cScript->setVariableObject<Scene>("scene", Scene::getActiveScene());
   cScript->setVariableFloat("deltaTime", Time::getClock(0).getDelta());
   
   if(cScript->isFunctionDefined("update"))
   {
      cScript->runFunction("update");
   }
}

void ComponentScript::setScriptName(const char* FileName)
{
   if(!FileName || strlen(FileName) == 0)
      return;

   sScriptName = FileName;
   cScript->loadFromFile(FileName);
   bInitialized = false;
}

const char* ComponentScript::getScriptName() const
{
   return sScriptName.c_str();
}
