
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Tools
//
//  --- GEContentCompiler.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEContentCompiler.h"

#if defined (GE_EDITOR_SUPPORT)

#include "Core/GEAllocator.h"
#include "Core/GEValue.h"
#include "Core/GEParser.h"
#include "Core/GEObject.h"
#include "Core/GELog.h"
#include "Core/GETime.h"
#include "Rendering/GERenderSystem.h"
#include "Entities/GEEntity.h"
#include "Entities/GEComponent.h"
#include "Content/GEResourcesManager.h"
#include "Audio/GEAudioBank.h"
#include "Audio/GEAudioSystem.h"
#include "Types/GECurve.h"
#include "Scripting/GEScriptingEnvironment.h"

#include "Externals/pugixml/pugixml.hpp"
#include <D3DCompiler.h>

#pragma comment(lib, "D3DCompiler.lib")

using namespace GE;
using namespace GE::Tools;
using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Entities;
using namespace GE::Content;
using namespace GE::Audio;
using namespace pugi;

const char* ContentCompiler::ContentXmlDirName = ".";
const char* ContentCompiler::ContentBinDirName = "..\\contentBin";

const bool StripLuaSymbols = true;

std::function<void()> ContentCompiler::packAppResources = nullptr;

ObjectManager<ShaderProgram> mManagerShaderPrograms;
ObjectManager<Texture> mManagerTextures;
ObjectManager<Material> mManagerMaterials;
ObjectManager<Font> mManagerFonts;

Scene cDummyScene(ObjectName("Dummy"));


class ContentCompilerConfig : public Serializable
{
private:
   bool mIncludeShadersDirectX;
   bool mIncludeShadersOpenGL;

   char mTextureFilesExtension[8];
   char mAudioFilesExtension[8];

public:
   ContentCompilerConfig()
      : Serializable("ContentCompilerConfig")
      , mIncludeShadersDirectX(true)
      , mIncludeShadersOpenGL(true)
   {
      strcpy(mTextureFilesExtension, ".dds");
      strcpy(mAudioFilesExtension, ".ogg");

      GERegisterProperty(Bool, IncludeShadersDirectX);
      GERegisterProperty(Bool, IncludeShadersOpenGL);
      GERegisterProperty(String, TextureFilesExtension);
      GERegisterProperty(String, AudioFilesExtension);
   }
   ~ContentCompilerConfig()
   {
   }

   GEDefaultGetter(bool, IncludeShadersDirectX, m)
   GEDefaultGetter(bool, IncludeShadersOpenGL, m)
   GEDefaultGetter(const char*, TextureFilesExtension, m)
   GEDefaultGetter(const char*, AudioFilesExtension, m)

   GEDefaultSetter(bool, IncludeShadersDirectX, m)
   GEDefaultSetter(bool, IncludeShadersOpenGL, m)
   GEDefaultSetterString(const char*, TextureFilesExtension, m)
   GEDefaultSetterString(const char*, AudioFilesExtension, m)
};


void registerCompilerObjectManagers()
{
   ResourcesManager::getInstance()->registerObjectManager<ShaderProgram>(ShaderProgram::TypeName, &mManagerShaderPrograms);
   ResourcesManager::getInstance()->registerObjectManager<Texture>(Texture::TypeName, &mManagerTextures);
   ResourcesManager::getInstance()->registerObjectManager<Material>(Material::TypeName, &mManagerMaterials);  
   ResourcesManager::getInstance()->registerObjectManager<Font>(Font::TypeName, &mManagerFonts);
}

void packEntity(const ObjectName& cName, const pugi::xml_node& xmlNode, std::ofstream& sOutputFile, Entity* cParent)
{
   Value(cName).writeToStream(sOutputFile);

   Entity* cEntity = cDummyScene.addEntity(cName, cParent);
   cEntity->xmlToStream(xmlNode, sOutputFile);

   xml_object_range<xml_named_node_iterator> xmlComponents = xmlNode.children("Component");
   GE::byte iComponentsCount = 0u;

   for(const pugi::xml_node& xmlComponent : xmlComponents)
   {
      iComponentsCount++;
   }

   Value(iComponentsCount).writeToStream(sOutputFile);

   for(const pugi::xml_node& xmlComponent : xmlComponents)
   {
      const char* sComponentType = xmlComponent.attribute("type").value();
      ObjectName cComponentTypeName = ObjectName(sComponentType);

      Value(cComponentTypeName.getID()).writeToStream(sOutputFile);

      Component* cComponent = cEntity->addComponent(cComponentTypeName);
      cComponent->xmlToStream(xmlComponent, sOutputFile);
   }

   xml_object_range<xml_named_node_iterator> xmlChildren = xmlNode.children("Entity");
   GE::byte iChildrenCount = 0u;

   for(const pugi::xml_node& xmlChild : xmlChildren)
   {
      iChildrenCount++;
   }

   Value(iChildrenCount).writeToStream(sOutputFile);

   for(const pugi::xml_node& xmlChild : xmlChildren)
   {
      ObjectName cChildName = ObjectName(xmlChild.attribute("name").value());
      packEntity(cChildName, xmlChild, sOutputFile, cEntity);
   }

   cDummyScene.removeEntityImmediately(cName);
}

void ContentCompiler::getBinFileName(const char* pXmlFileName, char* pBinFileName)
{
   const size_t xmlFileNameLength = strlen(pXmlFileName);
   strcpy(pBinFileName, pXmlFileName);
   pBinFileName[xmlFileNameLength - 3u] = 'g';
   pBinFileName[xmlFileNameLength - 2u] = 'e';
   pBinFileName[xmlFileNameLength - 1u] = '\0';
}

void ContentCompiler::packShaders(ApplicationRenderingAPI pRenderingAPI)
{
   Log::log(LogType::Info, "Packing shaders %s...", pRenderingAPI == ApplicationRenderingAPI::DirectX ? "(DirectX)" : "(OpenGL)");

   pugi::xml_document xml;
   xml.load_file(L"Shaders\\Shaders.xml");
   const pugi::xml_node& xmlShaders = xml.child("ShaderProgramList");
   uint32_t iShadersCount = 0;

   for(const pugi::xml_node& xmlShader : xmlShaders.children("ShaderProgram"))
   {
      iShadersCount++;
   }

   char sOutputPath[MAX_PATH];
   GetCurrentDirectory(MAX_PATH, sOutputPath);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
   size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\Shaders", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
   toHashPath(sOutputPath + outputPathOffset);
   outputPathOffset = strlen(sOutputPath) + 1u;
#endif
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\Shaders.%s.ge", sOutputPath, (pRenderingAPI == ApplicationRenderingAPI::DirectX ? "hlsl" : "glsl"));
#if GE_HASH_OUTPUT_PATHS
   toHashPath(sOutputPath + outputPathOffset);
#endif

   std::string sShaderSource;
   std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
   GEAssert(sOutputFile.is_open());

   Value((GE::byte)iShadersCount).writeToStream(sOutputFile);

   for(const pugi::xml_node& xmlShader : xmlShaders.children("ShaderProgram"))
   {
      const char* sShaderProgramName = xmlShader.attribute("name").value();
      ObjectName cShaderProgramName = ObjectName(sShaderProgramName);

      Value(cShaderProgramName).writeToStream(sOutputFile);

      ShaderProgram* cShaderProgram = mManagerShaderPrograms.get(cShaderProgramName);

      if(!cShaderProgram)
      {
         cShaderProgram = new ShaderProgram(cShaderProgramName);
         cShaderProgram->loadFromXml(xmlShader);

         mManagerShaderPrograms.add(cShaderProgram);
      }

      cShaderProgram->saveToStream(sOutputFile);

      const char* sShaderName[2];
      sShaderName[0] = cShaderProgram->getVertexSource();
      sShaderName[1] = cShaderProgram->getFragmentSource();

      for(uint32_t i = 0; i < 2; i++)
      {
         char* pShaderByteCodeData = 0;
         uint32_t iShaderByteCodeSize = 0;

         if(pRenderingAPI == ApplicationRenderingAPI::DirectX)
         {
            //
            //  HLSL
            //
            const char* sShaderType[2] = { "vsh", "psh" };
            const char* sShaderTarget[2] = { "vs_5_0", "ps_5_0" };

            char sInputPath[MAX_PATH];
            sprintf(sInputPath, "%s\\Shaders\\hlsl\\%s.%s.hlsl", ContentXmlDirName, sShaderName[i], sShaderType[i]);

            wchar_t wsInputPath[MAX_PATH];
            mbstowcs(wsInputPath, sInputPath, strlen(sInputPath) + 1);

            const PropertyArrayEntries& vMacros = cShaderProgram->vShaderProgramPreprocessorMacroList;
            D3D_SHADER_MACRO* dxDefines = 0;

            if(!vMacros.empty())
            {
               dxDefines = new D3D_SHADER_MACRO[vMacros.size() + 1];

               for(uint32_t i = 0; i < vMacros.size(); i++)
               {
                  const ShaderProgramPreprocessorMacro* cMacro = static_cast<const ShaderProgramPreprocessorMacro*>(vMacros[i]);
                  dxDefines[i].Name = cMacro->getName();
                  dxDefines[i].Definition = cMacro->getValue();
               }

               dxDefines[vMacros.size()].Name = 0;
               dxDefines[vMacros.size()].Definition = 0;
            }

            ID3DBlob* dxCodeBlob = 0;
            ID3DBlob* dxErrorBlob = 0;
            HRESULT hr = D3DCompileFromFile(wsInputPath, dxDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", sShaderTarget[i], 0, 0, &dxCodeBlob, &dxErrorBlob);

            if(!vMacros.empty())
            {
               delete[] dxDefines;
               dxDefines = 0;
            }

            if(FAILED(hr))
            {
               if(dxErrorBlob)
               {
                  OutputDebugStringA((char*)dxErrorBlob->GetBufferPointer());
                  dxErrorBlob->Release();
               }

               GEAssert(false);
            }

            pShaderByteCodeData = (char*)dxCodeBlob->GetBufferPointer();
            iShaderByteCodeSize = (uint32_t)dxCodeBlob->GetBufferSize();
         }
         else
         {
            //
            //  GLSL
            //
            const char* sShaderType[2] = { "vsh", "fsh" };

            char sInputPath[MAX_PATH];
            sprintf(sInputPath, "%s\\Shaders\\glsl\\%s.%s", ContentXmlDirName, sShaderName[i], sShaderType[i]);

            std::ifstream sShaderFile(sInputPath);
            sShaderSource = std::string((std::istreambuf_iterator<char>(sShaderFile)), std::istreambuf_iterator<char>());
            sShaderFile.close();

            // add preprocessor macros
            const PropertyArrayEntries& vMacros = cShaderProgram->vShaderProgramPreprocessorMacroList;

            for(uint32_t i = 0; i < vMacros.size(); i++)
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

               std::string sIncludedFileName = sShaderSource.substr(iIncludedFileNamePositionStart, iIncludedFileNameLength);
               std::string sIncludedFileNameWithoutExtension = sIncludedFileName.substr(0, sIncludedFileName.find('.'));

               char sIncludeFilePath[MAX_PATH];
               sprintf(sIncludeFilePath, "%s\\Shaders\\glsl\\%s.%s", ContentXmlDirName, sIncludedFileNameWithoutExtension.c_str(), sShaderType[i]);

               std::ifstream sIncludedShaderFile(sIncludeFilePath);
               std::string sIncludedShaderSource((std::istreambuf_iterator<char>(sIncludedShaderFile)), std::istreambuf_iterator<char>());
               sIncludedShaderFile.close();

               sShaderSource.replace(iIncludePosition, IncludeStrLength + iIncludedFileNameLength + 2, sIncludedShaderSource.c_str());

               iIncludePosition = sShaderSource.find(IncludeStr);
            }

            // clean comments and empty lines
            const char* CommentStr = "//";
            const size_t CommentStrLength = strlen(CommentStr);

            size_t iCommentPosition = sShaderSource.find(CommentStr);

            while(iCommentPosition != std::string::npos)
            {
               size_t iEndLinePosition = sShaderSource.find('\n', iCommentPosition);
               sShaderSource.replace(iCommentPosition, iEndLinePosition - iCommentPosition, "");
               iCommentPosition = sShaderSource.find(CommentStr, iCommentPosition);
            }

            for(uint32_t i = 0; i < sShaderSource.size() - 1; i++)
            {
               if(sShaderSource[i] == '\n' && sShaderSource[i + 1] == '\n')
               {
                  sShaderSource.erase(sShaderSource.begin() + i);
                  i--;
               }
            }

            iShaderByteCodeSize = (uint32_t)sShaderSource.size();
            pShaderByteCodeData = &sShaderSource[0];
         }

         GEAssert(iShaderByteCodeSize > 0);

         for(uint32_t i = 0; i < sShaderSource.size(); i++)
         {
            sShaderSource[i] += 128;
         }

         Value(iShaderByteCodeSize).writeToStream(sOutputFile);
         sOutputFile.write(pShaderByteCodeData, iShaderByteCodeSize);
      }
   }

   sOutputFile.close();
}

void ContentCompiler::packTextures(const char* pTextureFilesExtension)
{
   Log::log(LogType::Info, "Packing textures...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Textures\\*.textures.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);
   
   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      packTextureFile(sFileData.cFileName, pTextureFilesExtension);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packTextureFile(const char* XmlFileName, const char* pTextureFileExtension)
{
   char sBinFileName[MAX_PATH];
   getBinFileName(XmlFileName, sBinFileName);

   char sOutputPath[MAX_PATH];
   GetCurrentDirectory(MAX_PATH, sOutputPath);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
   size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\Textures", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
   toHashPath(sOutputPath + outputPathOffset);
   outputPathOffset = strlen(sOutputPath) + 1u;
#endif
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
   toHashPath(sOutputPath + outputPathOffset);
#endif

   std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
   GEAssert(sOutputFile.is_open());

   char sInputPath[MAX_PATH];
   sprintf(sInputPath, "%s\\Textures\\%s", ContentXmlDirName, XmlFileName);

   pugi::xml_document xml;
   xml.load_file(sInputPath);
   const pugi::xml_node& xmlTextures = xml.child("TextureList");
   uint16_t iTexturesCount = 0u;

   for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
   {
      iTexturesCount++;
   }

   Value(iTexturesCount).writeToStream(sOutputFile);

   std::string sSetName = std::string(XmlFileName);
   size_t iExtPos = sSetName.find(".textures.xml");
   sSetName.replace(iExtPos, sSetName.length(), "");

   for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
   {
      const char* sTextureName = xmlTexture.attribute("name").value();
      Value(sTextureName).writeToStream(sOutputFile);

      Texture* cTexture = new Texture(sTextureName, sSetName.c_str());
      const bool textureAlreadyAdded = mManagerTextures.get(sTextureName) != nullptr;

      if(!textureAlreadyAdded)
      {
         mManagerTextures.add(cTexture);
      }

      cTexture->loadFromXml(xmlTexture);
      cTexture->saveToStream(sOutputFile);

      char textureFileExtension[8];

      if(pTextureFileExtension[0] != '\0')
      {
         strcpy(textureFileExtension, pTextureFileExtension);
      }
      else
      {
         sprintf(textureFileExtension, ".%s", cTexture->getFormat());
      }

      std::string sTextureFilePath;
      sTextureFilePath.append(ContentXmlDirName);
      sTextureFilePath.append("\\Textures\\");
      sTextureFilePath.append(sSetName);
      sTextureFilePath.append("\\");
      sTextureFilePath.append(sTextureName);
      sTextureFilePath.append(textureFileExtension);
      
      std::ifstream sTextureFile(sTextureFilePath, std::ios::in | std::ios::binary);
      GEAssert(sTextureFile.is_open());
      sTextureFile.seekg(0, std::ios::end);
      uint32_t iTextureFileSize = (uint32_t)sTextureFile.tellg();
      sTextureFile.seekg(0, std::ios::beg);

      Value(iTextureFileSize).writeToStream(sOutputFile);
      std::copy_n(std::istreambuf_iterator<char>(sTextureFile), iTextureFileSize, std::ostreambuf_iterator<char>(sOutputFile));

      if(GEHasFlag(cTexture->getSettings(), TextureSettingsBitMask::AtlasUV))
      {
         std::string sTextureAtlasFilePath;
         sTextureAtlasFilePath.append(ContentXmlDirName);
         sTextureAtlasFilePath.append("\\Textures\\");
         sTextureAtlasFilePath.append(sSetName);
         sTextureAtlasFilePath.append("\\");
         sTextureAtlasFilePath.append(sTextureName);
         sTextureAtlasFilePath.append(".xml");

         pugi::xml_document xmlAtlas;
         xmlAtlas.load_file(sTextureAtlasFilePath.c_str());
         const pugi::xml_node& xmlChars = xmlAtlas.child("TextureAtlas");

         GE::byte iAtlasEntriesCount = 0u;

         for(const pugi::xml_node& xmlChar : xmlChars.children("sprite"))
         {
            iAtlasEntriesCount++;
         }

         Value(iAtlasEntriesCount).writeToStream(sOutputFile);

         for(const pugi::xml_node& xmlChar : xmlChars.children("sprite"))
         {
            Value(ObjectName(xmlChar.attribute("n").value())).writeToStream(sOutputFile);
            Value((uint16_t)Parser::parseInt(xmlChar.attribute("x").value())).writeToStream(sOutputFile);
            Value((uint16_t)Parser::parseInt(xmlChar.attribute("y").value())).writeToStream(sOutputFile);
            Value((uint16_t)Parser::parseInt(xmlChar.attribute("w").value())).writeToStream(sOutputFile);
            Value((uint16_t)Parser::parseInt(xmlChar.attribute("h").value())).writeToStream(sOutputFile);
         }
      }

      sTextureFile.close();

      if(textureAlreadyAdded)
      {
         delete cTexture;
      }
   }

   sOutputFile.close();
}

void ContentCompiler::packMaterials()
{
   Log::log(LogType::Info, "Packing materials...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Materials\\*.materials.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      const char* sXmlFileName = sFileData.cFileName;

      char sBinFileName[MAX_PATH];
      getBinFileName(sXmlFileName, sBinFileName);

      char sGroupName[MAX_PATH];
      strcpy(sGroupName, sXmlFileName);
      size_t iXmlFileNameLength = strlen(sXmlFileName);
      sGroupName[iXmlFileNameLength - 14] = '\0';

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
      size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Materials", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
      outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
#endif

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Materials\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlMaterials = xml.child("MaterialList");
      GE::byte iMaterialsCount = 0u;

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
      {
         iMaterialsCount++;
      }

      Value(iMaterialsCount).writeToStream(sOutputFile);

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
      {
         const char* sMaterialName = xmlMaterial.attribute("name").value();
         Value(sMaterialName).writeToStream(sOutputFile);

         Material* cMaterial = new Material(sMaterialName, sGroupName);
         mManagerMaterials.add(cMaterial);
         cMaterial->loadFromXml(xmlMaterial);
         cMaterial->saveToStream(sOutputFile);
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packFonts(const char* pTextureFilesExtension)
{
   Log::log(LogType::Info, "Packing fonts...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Fonts\\*.fonts.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      packFontFile(sFileData.cFileName, pTextureFilesExtension);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packFontFile(const char* XmlFileName, const char* pTextureFileExtension)
{
   char sBinFileName[MAX_PATH];
   getBinFileName(XmlFileName, sBinFileName);

   char sOutputPath[MAX_PATH];
   GetCurrentDirectory(MAX_PATH, sOutputPath);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
   size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\Fonts", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
   toHashPath(sOutputPath + outputPathOffset);
   outputPathOffset = strlen(sOutputPath) + 1u;
#endif
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

#if GE_HASH_OUTPUT_PATHS
   toHashPath(sOutputPath + outputPathOffset);
#endif

   std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
   GEAssert(sOutputFile.is_open());

   char sInputPath[MAX_PATH];
   sprintf(sInputPath, "%s\\Fonts\\%s", ContentXmlDirName, XmlFileName);

   pugi::xml_document xml;
   xml.load_file(sInputPath);
   const pugi::xml_node& xmlFonts = xml.child("FontList");
   GE::byte iFontsCount = 0u;

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
   {
      iFontsCount++;
   }

   Value(iFontsCount).writeToStream(sOutputFile);

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
   {
      const char* sFontName = xmlFont.attribute("name").value();
      Value(sFontName).writeToStream(sOutputFile);
   }

   std::string sSetName = std::string(XmlFileName);
   size_t iExtPos = sSetName.find(".fonts.xml");
   sSetName.replace(iExtPos, sSetName.length(), "");

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
   {
      const char* sFontName = xmlFont.attribute("name").value();
      Value(sFontName).writeToStream(sOutputFile);

      Font cFont(sFontName, sSetName.c_str());
      cFont.loadFromXml(xmlFont);
      cFont.saveToStream(sOutputFile);

      if(cFont.getFontCharacterSetCount() == 0u)      
      {
         cFont.addFontCharacterSet();
      }

      for(uint32_t i = 0u; i < cFont.getFontCharacterSetCount(); i++)
      {
         FontCharacterSet* charSet = cFont.getFontCharacterSet(i);

         std::string sFontCharSetFilePath;
         sFontCharSetFilePath.append(ContentXmlDirName);
         sFontCharSetFilePath.append("\\Fonts\\");
         sFontCharSetFilePath.append(sSetName);
         sFontCharSetFilePath.append("\\");
         sFontCharSetFilePath.append(sFontName);

         if(!charSet->getName().isEmpty())
         {
            sFontCharSetFilePath.append("_");
            sFontCharSetFilePath.append(charSet->getName().getString());
         }

         sFontCharSetFilePath.append(".fnt");

         pugi::xml_document xmlFontDesc;
         xmlFontDesc.load_file(sFontCharSetFilePath.c_str());
         const pugi::xml_node& xmlFontDescRoot = xmlFontDesc.child("font");

         const pugi::xml_node& xmlCommon = xmlFontDescRoot.child("common");
         Value((uint16_t)Parser::parseUInt(xmlCommon.attribute("scaleW").value())).writeToStream(sOutputFile);
         Value((uint16_t)Parser::parseUInt(xmlCommon.attribute("scaleH").value())).writeToStream(sOutputFile);
         Value(Parser::parseFloat(xmlCommon.attribute("base").value())).writeToStream(sOutputFile);
         Value(Parser::parseFloat(xmlCommon.attribute("lineHeight").value())).writeToStream(sOutputFile);

         const pugi::xml_node& xmlChars = xmlFontDescRoot.child("chars");
         uint16_t iFontCharsCount = 0u;

         for(const pugi::xml_node& xmlChar : xmlChars.children("char"))
         {
            iFontCharsCount++;
         }

         Value(iFontCharsCount).writeToStream(sOutputFile);

         for(const pugi::xml_node& xmlChar : xmlChars.children("char"))
         {
            Value((uint16_t)Parser::parseUInt(xmlChar.attribute("id").value())).writeToStream(sOutputFile);
            Value(Parser::parseFloat(xmlChar.attribute("x").value())).writeToStream(sOutputFile);
            Value(Parser::parseFloat(xmlChar.attribute("y").value())).writeToStream(sOutputFile);
            Value(Parser::parseFloat(xmlChar.attribute("width").value())).writeToStream(sOutputFile);
            Value(Parser::parseFloat(xmlChar.attribute("height").value())).writeToStream(sOutputFile);
            Value(Parser::parseFloat(xmlChar.attribute("xoffset").value())).writeToStream(sOutputFile);
            Value(Parser::parseFloat(xmlChar.attribute("yoffset").value())).writeToStream(sOutputFile);
            Value(Parser::parseFloat(xmlChar.attribute("xadvance").value())).writeToStream(sOutputFile);
         }

         const pugi::xml_node& xmlKernings = xmlFontDescRoot.child("kernings");
         uint16_t iFontKerningsCount = (uint16_t)Parser::parseUInt(xmlKernings.attribute("count").value());
         Value(iFontKerningsCount).writeToStream(sOutputFile);

         for(const pugi::xml_node& xmlKerning : xmlKernings.children("kerning"))
         {
            uint16_t iKerningFirstCharId = (uint16_t)Parser::parseUInt(xmlKerning.attribute("first").value());
            uint16_t iKerningSecondCharId = (uint16_t)Parser::parseUInt(xmlKerning.attribute("second").value());
            int iKerningAmount = Parser::parseInt(xmlKerning.attribute("amount").value());

            Value(iKerningFirstCharId).writeToStream(sOutputFile);
            Value(iKerningSecondCharId).writeToStream(sOutputFile);
            Value(iKerningAmount).writeToStream(sOutputFile);
         }
      }

      char textureFileExtension[8];

      if(pTextureFileExtension[0] != '\0')
      {
         strcpy(textureFileExtension, pTextureFileExtension);
      }
      else
      {
         strcpy(textureFileExtension, ".png");
      }

      std::string sFontFilePath;
      sFontFilePath.append(ContentXmlDirName);
      sFontFilePath.append("\\Fonts\\");
      sFontFilePath.append(sSetName);
      sFontFilePath.append("\\");
      sFontFilePath.append(sFontName);
      sFontFilePath.append(textureFileExtension);

      std::ifstream sFontTextureFile(sFontFilePath, std::ios::in | std::ios::binary);
      GEAssert(sFontTextureFile.is_open());
      sFontTextureFile.seekg(0, std::ios::end);
      uint32_t iFontFileSize = (uint32_t)sFontTextureFile.tellg();
      sFontTextureFile.seekg(0, std::ios::beg);

      Value(iFontFileSize).writeToStream(sOutputFile);
      std::copy_n(std::istreambuf_iterator<char>(sFontTextureFile), iFontFileSize, std::ostreambuf_iterator<char>(sOutputFile));

      sFontTextureFile.close();
   }

   sOutputFile.close();
}

void ContentCompiler::packStrings()
{
   Log::log(LogType::Info, "Packing strings...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Strings\\*.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      const char* sXmlFileName = sFileData.cFileName;

      char sBinFileName[MAX_PATH];
      getBinFileName(sXmlFileName, sBinFileName);

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
      size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Strings", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
      outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
#endif

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Strings\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlStrings = xml.child("Strings");
      uint16_t iStringsCount = 0u;

      for(const pugi::xml_node& xmlString : xmlStrings.children("String"))
      {
         iStringsCount++;
      }

      Value(iStringsCount).writeToStream(sOutputFile);

      for(const pugi::xml_node& xmlString : xmlStrings.children("String"))
      {
         const char* stringID = xmlString.attribute("id").value();
         Value(stringID).writeToStream(sOutputFile);

         const char* stringText = xmlString.attribute("text").value();

         const uint16_t stringLength = (uint16_t)strlen(stringText);
         Value(stringLength).writeToStream(sOutputFile);

         sOutputFile.write(stringText, stringLength);
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packMeshes()
{
   Log::log(LogType::Info, "Packing meshes...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Meshes\\*.meshes.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);
   
   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      packMeshFile(sFileData.cFileName);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packMeshFile(const char* XmlFileName)
{
   char sBinFileName[MAX_PATH];
   getBinFileName(XmlFileName, sBinFileName);

   char sOutputPath[MAX_PATH];
   GetCurrentDirectory(MAX_PATH, sOutputPath);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
   size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\Meshes", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
   toHashPath(sOutputPath + outputPathOffset);
   outputPathOffset = strlen(sOutputPath) + 1u;
#endif
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
   toHashPath(sOutputPath + outputPathOffset);
#endif

   std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
   GEAssert(sOutputFile.is_open());

   char sInputPath[MAX_PATH];
   sprintf(sInputPath, "%s\\Meshes\\%s", ContentXmlDirName, XmlFileName);

   pugi::xml_document xml;
   xml.load_file(sInputPath);
   const pugi::xml_node& xmlMeshes = xml.child("MeshList");
   uint16_t iMeshesCount = 0u;

   for(const pugi::xml_node& xmlMesh : xmlMeshes.children("Mesh"))
   {
      iMeshesCount++;
   }

   Value(iMeshesCount).writeToStream(sOutputFile);

   std::string sSetName = std::string(XmlFileName);
   size_t iExtPos = sSetName.find(".meshes.xml");
   sSetName.replace(iExtPos, sSetName.length(), "");

   for(const pugi::xml_node& xmlMesh : xmlMeshes.children("Mesh"))
   {
      const char* sMeshName = xmlMesh.attribute("name").value();
      Value(sMeshName).writeToStream(sOutputFile);

      std::string sMeshFilePath;
      sMeshFilePath.append(ContentXmlDirName);
      sMeshFilePath.append("\\Meshes\\");
      sMeshFilePath.append(sSetName);
      sMeshFilePath.append("\\");
      sMeshFilePath.append(sMeshName);
      sMeshFilePath.append(".mesh.ge");
      
      std::ifstream sMeshFile(sMeshFilePath, std::ios::in | std::ios::binary);
      GEAssert(sMeshFile.is_open());
      sMeshFile.seekg(0, std::ios::end);
      const uint32_t iMeshFileSize = (uint32_t)sMeshFile.tellg();
      sMeshFile.seekg(0, std::ios::beg);

      Value(iMeshFileSize).writeToStream(sOutputFile);
      std::copy_n(std::istreambuf_iterator<char>(sMeshFile), iMeshFileSize, std::ostreambuf_iterator<char>(sOutputFile));

      sMeshFile.close();
   }

   sOutputFile.close();
}

void ContentCompiler::packSkeletons()
{
   Log::log(LogType::Info, "Packing skeletons...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Meshes\\*.skeleton.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      const char* sXmlFileName = sFileData.cFileName;

      char sBinFileName[MAX_PATH];
      getBinFileName(sXmlFileName, sBinFileName);

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
      size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Meshes", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
      outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
#endif

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Meshes\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlSkeleton = xml.child("Skeleton");

      uint32_t iBonesCount = Parser::parseUInt(xmlSkeleton.attribute("bonesCount").value());
      Value((GE::byte)iBonesCount).writeToStream(sOutputFile);

      for(pugi::xml_node_iterator it = xmlSkeleton.begin(); it != xmlSkeleton.end(); it++)
      {
         const pugi::xml_node& xmlBone = *it;
         
         ObjectName cBoneName = ObjectName(xmlBone.attribute("name").value());
         Value(cBoneName).writeToStream(sOutputFile);

         pugi::xml_attribute xmlBoneParentIndex = xmlBone.attribute("parentIndex");
         uint32_t iBoneParentIndex = !xmlBoneParentIndex.empty()
            ? Parser::parseInt(xmlBoneParentIndex.value())
            : 0;
         Value((GE::byte)iBoneParentIndex).writeToStream(sOutputFile);

         Value(Parser::parseVector3(xmlBone.attribute("bindT").value())).writeToStream(sOutputFile);
         Value(Parser::parseVector3(xmlBone.attribute("bindR").value())).writeToStream(sOutputFile);
         Value(Parser::parseVector3(xmlBone.attribute("bindS").value())).writeToStream(sOutputFile);
         Value(Parser::parseFloat(xmlBone.attribute("size").value())).writeToStream(sOutputFile);

         const pugi::xml_node& xmlBoneChildren = xmlBone.child("Children");
         uint32_t iBoneChildrenCount = 0;

         for(pugi::xml_node_iterator it2 = xmlBoneChildren.begin(); it2 != xmlBoneChildren.end(); it2++)
         {
            iBoneChildrenCount++;
         }

         Value((GE::byte)iBoneChildrenCount).writeToStream(sOutputFile);

         for(pugi::xml_node_iterator it2 = xmlBoneChildren.begin(); it2 != xmlBoneChildren.end(); it2++)
         {
            const pugi::xml_node& xmlBoneChild = *it2;
            Value((GE::byte)Parser::parseUInt(xmlBoneChild.attribute("index").value())).writeToStream(sOutputFile);
         }
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packAnimations()
{
   Log::log(LogType::Info, "Packing animations...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Animations\\*.animationset.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      const char* sXmlFileName = sFileData.cFileName;

      char sBinFileName[MAX_PATH];
      getBinFileName(sXmlFileName, sBinFileName);

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
      size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Animations", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
      outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
#endif

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Animations\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlAnimationSet = xml.child("AnimationSet");
      GE::byte iAnimationsCount = 0u;

      for(const pugi::xml_node& xmlAnimation : xmlAnimationSet.children("Animation"))
      {
         iAnimationsCount++;
      }

      Value(iAnimationsCount).writeToStream(sOutputFile);

      std::string sSetName = std::string(sXmlFileName);
      size_t iExtPos = sSetName.find(".animationset.xml");
      sSetName.replace(iExtPos, sSetName.length(), "");

      for(const pugi::xml_node& xmlAnimation : xmlAnimationSet.children("Animation"))
      {
         const char* sAnimationName = xmlAnimation.attribute("name").value();
         Value(sAnimationName).writeToStream(sOutputFile);
         const char* sAnimationFileName = xmlAnimation.attribute("fileName").value();
         Value(sAnimationFileName).writeToStream(sOutputFile);

         pugi::xml_attribute xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionX");
         bool bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOutputFile);

         xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionY");
         bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOutputFile);

         xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionZ");
         bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOutputFile);

         std::string sAnimationFilePath;
         sAnimationFilePath.append(ContentXmlDirName);
         sAnimationFilePath.append("\\Animations\\");
         sAnimationFilePath.append(sSetName);
         sAnimationFilePath.append("\\");
         sAnimationFilePath.append(sAnimationFileName);
         sAnimationFilePath.append(".animation.ge");
      
         std::ifstream sAnimationFile(sAnimationFilePath, std::ios::in | std::ios::binary);
         GEAssert(sAnimationFile.is_open());
         sAnimationFile.seekg(0, std::ios::end);
         const uint32_t iAnimationFileSize = (uint32_t)sAnimationFile.tellg();
         sAnimationFile.seekg(0, std::ios::beg);

         Value(iAnimationFileSize).writeToStream(sOutputFile);
         std::copy_n(std::istreambuf_iterator<char>(sAnimationFile), iAnimationFileSize, std::ostreambuf_iterator<char>(sOutputFile));

         sAnimationFile.close();
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packSounds(const char* pAudioFilesExtension)
{
   Log::log(LogType::Info, "Packing sounds...");

   packSerializableResources<AudioBank>();
   packSerializableResources<AudioEvent>();
   packSerializableResources<AudioBus>();

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Audio\\files\\*%s", ContentXmlDirName, pAudioFilesExtension);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do
   {
      const char* sFileName = sFileData.cFileName;

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Audio\\files\\%s", ContentXmlDirName, sFileName);

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
      size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Audio\\files", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
      outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sFileName);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
#endif

      CopyFile(sInputPath, sOutputPath, false);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packPrefabs()
{
   Log::log(LogType::Info, "Packing prefabs...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Prefabs\\*.prefab.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do
   {
      const char* sXmlFileName = sFileData.cFileName;

      char sBinFileName[MAX_PATH];
      getBinFileName(sXmlFileName, sBinFileName);

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
      size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Prefabs", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
      outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
#endif

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Prefabs\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      pugi::xml_node xmlPrefab = xml.child("Prefab");

      pugi::xml_attribute xmlPrefabBase = xmlPrefab.attribute("base");

      while(!xmlPrefabBase.empty())
      {
         pugi::xml_document xmlDerived;
         xmlDerived.append_copy(xmlPrefab);
         pugi::xml_node xmlDerivedRoot = xmlDerived.child("Prefab");

         const char* prefabBaseName = xmlPrefabBase.value();
         sprintf(sInputPath, "%s\\Prefabs\\%s.prefab.xml", ContentXmlDirName, prefabBaseName);

         xml.remove_child(xmlPrefab);

         xml.load_file(sInputPath);
         xmlPrefab = xml.child("Prefab");

         Entity::mergeXmlDescription(xmlPrefab, xmlDerivedRoot);

         xmlPrefabBase = xmlPrefab.attribute("base");
      }

      packEntity(ObjectName::Empty, xmlPrefab, sOutputFile, 0);

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packScenes()
{
   Log::log(LogType::Info, "Packing scenes...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Scenes\\*.scene.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do
   {
      const char* sXmlFileName = sFileData.cFileName;

      char sBinFileName[MAX_PATH];
      getBinFileName(sXmlFileName, sBinFileName);

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
      size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Scenes", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
      outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
#endif

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Scenes\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      pugi::xml_node xmlScene = xml.child("Scene");
      cDummyScene.xmlToStream(xmlScene, sOutputFile);
      
      xml_object_range<xml_named_node_iterator> xmlRootEntities = xmlScene.children("Entity");
      GE::byte iRootEntitiesCount = 0u;

      for(const pugi::xml_node& xmlRootEntity : xmlRootEntities)
      {
         iRootEntitiesCount++;
      }

      Value(iRootEntitiesCount).writeToStream(sOutputFile);

      for(const pugi::xml_node& xmlRootEntity : xmlRootEntities)
      {
         ObjectName cEntityName = ObjectName(xmlRootEntity.attribute("name").value());
         packEntity(cEntityName, xmlRootEntity, sOutputFile, 0);
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::packData()
{
   Log::log(LogType::Info, "Packing data...");

   packSerializableResources<Clock>();
   packSerializableResources<Curve>();

   if(packAppResources)
   {
      packAppResources();
   }
}

void ContentCompiler::compileScripts()
{
   Log::log(LogType::Info, "Compiling scripts...");

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Scripts\\*.lua", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   const size_t bufferSize = 256u * 1024u;

   char codeBuffer[bufferSize];
   char byteCodeBuffer[bufferSize];

   do
   {
      const char* sScriptFileName = sFileData.cFileName;

      char sCurrentDirectory[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sCurrentDirectory);

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\%s\\Scripts\\%s", sCurrentDirectory, ContentXmlDirName, sScriptFileName);

      char sOutputPath[MAX_PATH];
      sprintf(sOutputPath, "%s\\%s", sCurrentDirectory, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
      size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Scripts", sOutputPath);
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
      outputPathOffset = strlen(sOutputPath) + 1u;
#endif
      CreateDirectory(sOutputPath, NULL);
#if defined GE_64_BIT
      sprintf(sOutputPath, "%s\\x64_%sbc", sOutputPath, sScriptFileName);      
#else
      sprintf(sOutputPath, "%s\\%sbc", sOutputPath, sScriptFileName);
#endif
#if GE_HASH_OUTPUT_PATHS
      toHashPath(sOutputPath + outputPathOffset);
#endif

      FILE* inputFile = fopen(sInputPath, "rb");
      fseek(inputFile, 0, SEEK_END);
      const size_t inputFileSize = (size_t)ftell(inputFile);
      rewind(inputFile);
      fread(codeBuffer, 1u, inputFileSize, inputFile);
      codeBuffer[inputFileSize] = '\0';
      fclose(inputFile);

      FILE* outputFile = fopen(sOutputPath, "wb");
      const size_t outputFileSize =
         Scripting::Environment::compileScript(codeBuffer, bufferSize, byteCodeBuffer);
      GEAssert(outputFileSize > 0u);
      fwrite(byteCodeBuffer, 1u, outputFileSize, outputFile);
      fclose(outputFile);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void ContentCompiler::compileContent(const char* pConfigFilePath)
{
   pugi::xml_document xml;
   xml.load_file(pConfigFilePath);
   const pugi::xml_node& xmlConfig = xml.child("ContentCompilerConfig");

   ContentCompilerConfig config;
   config.loadFromXml(xmlConfig);

   registerCompilerObjectManagers();

   if(config.getIncludeShadersDirectX())
   {
      packShaders(ApplicationRenderingAPI::DirectX);
   }

   mManagerShaderPrograms.clear();

   if(config.getIncludeShadersOpenGL())
   {
      packShaders(ApplicationRenderingAPI::OpenGL);
   }

   packTextures(config.getTextureFilesExtension());
   packMaterials();
   packFonts(config.getTextureFilesExtension());
   packStrings();
   packMeshes();
   packSkeletons();
   packAnimations();
   packSounds(config.getAudioFilesExtension());
   packPrefabs();
   packScenes();
   packData();
   compileScripts();

   mManagerShaderPrograms.clear();
   mManagerTextures.clear();
   mManagerMaterials.clear();
   mManagerFonts.clear();

   RenderSystem::getInstance()->registerObjectManagers();
}

#endif
