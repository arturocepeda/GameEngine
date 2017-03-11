
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEScript.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GESingleton.h"
#include "Entities/GEEntity.h"

#include "Externals/sol2/sol.hpp"

namespace GE { namespace Core
{
   class Script
   {
   private:
      sol::state lua;

      void registerTypes();

   public:
      Script();
      ~Script();

      void loadFromCode(const GESTLString& Code);
      void loadFromFile(const char* FileName);

      void setVariableInt(const char* VariableName, int Value);
      void setVariableFloat(const char* VariableName, float Value);

      template<typename T>
      void setVariableObject(const char* VariableName, T* Ptr) { lua[VariableName] = Ptr; }

      bool isFunctionDefined(const char* FunctionName) const;
      void runFunction(const char* FunctionName);
   };
}}
