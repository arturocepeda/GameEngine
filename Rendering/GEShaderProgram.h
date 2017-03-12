
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
#include "Rendering/GERenderingObjects.h"

#include "Externals/pugixml/pugixml.hpp"

namespace GE { namespace Rendering
{
   struct ShaderProgramParameter
   {
      Core::ObjectName Name;
      Core::ValueType Type;
      uint Offset;
   };


   class ShaderProgram : public Core::Object, public Core::Serializable
   {
   public:
      typedef GESTLVector(ShaderProgramParameter) ParameterList;

   protected:
      DepthBufferMode eDepthBufferMode;

      void parseParameters(const pugi::xml_node& xmlShader, const char* sTag, ParameterList* vParameterList);

   public:
      ParameterList VertexParameters;
      ParameterList FragmentParameters;

      ShaderProgram(const Core::ObjectName& Name);
      virtual ~ShaderProgram();

      const DepthBufferMode getDepthBufferMode() const;
      void setDepthBufferMode(DepthBufferMode Mode);

      void parseParameters(const pugi::xml_node& xmlShader);

      GEPropertyEnum(DepthBufferMode, DepthBufferMode)
   };
}}
