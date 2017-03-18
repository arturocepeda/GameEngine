
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
#include "Core/GEPlatform.h"
#include "Entities/GEEntity.h"
#include "Types/GESTLTypes.h"

#if defined (GE_PLATFORM_IOS)
# if !defined (SOL_USING_CXX_LUA)
#  define SOL_USING_CXX_LUA
# endif
#endif

#include "Externals/sol2/sol.hpp"

namespace GE { namespace Core
{
   class Script
   {
   private:
      sol::state lua;

      GESTLVector(ObjectName) vGlobalVariableNames;
      GESTLVector(ObjectName) vGlobalFunctionNames;

      static GESTLSet(uint) sDefaultGlobalNames;

      void collectGlobalSymbols();
      void registerTypes();

   public:
      Script();
      ~Script();

      static void handleScriptError(const char* ScriptName);
      static void handleFunctionError(const char* FunctionName);

      void loadFromCode(const GESTLString& Code);
      void loadFromFile(const char* FileName);

      template<typename T>
      void setVariable(const char* VariableName, T Value)
      {
         lua[VariableName] = Value;
      }
      template<typename T>
      T getVariable(const char* VariableName)
      {
         return lua.get<T>(VariableName);
      }

      ValueType getVariableType(const char* VariableName) const;

      const GESTLVector(ObjectName)& getGlobalVariableNames() const { return vGlobalVariableNames; }
      const GESTLVector(ObjectName)& getGlobalFunctionNames() const { return vGlobalFunctionNames; }

      bool isFunctionDefined(const ObjectName& FunctionName) const;
      void runFunction(const char* FunctionName);

      template<typename T>
      std::function<T> getFunction(const char* FunctionName)
      {
         return (std::function<T>)lua[FunctionName];
      }
   };
}}
