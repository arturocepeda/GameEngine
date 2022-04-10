
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
#include "Core/GELog.h"
#include "Entities/GEEntity.h"
#include "Types/GESTLTypes.h"

#define SOL_SAFE_NUMERICS 1

#include "Externals/sol2/sol.hpp"
#include "Externals/tlsf/tlsf.h"

#define GE_SAFE_FUNCTIONS 1


namespace GE { namespace Content
{
   class ContentData;
}}


namespace GE { namespace Scripting
{
   typedef sol::state State;
   typedef sol::table Table;
#if (GE_SAFE_FUNCTIONS)
   typedef sol::protected_function Function;
#else
   typedef sol::function Function;
#endif


   static void handleScriptError(const char* pScriptName, const char* pMsg = nullptr)
   {
      if(pMsg)
      {
         Core::Log::log(
            Core::LogType::Error, "Lua error (the '%s' script could not be loaded): %s", pScriptName, pMsg);
      }
      else
      {
         Core::Log::log(
            Core::LogType::Error, "Lua error (the '%s' script could not be loaded)", pScriptName);
      }
   }

   static void handleFunctionError(const char* pFunctionName, const char* pMsg = nullptr)
   {
      if(pMsg)
      {
         Core::Log::log(Core::LogType::Error, "Lua error ('%s' function): %s", pFunctionName, pMsg);
      }
      else
      {
         Core::Log::log(Core::LogType::Error, "Lua error ('%s' function)", pFunctionName);
      }
   }


   class Namespace
   {
   private:
      GESTLString mName;
      Namespace* mParent;
      State* mState;
      Table mTable;

      GESTLVector(Core::ObjectName) mGlobalVariableNames;
      GESTLVector(Core::ObjectName) mGlobalFunctionNames;
      GESTLMap(uint32_t, Function) mFunctions;
      GESTLMap(uint32_t, Namespace) mNamespaces;

      static bool alphabeticalComparison(const Core::ObjectName& pL, const Core::ObjectName& pR);

      void collectSymbols();

   public:
      Namespace();
      Namespace(State* pState, const char* pName);
      ~Namespace();

      void load(Namespace* pParent);
      void unload();

      Namespace* getParent() { return mParent; }

      Namespace* addNamespace(const Core::ObjectName& pName);
      Namespace* addNamespaceFromModule(const Core::ObjectName& pName, const char* pModuleName);
      Namespace* getNamespace(const Core::ObjectName& pName);
      void removeNamespace(const Core::ObjectName& pName);

      template<typename T>
      void setVariable(const Core::ObjectName& pVariableName, T pValue)
      {
         mTable[pVariableName.getString()] = pValue;
      }
      template<typename T>
      T getVariable(const Core::ObjectName& pVariableName)
      {
         return mTable.get<T>(pVariableName.getString());
      }

      Core::ValueType getVariableType(const Core::ObjectName& pVariableName) const;

      const GESTLVector(Core::ObjectName)& getGlobalVariableNames() const { return mGlobalVariableNames; }
      const GESTLVector(Core::ObjectName)& getGlobalFunctionNames() const { return mGlobalFunctionNames; }

      bool isFunctionDefined(const Core::ObjectName& pFunctionName) const;
      uint32_t getFunctionParametersCount(const Core::ObjectName& pFunctionName) const;

      template<typename ReturnType, typename... Parameters>
      ReturnType runFunction(const Core::ObjectName& pFunctionName, Parameters&&... pParameterList)
      {
         Function& luaFunction = mFunctions.find(pFunctionName.getID())->second;
#if (GE_SAFE_FUNCTIONS)
         auto luaFunctionResult = luaFunction(pParameterList...);

         if(!luaFunctionResult.valid())
         {
            sol::error luaError = luaFunctionResult;
            handleFunctionError(pFunctionName.getString(), luaError.what());
            return (ReturnType)0;
         }

         return (ReturnType)luaFunctionResult;
#else
         return (ReturnType)luaFunction(pParameterList...);
#endif
      }
   };


   class Environment
   {
   public:
      typedef std::function<void(State&)> registerTypesExtension;

   private:
      State mLua;
      Namespace mGlobalNamespace;

#if defined (GE_EDITOR_SUPPORT)
      uint mDebugBreakpointLine;
      bool mDebuggerActive;
#endif
      static void* smAllocatorBuffer;
      static tlsf_t smAllocator;

      static GESTLVector(registerTypesExtension) smRegisterTypesExtensions;

      static void* customAlloc(void*, void* pPtr, size_t pOldSize, size_t pNewSize);

      static void collectPredefinedGlobalSymbols();
      static void addPredefinedGlobalSymbol(const Core::ObjectName& pSymbol);

      void registerTypes();

   public:
      Environment();
      ~Environment();

      static void initStaticData();
      static void releaseStaticData();

      static void addRegisterTypesExtension(registerTypesExtension pExtension);

      static bool loadModuleContent(Content::ContentData* pContentData, const char* pModuleName);
      static bool loadModule(State* pState, const char* pModuleName, Table* pOutReturnValue);

      static size_t compileScript(const char* pCode, size_t pBytecodeMaxSize, char* pOutBytecode);

      void load();
      void collectGarbage();
      void collectGarbageStep();

      void loadFromCode(const GESTLString& pCode);
      bool loadFromFile(const char* pFileName);

      Namespace* getGlobalNamespace() { return &mGlobalNamespace; }

#if defined (GE_EDITOR_SUPPORT)
      void setDebugBreakpointLine(uint32_t pLine);
      void enableDebugger();
      void disableDebugger();
#endif
   };
}}
