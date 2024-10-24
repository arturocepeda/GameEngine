
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Rendering Engine (OpenGL ES)
//
//  --- GERenderingShadersES20.cpp ---
//
//////////////////////////////////////////////////////////////////


#include "GERenderingShadersES20.h"
#include "Core/GEDevice.h"
#include "Core/GELog.h"
#include "Core/GEAllocator.h"

#include <cstring>

#include "GEOpenGLES20.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Rendering;

//
//  Shader
//
Shader::~Shader()
{
   glDeleteShader(iID);
}

void Shader::load(const char* sFilename, const char* sExt, ShaderProgram* cShaderProgram)
{
   // read source file
   ContentData cShader;
   Device::readContentFile(ContentType::Shader, "Shaders/glsl", sFilename, sExt, &cShader);

   GESTLString sShaderSource(cShader.getData(), cShader.getDataSize());

   // add preprocessor macros
   const PropertyArrayEntries& vMacros = cShaderProgram->vShaderProgramPreprocessorMacroList;

   for(uint i = 0; i < vMacros.size(); i++)
   {
      const ShaderProgramPreprocessorMacro* cMacro = static_cast<const ShaderProgramPreprocessorMacro*>(vMacros[i]);
      char sBuffer[256];
      sprintf(sBuffer, "#define %s %s\n", cMacro->getName(), cMacro->getValue());
      sShaderSource.insert(0, sBuffer);
   }

   // process include directives
   const char* IncludeStr = "#include \"";
   const size_t IncludeStrLength = strlen(IncludeStr);

   size_t iIncludePosition = sShaderSource.find(IncludeStr);

   while(iIncludePosition != std::string::npos)
   {
      size_t iIncludedFileNamePositionStart = iIncludePosition + IncludeStrLength;
      size_t iIncludedFileNamePositionEnd = sShaderSource.find('"', iIncludedFileNamePositionStart);
      size_t iIncludedFileNameLength = iIncludedFileNamePositionEnd - iIncludedFileNamePositionStart;

      GESTLString sIncludedFileName = sShaderSource.substr(iIncludedFileNamePositionStart, iIncludedFileNameLength);
      GESTLString sIncludedFileNameWithoutExtension = sIncludedFileName.substr(0, sIncludedFileName.find('.'));

      ContentData cIncludedShader;
      Device::readContentFile(ContentType::Shader, "Shaders/glsl", sIncludedFileNameWithoutExtension.c_str(), sExt, &cIncludedShader);

      GESTLString sIncludedShaderSource(cIncludedShader.getData(), cIncludedShader.getDataSize());
      sShaderSource.replace(iIncludePosition, IncludeStrLength + iIncludedFileNameLength + 2, sIncludedShaderSource.c_str());

      iIncludePosition = sShaderSource.find(IncludeStr);
   }

   const char* sShaderSourceArray[1];
   sShaderSourceArray[0] = sShaderSource.c_str();
   const char** sShaderSourceAsDoublePtr = reinterpret_cast<const char**>(sShaderSourceArray);
   glShaderSource(iID, 1, sShaderSourceAsDoublePtr, 0);
   
   // compile shader
   glCompileShader(iID);
   glGetShaderiv(iID, GL_COMPILE_STATUS, &iStatus);
   
   if(!check())
   {
      // get info log length
      GLint iLogLength;
      glGetShaderiv(iID, GL_INFO_LOG_LENGTH, &iLogLength);
      
      // get info log contents
      GLchar* sLog = Allocator::alloc<GLchar>(iLogLength + 1);
      glGetShaderInfoLog(iID, iLogLength, NULL, sLog);
      sLog[iLogLength] = '\0';
      
      // show info log
      Log::log(LogType::Error, "Shader '%s.%s': compiling error\n%s", sFilename, sExt, sLog);
      
      Allocator::free(sLog);
   }
}

void Shader::load(const char* sData, int iDataSize)
{
   const char* sShaderSourceArray[1];
   sShaderSourceArray[0] = sData;
   const char** sShaderSourceAsDoublePtr = reinterpret_cast<const char**>(sShaderSourceArray);
   glShaderSource(iID, 1, sShaderSourceAsDoublePtr, 0);

   // compile shader
   glCompileShader(iID);
   glGetShaderiv(iID, GL_COMPILE_STATUS, &iStatus);
}

uint Shader::getID()
{
   return iID;
}

bool Shader::check()
{
   return (iStatus != 0);
}


//
//  VertexShader
//
VertexShader::VertexShader(ShaderProgram* cShaderProgram)
{
   // get shader ID
   iID = glCreateShader(GL_VERTEX_SHADER);
   iStatus = 0;
   
   // load shader
   load(cShaderProgram->getVertexSource(), "vsh", cShaderProgram);
}

VertexShader::VertexShader(const char* Data, int DataSize)
{
   // get shader ID
   iID = glCreateShader(GL_VERTEX_SHADER);
   iStatus = 0;

   // load shader
   load(Data, DataSize);
}


//
//  FragmentShader
//
FragmentShader::FragmentShader(ShaderProgram* cShaderProgram)
{
   // get shader ID
   iID = glCreateShader(GL_FRAGMENT_SHADER);
   iStatus = 0;
   
   // load shader
   load(cShaderProgram->getFragmentSource(), "fsh", cShaderProgram);
}

FragmentShader::FragmentShader(const char* Data, int DataSize)
{
   // get shader ID
   iID = glCreateShader(GL_FRAGMENT_SHADER);
   iStatus = 0;

   // load shader
   load(Data, DataSize);
}


//
//  ShaderProgramES20
//
ShaderProgramES20::ShaderProgramES20(const ObjectName& Name)
   : ShaderProgram(Name)
   , ID(0)
   , Status(0)
   , VS(0)
   , FS(0)
{
}

ShaderProgramES20::~ShaderProgramES20()
{
   GEInvokeDtor(VertexShader, VS);
   Core::Allocator::free(VS);
   GEInvokeDtor(FragmentShader, FS);
   Core::Allocator::free(FS);
}

uint ShaderProgramES20::getUniformLocation(uint UniformIndex) const
{
   return iUniforms[UniformIndex];
}

void ShaderProgramES20::setUniformLocation(uint UniformIndex, uint UniformLocation)
{
   iUniforms[UniformIndex] = UniformLocation;
}
