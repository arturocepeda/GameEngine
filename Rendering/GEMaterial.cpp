
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering
//
//  --- GEMaterial.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEMaterial.h"
#include "GETexture.h"
#include "GERenderSystem.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Rendering;

//
//  Material
//
Material::Material(const ObjectName& Name)
   : Object(Name)
   , Serializable("Material")
   , cDiffuseColor(1.0f, 1.0f, 1.0f)
   , cSpecularColor(1.0f, 1.0f, 1.0f)
   , cDiffuseTexture(0)
   , eBlendingMode(BlendingMode::None)
   , bBatchRendering(false)
{
   GERegisterPropertyReadonly(Material, ObjectName, Name);
   GERegisterPropertyObjectManager(Material, ObjectName, ShaderProgram, ShaderProgram);
   GERegisterProperty(Material, Color, DiffuseColor);
   GERegisterPropertyObjectManager(Material, ObjectName, DiffuseTextureName, Texture);
   GERegisterPropertyEnum(Material, BlendingMode, BlendingMode);
   GERegisterProperty(Material, Bool, BatchRendering);
}

const ObjectName& Material::getShaderProgram() const
{
   return cShaderProgramName;
}

const Color& Material::getDiffuseColor() const
{
   return cDiffuseColor;
}

const float Material::getAlpha() const
{
   return cDiffuseColor.Alpha;
}

const Color& Material::getSpecularColor() const
{
   return cSpecularColor;
}

const Texture* Material::getDiffuseTexture() const
{
   return cDiffuseTexture;
}

const ObjectName& Material::getDiffuseTextureName() const
{
   return cDiffuseTextureName;
}

const BlendingMode Material::getBlendingMode() const
{
   return eBlendingMode;
}

bool Material::getBatchRendering() const
{
   return bBatchRendering;
}

void Material::setShaderProgram(const ObjectName& Name)
{
   if(cShaderProgramName == Name)
      return;

   cShaderProgramName = Name;
}

void Material::setDiffuseColor(const Color& DiffuseColor)
{
   cDiffuseColor = DiffuseColor;
}

void Material::setAlpha(float Alpha)
{
   cDiffuseColor.Alpha = Alpha;
}

void Material::setSpecularColor(const Color& SpecularColor)
{
   cSpecularColor = SpecularColor;
}

void Material::setDiffuseTexture(Texture* DiffuseTexture)
{
   cDiffuseTexture = DiffuseTexture;
}

void Material::setDiffuseTextureName(const ObjectName& Name)
{
   cDiffuseTextureName = Name;

   if(RenderSystem::getInstance())
   {
      cDiffuseTexture = RenderSystem::getInstance()->getTexture(Name);
      GEAssert(cDiffuseTexture);
   }
}

void Material::setBlendingMode(BlendingMode Mode)
{
   eBlendingMode = Mode;
}

void Material::setBatchRendering(bool Value)
{
   bBatchRendering = Value;
}


//
//  MaterialPass
//
const ObjectName ShadersObjectRegistryName = ObjectName("ShaderProgram");
const ObjectName MaterialObjectRegistryName = ObjectName("Material");

const ObjectName MaterialPass::EventMaterialSet = ObjectName("EventMaterialSet");

MaterialPass::MaterialPass()
   : EventHandlingObject("MaterialPass")
   , Serializable("MaterialPass")
   , cMaterial(0)
   , bActive(true)
   , sConstantBufferDataVertex(0)
   , sConstantBufferDataFragment(0)
{
   GERegisterPropertyObjectManager(MaterialPass, ObjectName, MaterialName, Material);
   GERegisterProperty(MaterialPass, Bool, Active);

   iBasePropertiesCount = getPropertiesCount();
}

MaterialPass::~MaterialPass()
{
   releaseConstantBufferData();
}

Material* MaterialPass::getMaterial()
{
   return cMaterial;
}

void MaterialPass::setMaterial(Material* M)
{
   if(cMaterial == M)
      return;

   cMaterial = M;

   while(getPropertiesCount() > iBasePropertiesCount)
      removeProperty(getPropertiesCount() - 1);

   releaseConstantBufferData();

   const ObjectRegistry* cShadersObjectRegistry = ObjectManagers::getInstance()->getObjectRegistry(ShadersObjectRegistryName);
   ShaderProgram* cShaderProgram = static_cast<ShaderProgram*>(cShadersObjectRegistry->find(cMaterial->getShaderProgram().getID())->second);
   GEAssert(cShaderProgram);

   if(cShaderProgram->VertexParameters.size() > 0)
   {
      sConstantBufferDataVertex = Allocator::alloc<char>(Material::ConstantBufferSize);
      registerShaderProperties(sConstantBufferDataVertex, cShaderProgram->VertexParameters);
   }

   if(cShaderProgram->FragmentParameters.size() > 0)
   {
      sConstantBufferDataFragment = Allocator::alloc<char>(Material::ConstantBufferSize);
      registerShaderProperties(sConstantBufferDataFragment, cShaderProgram->FragmentParameters);
   }

   EventArgs sEventArgs;
   sEventArgs.Sender = this;
   triggerEvent(EventMaterialSet, &sEventArgs);
}

void MaterialPass::registerShaderProperties(char* sBuffer, const ShaderProgram::ParameterList& vParameterList)
{
   for(uint i = 0; i < vParameterList.size(); i++)
   {
      const ShaderProgramParameter& sParameter = vParameterList[i];

      PropertySetter setter = [sBuffer, &sParameter](Value& cValue)
      {
         memcpy(sBuffer + sParameter.Offset, cValue.getRawData(), cValue.getSize());
      };
      PropertyGetter getter = [sBuffer, &sParameter]() -> Value
      {
         const uint iValueSize = Value::getDefaultValue(sParameter.Type).getSize();
         return Value::fromRawData(sParameter.Type, sBuffer + sParameter.Offset, iValueSize);
      };

      registerProperty(sParameter.Name, sParameter.Type, setter, getter);
   }
}

void MaterialPass::releaseConstantBufferData()
{
   if(sConstantBufferDataVertex)
   {
      Allocator::free(sConstantBufferDataVertex);
      sConstantBufferDataVertex = 0;
   }

   if(sConstantBufferDataFragment)
   {
      Allocator::free(sConstantBufferDataFragment);
      sConstantBufferDataFragment = 0;
   }
}

const Core::ObjectName& MaterialPass::getMaterialName() const
{
   return cMaterial ? cMaterial->getName() : ObjectName::Empty;
}

bool MaterialPass::getActive() const
{
   return bActive;
}

void MaterialPass::setMaterialName(const Core::ObjectName& Name)
{
   if(cMaterial && Name == cMaterial->getName())
      return;

   const ObjectRegistry* cMaterialObjectRegistry = ObjectManagers::getInstance()->getObjectRegistry(MaterialObjectRegistryName);
   GEAssert(cMaterialObjectRegistry->find(Name.getID()) != cMaterialObjectRegistry->end());
   setMaterial(static_cast<Material*>(cMaterialObjectRegistry->find(Name.getID())->second));
}

void MaterialPass::setActive(bool Active)
{
   bActive = Active;
}

bool MaterialPass::hasVertexParameters() const
{
   return sConstantBufferDataVertex != 0;
}

bool MaterialPass::hasFragmentParameters() const
{
   return sConstantBufferDataFragment != 0;
}

const char* MaterialPass::getConstantBufferDataVertex() const
{
   return sConstantBufferDataVertex;
}

const char* MaterialPass::getConstantBufferDataFragment() const
{
   return sConstantBufferDataFragment;
}

void MaterialPass::reload()
{
   if(!cMaterial)
      return;

   if(getPropertiesCount() == iBasePropertiesCount)
      return;

   while(getPropertiesCount() > iBasePropertiesCount)
      removeProperty(getPropertiesCount() - 1);

   const ObjectRegistry* cShadersObjectRegistry = ObjectManagers::getInstance()->getObjectRegistry(ShadersObjectRegistryName);
   ShaderProgram* cShaderProgram = static_cast<ShaderProgram*>(cShadersObjectRegistry->find(cMaterial->getShaderProgram().getID())->second);
   GEAssert(cShaderProgram);

   if(cShaderProgram->VertexParameters.size() > 0)
   {
      registerShaderProperties(sConstantBufferDataVertex, cShaderProgram->VertexParameters);
   }

   if(cShaderProgram->FragmentParameters.size() > 0)
   {
      registerShaderProperties(sConstantBufferDataFragment, cShaderProgram->FragmentParameters);
   }

   EventArgs sEventArgs;
   sEventArgs.Sender = this;
   triggerEvent(EventMaterialSet, &sEventArgs);
}
