
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEMaterial.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEObject.h"
#include "Core/GESerializable.h"
#include "Content/GEResource.h"
#include "GERenderingObjects.h"
#include "GEShaderProgram.h"

namespace GE { namespace Rendering
{
   class Texture;
   
   class Material : public Content::SerializableResource
   {
   public:
      static const uint ConstantBufferSize = 64;

   private:
      Core::ObjectName cShaderProgramName;
      Color cDiffuseColor;
      Color cSpecularColor;
      Core::ObjectName cDiffuseTextureName;
      Texture* cDiffuseTexture;
      BlendingMode eBlendingMode;
      bool bBatchRendering;

   public:
      Material(const Core::ObjectName& Name, const Core::ObjectName& GroupName);

      virtual uint getSizeInBytes() const override;

      const Core::ObjectName& getShaderProgram() const;
      const Color& getDiffuseColor() const;
      const float getAlpha() const;
      const Color& getSpecularColor() const;
      const Texture* getDiffuseTexture() const;
      const Core::ObjectName& getDiffuseTextureName() const;
      const BlendingMode getBlendingMode() const;
      bool getBatchRendering() const;

      void setShaderProgram(const Core::ObjectName& Name);
      void setDiffuseColor(const Color& DiffuseColor);
      void setAlpha(float Alpha);
      void setSpecularColor(const Color& SpecularColor);
      void setDiffuseTexture(Texture* DiffuseTexture);
      void setDiffuseTextureName(const Core::ObjectName& Name);
      void setBlendingMode(BlendingMode Mode);
      void setBatchRendering(bool Value);

      GEPropertyReadonly(ObjectName, Name)
      GEProperty(ObjectName, ShaderProgram)
      GEProperty(Color, DiffuseColor)
      GEProperty(ObjectName, DiffuseTextureName)
      GEPropertyEnum(BlendingMode, BlendingMode)
      GEProperty(Bool, BatchRendering)
   };


   GESerializableEnum(MaterialPassFlagsBitMask)
   {
      Active              = 1 << 0,
      RenderOncePerLight  = 1 << 1,

      Count = 2
   };


   class MaterialPass : public Core::SerializableArrayElement
   {
   private:
      Material* cMaterial;
      uint8_t eFlags;

      uint iBasePropertiesCount;

      char* sConstantBufferDataVertex;
      char* sConstantBufferDataFragment;

      void registerShaderProperties(char* sBuffer, const ShaderProgram::ParameterList& vParameterList);
      void releaseConstantBufferData();

   public:
      MaterialPass();
      ~MaterialPass();

      Material* getMaterial();
      const Core::ObjectName& getMaterialName() const;
      bool getActive() const;
      uint8_t getFlags() const;

      void setMaterial(Material* M);
      void setMaterialName(const Core::ObjectName& Name);
      void setActive(bool Active);
      void setFlags(uint8_t Flags);

      bool hasVertexParameters() const;
      bool hasFragmentParameters() const;

      const char* getConstantBufferDataVertex() const;
      const char* getConstantBufferDataFragment() const;

      void reload();

      GEProperty(ObjectName, MaterialName);
      GEPropertyBitMask(MaterialPassFlagsBitMask, Flags);
   };
}}
