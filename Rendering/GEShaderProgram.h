
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
   struct ShaderProgramParameter
   {
      Core::ObjectName Name;
      Core::ValueType Type;
      uint Offset;
   };


   class ShaderProgram : public Content::Resource, public Core::Serializable
   {
   public:
      typedef GESTLVector(ShaderProgramParameter) ParameterList;

   protected:
      DepthBufferMode eDepthBufferMode;
      CullingMode eCullingMode;

      void parseParameters(const pugi::xml_node& xmlShader, const char* sTag, ParameterList* vParameterList);
      void parseParameters(std::istream& sStream, ParameterList* vParameterList);

   public:
      ParameterList VertexParameters;
      ParameterList FragmentParameters;

      ShaderProgram(const Core::ObjectName& Name);
      virtual ~ShaderProgram();

      const DepthBufferMode getDepthBufferMode() const;
      void setDepthBufferMode(DepthBufferMode Mode);

      const CullingMode getCullingMode() const;
      void setCullingMode(CullingMode Mode);

      uint getVertexElementsMask(const pugi::xml_node& xmlShader);
      void parseParameters(const pugi::xml_node& xmlShader);
      void parseParameters(std::istream& sStream);

      GEPropertyEnum(DepthBufferMode, DepthBufferMode)
      GEPropertyEnum(CullingMode, CullingMode)
   };
}}
