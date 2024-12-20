
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda P�rez
//  Game Engine
//
//  Rendering Engine (OpenGL ES)
//
//  --- GERenderingShadersES20.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Types/GETypeDefinitions.h"
#include "Core/GEObject.h"
#include "Core/GEAllocator.h"
#include "Rendering/GERenderingObjects.h"
#include "Rendering/GEShaderProgram.h"

namespace GE { namespace Rendering
{
   enum class VertexAttributes
   {
      Position,
      TextureCoord0,
      Normal,
      Color,

      Count
   };

   enum class Uniforms
   {
      // Transform
      WorldViewProjectionMatrix, 
      WorldMatrix,
      InverseTransposeWorldMatrix,
      LightWorldViewProjectionMatrix,
      ViewProjectionMatrix,

      // Material
      DiffuseColor,
      SpecularColor,
      DiffuseTexture,

      // Lighting
      ShadowTexture,
      AmbientLightColor,
      LightColor,
      EyePosition,
      LightType,
      LightPosition,
      Attenuation,
      LightDirection,
      SpotAngle,
      ShadowIntensity,

      // Custom Parameters
      VertexParameters,
      FragmentParameters,

      Count
   };


   //
   //  Shader
   //
   class Shader
   {
   protected:
      GE::uint iID;
      int iStatus;
   
      void load(const char* sFilename, const char* sExt, ShaderProgram* cShaderProgram);
      void load(const char* sData, int iDataSize);
   
   public:
      ~Shader();
   
      bool check();
      GE::uint getID();
   };


   class VertexShader : public Shader
   {
   public:
      VertexShader(ShaderProgram* cShaderProgram);
      VertexShader(const char* Data, int DataSize);
   };


   class FragmentShader : public Shader
   {
   public:
      FragmentShader(ShaderProgram* cShaderProgram);
      FragmentShader(const char* Data, int DataSize);
   };


   //
   //  ShaderProgramES20
   //
   class ShaderProgramES20 : public ShaderProgram
   {
   private:
      uint iUniforms[(int)Uniforms::Count];

   public:
      GE::uint ID;
      int Status;

      VertexShader* VS;
      FragmentShader* FS;

      ShaderProgramES20(const Core::ObjectName& Name);
      ~ShaderProgramES20();

      uint getUniformLocation(uint UniformIndex) const;
      void setUniformLocation(uint UniformIndex, uint UniformLocation);
   };
}}
