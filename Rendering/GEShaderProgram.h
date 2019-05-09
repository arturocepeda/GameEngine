
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEShaderProgram.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypes.h"
#include "Types/GESTLTypes.h"
#include "Core/GEObject.h"
#include "Core/GEValue.h"
#include "Core/GESerializable.h"
#include "Content/GEResource.h"
#include "Rendering/GERenderingObjects.h"

#include "Externals/pugixml/pugixml.hpp"

namespace GE { namespace Rendering
{
   GESerializableEnum(VertexElementsBitMask)
   {
      Position  = 1 << 0,
      Color     = 1 << 1,
      Normal    = 1 << 2,
      Tangent   = 1 << 3,
      Binormal  = 1 << 4,
      TexCoord  = 1 << 5,

      Count = 6
   };


   class ShaderProgramPreprocessorMacro : public Core::SerializableArrayElement
   {
   private:
      char sName[64];
      char sValue[64];

   public:
      ShaderProgramPreprocessorMacro()
         : Core::SerializableArrayElement("ShaderProgramPreprocessorMacro")
      {
         sName[0] = '\0';
         sValue[0] = '\0';

         GERegisterProperty(String, Name);
         GERegisterProperty(String, Value);
      }

      const char* getName() const { return sName; }
      void setName(const char* Value) { strcpy(sName, Value); }

      const char* getValue() const { return sValue; }
      void setValue(const char* Value) { strcpy(sValue, Value); }
   };


   class ShaderProgramParameter : public Core::GenericVariable
   {
   protected:
      bool mExposed;

      ShaderProgramParameter(const Core::ObjectName& ClassName)
         : Core::GenericVariable(ClassName)
         , mExposed(true)
      {
         GERegisterProperty(Bool, Exposed);
      }

   public:
      GEDefaultGetter(bool, Exposed, m)
      GEDefaultSetter(bool, Exposed, m)
   };


   class ShaderProgramVertexParameter : public ShaderProgramParameter
   {
   public:
      ShaderProgramVertexParameter()
         : ShaderProgramParameter("ShaderProgramVertexParameter")
      {
      }
   };


   class ShaderProgramFragmentParameter : public ShaderProgramParameter
   {
   public:
      ShaderProgramFragmentParameter()
         : ShaderProgramParameter("ShaderProgramFragmentParameter")
      {
      }
   };


   class ShaderProgram : public Content::Resource
   {
   protected:
      char sVertexSource[32];
      char sFragmentSource[32];
      uint8_t eVertexElements;
      DepthBufferMode eDepthBufferMode;
      CullingMode eCullingMode;

   public:
      static const Core::ObjectName TypeName;

      ShaderProgram(const Core::ObjectName& Name, const Core::ObjectName& GroupName = Core::ObjectName::Empty);
      virtual ~ShaderProgram();

      const char* getVertexSource() const { return sVertexSource; }
      void setVertexSource(const char* Value) { strcpy(sVertexSource, Value); }

      const char* getFragmentSource() const { return sFragmentSource; }
      void setFragmentSource(const char* Value) { strcpy(sFragmentSource, Value); }

      uint8_t getVertexElements() const;
      void setVertexElements(uint8_t VertexElements);

      const DepthBufferMode getDepthBufferMode() const;
      void setDepthBufferMode(DepthBufferMode Mode);

      const CullingMode getCullingMode() const;
      void setCullingMode(CullingMode Mode);

      GEPropertyArray(ShaderProgramPreprocessorMacro, ShaderProgramPreprocessorMacro)
      GEPropertyArray(ShaderProgramVertexParameter, ShaderProgramVertexParameter)
      GEPropertyArray(ShaderProgramFragmentParameter, ShaderProgramFragmentParameter)
   };
}}
