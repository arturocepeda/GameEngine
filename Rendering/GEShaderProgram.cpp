
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
   sVertexSource[0] = '\0';
   sFragmentSource[0] = '\0';

   GERegisterPropertyReadonly(ObjectName, Name);
   GERegisterProperty(String, VertexSource);
   GERegisterProperty(String, FragmentSource);
   GERegisterPropertyBitMask(VertexElementsBitMask, VertexElements);
   GERegisterPropertyEnum(DepthBufferMode, DepthBufferMode);
   GERegisterPropertyEnum(CullingMode, CullingMode);
   GERegisterPropertyArray(ShaderProgramPreprocessorMacro);
   GERegisterPropertyArray(ShaderProgramVertexParameter);
   GERegisterPropertyArray(ShaderProgramFragmentParameter);
}

ShaderProgram::~ShaderProgram()
{
   GEReleasePropertyArray(ShaderProgramPreprocessorMacro);
   GEReleasePropertyArray(ShaderProgramVertexParameter);
   GEReleasePropertyArray(ShaderProgramFragmentParameter);
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
