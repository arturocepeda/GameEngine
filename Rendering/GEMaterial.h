
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


   GESerializableEnum(MaterialFlagsBitMask)
   {
      RenderOncePerLight = 1 << 0,
      BatchRendering     = 1 << 1,

      Count = 2
   };

   
   class Material : public Content::Resource
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
      uint8_t eFlags;

   public:
      static const Core::ObjectName TypeName;

      Material(const Core::ObjectName& Name, const Core::ObjectName& GroupName);

      virtual uint getSizeInBytes() const override;

      const Core::ObjectName& getShaderProgram() const;
      const Color& getDiffuseColor() const;
      const float getAlpha() const;
      const Color& getSpecularColor() const;
      const Texture* getDiffuseTexture() const;
      const Core::ObjectName& getDiffuseTextureName() const;
      const BlendingMode getBlendingMode() const;
      uint8_t getFlags() const;

      void setShaderProgram(const Core::ObjectName& Name);
      void setDiffuseColor(const Color& DiffuseColor);
      void setAlpha(float Alpha);
      void setSpecularColor(const Color& SpecularColor);
      void setDiffuseTexture(Texture* DiffuseTexture);
      void setDiffuseTextureName(const Core::ObjectName& Name);
      void setBlendingMode(BlendingMode Mode);
      void setFlags(uint8_t Flags);
   };


   class MaterialPass : public Core::SerializableArrayElement
   {
   private:
      Material* cMaterial;
      bool bActive;

      uint iBasePropertiesCount;

      char* sConstantBufferDataVertex;
      char* sConstantBufferDataFragment;

      void registerShaderProperties(char* sBuffer, const Core::PropertyArrayEntries& vParameterList);
      void releaseConstantBufferData();

   public:
      MaterialPass();
      ~MaterialPass();

      Material* getMaterial();
      const Core::ObjectName& getMaterialName() const;
      bool getActive() const;

      void setMaterial(Material* M);
      void setMaterialName(const Core::ObjectName& Name);
      void setActive(bool Active);

      bool hasVertexParameters() const;
      bool hasFragmentParameters() const;

      const char* getConstantBufferDataVertex() const;
      const char* getConstantBufferDataFragment() const;

      void reload();
   };
}}
