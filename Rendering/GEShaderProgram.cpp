
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEShaderProgram.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEShaderProgram.h"

using namespace GE::Rendering;
using namespace GE::Core;

ShaderProgram::ShaderProgram(const ObjectName& Name)
   : Object(Name)
   , Serializable("ShaderProgram")
   , eDepthBufferMode(DepthBufferMode::NoDepth)
{
   GERegisterPropertyEnum(ShaderProgram, DepthBufferMode, DepthBufferMode);
}

ShaderProgram::~ShaderProgram()
{
}

const DepthBufferMode ShaderProgram::getDepthBufferMode() const
{
   return eDepthBufferMode;
}

void ShaderProgram::setDepthBufferMode(DepthBufferMode Mode)
{
   eDepthBufferMode = Mode;
}

void ShaderProgram::parseParameters(const pugi::xml_node& xmlShader, const char* sTag, ParameterList* vParameterList)
{
   uint iOffset = 0;

   for(const pugi::xml_node& xmlParameter : xmlShader.children(sTag))
   {
      const char* sParameterName = xmlParameter.attribute("name").value();
      const char* sParameterType = xmlParameter.attribute("type").value();

      ValueType eValueType = Value::getValueType(sParameterType);
      uint iValueTypeSize = Value::getDefaultValue(eValueType).getSize();

      ShaderProgramParameter sParameter;
      sParameter.Name = ObjectName(sParameterName);
      sParameter.Type = eValueType;
      sParameter.Offset = iOffset;
      vParameterList->push_back(sParameter);

      iOffset += iValueTypeSize;
   }
}

void ShaderProgram::parseParameters(const pugi::xml_node& xmlShader)
{
   parseParameters(xmlShader, "VertexParameter", &VertexParameters);
   parseParameters(xmlShader, "FragmentParameter", &FragmentParameters);
}
