
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

const ObjectName ShaderProgramName = ObjectName("ShaderProgram");

ShaderProgram::ShaderProgram(const ObjectName& Name, const ObjectName& GroupName)
   : SerializableResource(Name, GroupName, ShaderProgramName)
   , eDepthBufferMode(DepthBufferMode::NoDepth)
   , eCullingMode(CullingMode::Back)
{
   GERegisterPropertyReadonly(ObjectName, Name);
   GERegisterPropertyBitMask(VertexElementsBitMask, VertexElements);
   GERegisterPropertyEnum(DepthBufferMode, DepthBufferMode);
   GERegisterPropertyEnum(CullingMode, CullingMode);
}

ShaderProgram::~ShaderProgram()
{
}

uint8_t ShaderProgram::getVertexElements() const
{
   return eVertexElements;
}

void ShaderProgram::setVertexElements(uint8_t VertexElements)
{
   eVertexElements = VertexElements;
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
      const char* sParameterDefaultValue = xmlParameter.attribute("defaultValue").value();

      ValueType eValueType = Value::getValueType(sParameterType);
      uint iValueTypeSize = Value::getDefaultValue(eValueType).getSize();

      ObjectName cParameterName = ObjectName(sParameterName);
      ShaderProgramParameter sParameter = ShaderProgramParameter(cParameterName, eValueType);

      if(sParameterDefaultValue[0] != '\0')
      {
         sParameter.DefaultValue = Value(eValueType, sParameterDefaultValue);
      }

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
      ObjectName cParameterName = Value::fromStream(ValueType::ObjectName, sStream).getAsObjectName();
      ValueType eParameterType = (ValueType)Value::fromStream(ValueType::Byte, sStream).getAsByte();

      ShaderProgramParameter sParameter = ShaderProgramParameter(cParameterName, eParameterType);
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
