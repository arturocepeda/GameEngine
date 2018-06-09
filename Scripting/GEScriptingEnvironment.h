
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Scripting
//
//  --- GEScriptingEnvironment.h ---
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

namespace GE { namespace Scripting
{
   class ScriptingEnvironment
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
      bool bReset;

      GESTLVector(Core::ObjectName) vGlobalVariableNames;
      GESTLVector(Core::ObjectName) vGlobalFunctionNames;

      GESTLMap(uint32_t, ScriptFunction) mFunctions;

#if defined (GE_EDITOR_SUPPORT)
      uint iDebugBreakpointLine;
      bool bDebuggerActive;
#endif
      static void* pAllocatorBuffer;
      static tlsf_t pAllocator;

      static GESTLSet(uint) sPredefinedGlobalSymbols;
      static GESTLVector(registerTypesExtension) vRegisterTypesExtensions;

      static void* customAlloc(void*, void* ptr, size_t, size_t nsize);
      static bool alphabeticalComparison(const Core::ObjectName& l, const Core::ObjectName& r);

      bool loadModule(const char* sModuleName);

      void collectGlobalSymbols();
      void registerTypes();

   public:
      ScriptingEnvironment();
      ~ScriptingEnvironment();

      static void initStaticData();
      static void releaseStaticData();

      static void addPredefinedGlobalSymbol(const Core::ObjectName& Symbol);
      static void addRegisterTypesExtension(registerTypesExtension Extension);

      static void handleScriptError(const char* ScriptName, const char* Msg = 0);
      static void handleFunctionError(const char* FunctionName, const char* Msg = 0);

      void reset();
      void collectGarbage();

      void loadFromCode(const GESTLString& Code);
      bool loadFromFile(const char* FileName);

      template<typename T>
      void setVariable(const Core::ObjectName& VariableName, T Value)
      {
         lua[VariableName.getString()] = Value;
      }
      template<typename T>
      T getVariable(const Core::ObjectName& VariableName)
      {
         return lua.get<T>(VariableName.getString());
      }

      Core::ValueType getVariableType(const Core::ObjectName& VariableName) const;

      const GESTLVector(Core::ObjectName)& getGlobalVariableNames() const { return vGlobalVariableNames; }
      const GESTLVector(Core::ObjectName)& getGlobalFunctionNames() const { return vGlobalFunctionNames; }

      bool isFunctionDefined(const Core::ObjectName& FunctionName) const;
      uint getFunctionParametersCount(const Core::ObjectName& FunctionName) const;

      template<typename ReturnType, typename... Parameters>
      ReturnType runFunction(const Core::ObjectName& FunctionName, Parameters&&... ParameterList)
      {
         ScriptFunction& luaFunction = mFunctions.find(FunctionName.getID())->second;
#if defined (GE_EDITOR_SUPPORT)
         auto luaFunctionResult = luaFunction(ParameterList...);

         if(!luaFunctionResult.valid())
         {
            sol::error luaError = luaFunctionResult;
            handleFunctionError(FunctionName.getString(), luaError.what());
            return (ReturnType)0;
         }

         return (ReturnType)luaFunctionResult;
#else
         return (ReturnType)luaFunction(ParameterList...);
#endif
      }

#if defined (GE_EDITOR_SUPPORT)
      void setDebugBreakpointLine(uint Line);
      void enableDebugger();
      void disableDebugger();
#endif
   };
}}
