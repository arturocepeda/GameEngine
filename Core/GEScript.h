
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
#include "Externals/tlsf/tlsf.h"

namespace GE { namespace Core
{
   class Script
   {
   public:
      typedef std::function<void(sol::state&)> registerTypesExtension;

   private:
#if defined (GE_EDITOR_SUPPORT)
      typedef sol::protected_function ScriptFunction;
#else
      typedef sol::function ScriptFunction;
#endif
      sol::state lua;

      GESTLVector(ObjectName) vGlobalVariableNames;
      GESTLVector(ObjectName) vGlobalFunctionNames;

      GESTLMap(uint32_t, ScriptFunction) mFunctions;

      static void* pAllocatorBuffer;
      static tlsf_t pAllocator;

      static GESTLSet(uint) sDefaultGlobalNames;
      static GESTLVector(registerTypesExtension) vRegisterTypesExtensions;

      static void* customAlloc(void*, void* ptr, size_t, size_t nsize);
      static bool alphabeticalComparison(const ObjectName& l, const ObjectName& r);

      void collectGlobalSymbols();
      void registerTypes();

   public:
      Script();
      ~Script();

      static void initStaticData();
      static void releaseStaticData();

      static void addRegisterTypesExtension(registerTypesExtension Extension);

      static void handleScriptError(const char* ScriptName, const char* Msg = 0);
      static void handleFunctionError(const char* FunctionName, const char* Msg = 0);

      void reset();

      void loadFromCode(const GESTLString& Code);
      bool loadFromFile(const char* FileName);

      template<typename T>
      void setVariable(const ObjectName& VariableName, T Value)
      {
         lua[VariableName.getString().c_str()] = Value;
      }
      template<typename T>
      T getVariable(const ObjectName& VariableName)
      {
         return lua.get<T>(VariableName.getString().c_str());
      }

      ValueType getVariableType(const ObjectName& VariableName) const;

      const GESTLVector(ObjectName)& getGlobalVariableNames() const { return vGlobalVariableNames; }
      const GESTLVector(ObjectName)& getGlobalFunctionNames() const { return vGlobalFunctionNames; }

      bool isFunctionDefined(const ObjectName& FunctionName) const;

      template<typename ReturnType, typename... Parameters>
      ReturnType runFunction(const ObjectName& FunctionName, Parameters&&... ParameterList)
      {
         ScriptFunction& luaFunction = mFunctions.find(FunctionName.getID())->second;
#if defined (GE_EDITOR_SUPPORT)
         auto luaFunctionResult = luaFunction(ParameterList...);

         if(!luaFunctionResult.valid())
         {
            sol::error luaError = luaFunctionResult;
            handleFunctionError(FunctionName.getString().c_str(), luaError.what());
            return (ReturnType)0;
         }

         return (ReturnType)luaFunctionResult;
#else
         return (ReturnType)luaFunction(ParameterList...);
#endif
      }
   };
}}
