
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
#include "Core/GELog.h"
#include "Core/GEEvents.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;
using namespace GE::Rendering;

//
//  Material
//
const ObjectName Material::TypeName = ObjectName("Material");

Material::Material(const ObjectName& Name, const ObjectName& GroupName)
   : Resource(Name, GroupName, TypeName)
   , cDiffuseColor(1.0f, 1.0f, 1.0f)
   , cSpecularColor(1.0f, 1.0f, 1.0f)
   , cDiffuseTexture(0)
   , eBlendingMode(BlendingMode::None)
   , eFlags(0)
{
   GERegisterPropertyReadonly(ObjectName, Name);
   GERegisterProperty(ObjectName, ShaderProgram);
   GERegisterProperty(Color, DiffuseColor);
   GERegisterProperty(ObjectName, DiffuseTextureName);
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
         Log::log(LogType::Warning, "Texture not found: '%s' (Referenced in Material: '%s')",
            Name.getString(), cName.getString());
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
   GERegisterProperty(ObjectName, MaterialName);
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

      if(cShaderProgram->getShaderProgramVertexParameterCount() > 0)
      {
         sConstantBufferDataVertex = Allocator::alloc<char>(Material::ConstantBufferSize);
         registerShaderProperties(sConstantBufferDataVertex, cShaderProgram->vShaderProgramVertexParameterList);
      }

      if(cShaderProgram->getShaderProgramFragmentParameterCount() > 0)
      {
         sConstantBufferDataFragment = Allocator::alloc<char>(Material::ConstantBufferSize);
         registerShaderProperties(sConstantBufferDataFragment, cShaderProgram->vShaderProgramFragmentParameterList);
      }
   }

#if defined (GE_EDITOR_SUPPORT)
   EventArgs sArgs;
   sArgs.Data = cOwner;
   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
#endif
}

void MaterialPass::registerShaderProperties(char* sBuffer, const PropertyArrayEntries& vParameterList)
{
   uint iParameterOffset = 0;

   for(uint i = 0; i < vParameterList.size(); i++)
   {
      const ShaderProgramParameter* cParameter = static_cast<const ShaderProgramParameter*>(vParameterList[i]);
      const ValueType eParameterType = cParameter->getType();

      PropertySetter setter = [sBuffer, iParameterOffset](const Value& cValue)
      {
         memcpy(sBuffer + iParameterOffset, cValue.getRawData(), cValue.getSize());
      };
      PropertyGetter getter = [sBuffer, eParameterType, iParameterOffset]() -> Value
      {
         const uint iValueSize = Value::getDefaultValue(eParameterType).getSize();
         return Value::fromRawData(eParameterType, sBuffer + iParameterOffset, iValueSize);
      };

      setter(cParameter->getValue());
      registerProperty(cParameter->getName(), cParameter->getType(), setter, getter);

      iParameterOffset += Value::getDefaultValue(eParameterType).getSize();
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
   const ObjectRegistry* cMaterialObjectRegistry =
      ResourcesManager::getInstance()->getObjectRegistry(MaterialObjectRegistryName);
   ObjectRegistry::const_iterator it = cMaterialObjectRegistry->find(Name.getID());
   
   if(it != cMaterialObjectRegistry->end())
   {
      cNewMaterial = static_cast<Material*>(it->second);
   }
   else
   {
      Log::log(LogType::Warning, "Material not found: '%s'", Name.getString());
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

   if(cShaderProgram->getShaderProgramVertexParameterCount() > 0)
   {
      registerShaderProperties(sConstantBufferDataVertex, cShaderProgram->vShaderProgramVertexParameterList);
   }

   if(cShaderProgram->getShaderProgramFragmentParameterCount() > 0)
   {
      registerShaderProperties(sConstantBufferDataFragment, cShaderProgram->vShaderProgramFragmentParameterList);
   }

#if defined (GE_EDITOR_SUPPORT)
   EventArgs sArgs;
   sArgs.Data = cOwner;
   EventHandlingObject::triggerEventStatic(Events::PropertiesUpdated, &sArgs);
#endif
}
