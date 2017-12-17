
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
#include "Content/GEResourcesManager.h"
#include "Core/GEDevice.h"
#include "Core/GEEvents.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Rendering;

//
//  Material
//
const ObjectName MaterialName = ObjectName("Material");

Material::Material(const ObjectName& Name, const ObjectName& GroupName)
   : SerializableResource(Name, GroupName, MaterialName)
   , cDiffuseColor(1.0f, 1.0f, 1.0f)
   , cSpecularColor(1.0f, 1.0f, 1.0f)
   , cDiffuseTexture(0)
   , eBlendingMode(BlendingMode::None)
   , eFlags(0)
{
   GERegisterPropertyReadonly(ObjectName, Name);
   GERegisterPropertyResource(ObjectName, ShaderProgram, ShaderProgram);
   GERegisterProperty(Color, DiffuseColor);
   GERegisterPropertyResource(ObjectName, DiffuseTextureName, Texture);
   GERegisterPropertyEnum(BlendingMode, BlendingMode);
   GERegisterPropertyBitMask(MaterialFlagsBitMask, Flags);
}

uint Material::getSizeInBytes() const
{
   return sizeof(Material);
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

uint8_t Material::getFlags() const
{
   return eFlags;
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

      if(!cDiffuseTexture)
      {
         Device::log("Texture not found: '%s' (Referenced in Material: '%s')",
            Name.getString().c_str(), cName.getString().c_str());
      }
   }
}

void Material::setBlendingMode(BlendingMode Mode)
{
   eBlendingMode = Mode;
}

void Material::setFlags(uint8_t Flags)
{
   eFlags = Flags;
}


//
//  MaterialPass
//
const ObjectName ShadersObjectRegistryName = ObjectName("ShaderProgram");
const ObjectName MaterialObjectRegistryName = ObjectName("Material");

MaterialPass::MaterialPass()
   : SerializableArrayElement("MaterialPass")
   , cMaterial(0)
   , bActive(true)
   , sConstantBufferDataVertex(0)
   , sConstantBufferDataFragment(0)
{
   GERegisterPropertyResource(ObjectName, MaterialName, Material);
   GERegisterProperty(Bool, Active);

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

   if(cMaterial)
   {
      const ObjectRegistry* cShadersObjectRegistry = ResourcesManager::getInstance()->getObjectRegistry(ShadersObjectRegistryName);
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
   }

   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated);
}

void MaterialPass::registerShaderProperties(char* sBuffer, const ShaderProgram::ParameterList& vParameterList)
{
   for(uint i = 0; i < vParameterList.size(); i++)
   {
      const ShaderProgramParameter& sParameter = vParameterList[i];

      PropertySetter setter = [sBuffer, &sParameter](const Value& cValue)
      {
         memcpy(sBuffer + sParameter.Offset, cValue.getRawData(), cValue.getSize());
      };
      PropertyGetter getter = [sBuffer, &sParameter]() -> Value
      {
         const uint iValueSize = Value::getDefaultValue(sParameter.Type).getSize();
         return Value::fromRawData(sParameter.Type, sBuffer + sParameter.Offset, iValueSize);
      };

      registerProperty(sParameter.Name, sParameter.Type, setter, getter);
      set(sParameter.Name, sParameter.DefaultValue);
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

   Material* cNewMaterial = 0;
   const ObjectRegistry* cMaterialObjectRegistry = ResourcesManager::getInstance()->getObjectRegistry(MaterialObjectRegistryName);
   
   if(cMaterialObjectRegistry->find(Name.getID()) != cMaterialObjectRegistry->end())
   {
      cNewMaterial = static_cast<Material*>(cMaterialObjectRegistry->find(Name.getID())->second);
   }
   else
   {
      Device::log("Material not found: '%s'", Name.getString().c_str());
   }

   setMaterial(cNewMaterial);
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

   const ObjectRegistry* cShadersObjectRegistry = ResourcesManager::getInstance()->getObjectRegistry(ShadersObjectRegistryName);
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

   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated);
}
