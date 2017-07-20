
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

using namespace GE;
using namespace GE::Rendering;
using namespace GE::Core;
using namespace GE::Content;

ShaderProgram::ShaderProgram(const ObjectName& Name)
   : Resource(Name, ObjectName::Empty, ResourceType::ShaderProgram)
   , Serializable("ShaderProgram")
   , eDepthBufferMode(DepthBufferMode::NoDepth)
   , eCullingMode(CullingMode::Back)
{
   GERegisterPropertyReadonly(ObjectName, Name);
   GERegisterPropertyEnum(DepthBufferMode, DepthBufferMode);
   GERegisterPropertyEnum(CullingMode, CullingMode);
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

const CullingMode ShaderProgram::getCullingMode() const
{
   return eCullingMode;
}

void ShaderProgram::setCullingMode(CullingMode Mode)
{
   eCullingMode = Mode;
}

void ShaderProgram::parsePreprocessorMacros(const pugi::xml_node& xmlShader)
{
   for(const pugi::xml_node& xmlParameter : xmlShader.children("PreprocessorMacro"))
   {
      ShaderProgramPreprocessorMacro sPreprocessorMacro;
      strcpy(sPreprocessorMacro.Name, xmlParameter.attribute("name").value());
      strcpy(sPreprocessorMacro.Value, xmlParameter.attribute("value").value());
      PreprocessorMacros.push_back(sPreprocessorMacro);
   }
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

void ShaderProgram::parseParameters(std::istream& sStream, ParameterList* vParameterList)
{
   uint iParameterCount = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();

   for(uint i = 0; i < iParameterCount; i++)
   {
      ShaderProgramParameter sParameter;
      sParameter.Name = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
      sParameter.Type = (ValueType)Value::fromStream(ValueType::Byte, sStream).getAsByte();
      sParameter.Offset = (uint)Value::fromStream(ValueType::Byte, sStream).getAsByte();
      vParameterList->push_back(sParameter);
   }
}

void ShaderProgram::parseParameters(const pugi::xml_node& xmlShader)
{
   parseParameters(xmlShader, "VertexParameter", &VertexParameters);
   parseParameters(xmlShader, "FragmentParameter", &FragmentParameters);
}

void ShaderProgram::parseParameters(std::istream& sStream)
{
   parseParameters(sStream, &VertexParameters);
   parseParameters(sStream, &FragmentParameters);
}

uint ShaderProgram::getVertexElementsMask(const pugi::xml_node& xmlShader)
{
   uint iVertexElements = 0;

   for(const pugi::xml_node& xmlVertexElement : xmlShader.children("VertexElement"))
   {
      const char* sVertexElement = xmlVertexElement.attribute("name").value();

      if(strcmp(sVertexElement, "Position") == 0)
         iVertexElements |= VE_Position;
      else if(strcmp(sVertexElement, "Color") == 0)
         iVertexElements |= VE_Color;
      else if(strcmp(sVertexElement, "Normal") == 0)
         iVertexElements |= VE_Normal;
      else if(strcmp(sVertexElement, "TexCoord") == 0)
         iVertexElements |= VE_TexCoord;
      else if(strcmp(sVertexElement, "WorldViewProjection") == 0)
         iVertexElements |= VE_WVP;
   }

   return iVertexElements;
}
