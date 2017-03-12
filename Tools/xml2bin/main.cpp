
#undef UNICODE

#include <windows.h>
#include <Shlobj.h>

#include "main.h"

#include "Core/GEPlatform.h"
#include "Core/GEApplication.h"
#include "Core/GEAllocator.h"
#include "Core/GEValue.h"
#include "Core/GEParser.h"
#include "Core/GEObject.h"
#include "Rendering/GEMaterial.h"
#include "Rendering/GEFont.h"
#include "Entities/GEEntity.h"
#include "Entities/GEComponent.h"

#include "Externals/pugixml/pugixml.hpp"

#include <iostream>
#include <fstream>

#pragma comment(lib, "./../GameEngine.DX11.lib")
#pragma comment(lib, "./../pugixml.Windows.lib")
#pragma comment(lib, "./../stb.Windows.lib")

#if defined (_M_X64)
# pragma comment(lib, "./../../Externals/Brofiler/ProfilerCore64.lib")
#else
# pragma comment(lib, "./../../Externals/Brofiler/ProfilerCore32.lib")
#endif

using namespace GE;
using namespace GE::Core;
using namespace GE::Rendering;
using namespace GE::Entities;
using namespace pugi;

const char ContentXmlDirName[] = "content";
const char ContentBinDirName[] = "contentBin";

ObjectManager<ShaderProgram> mManagerShaderPrograms;
ObjectManager<Material> mManagerMaterials;
ObjectManager<Texture> mManagerTextures;
ObjectManager<Font> mManagerFonts;

Scene cDummyScene = Scene(ObjectName("Dummy"));

int main(int argc, char* argv[])
{
   std::cout << "\n Game Engine\n Arturo Cepeda\n Tools\n";

   Application::startUp();
   
   registerObjectManagers();
   loadShaders();

   packTextures();
   packMaterials();
   packFonts();
   packStrings();
   packMeshes();
   packSkeletons();
   packAnimations();
   packPrefabs();
   packScenes();
   compileScripts();

   Application::shutDown();

   return 0;
}

void registerObjectManagers()
{
   ObjectManagers::getInstance()->registerObjectManager<ShaderProgram>("ShaderProgram", &mManagerShaderPrograms);
   ObjectManagers::getInstance()->registerObjectManager<Material>("Material", &mManagerMaterials);
   ObjectManagers::getInstance()->registerObjectManager<Texture>("Texture", &mManagerTextures);
   ObjectManagers::getInstance()->registerObjectManager<Font>("Font", &mManagerFonts);
}

void loadShaders()
{
   pugi::xml_document xml;
   xml.load_file(L"Content\\Shaders\\Shaders.xml");
   const pugi::xml_node& xmlShaders = xml.child("Shaders");

   for(const pugi::xml_node& xmlShader : xmlShaders.children("Shader"))
   {
      const char* sShaderProgramName = xmlShader.attribute("name").value();
      ShaderProgram* cShaderProgram = new ShaderProgram(sShaderProgramName);
      mManagerShaderPrograms.add(cShaderProgram);

      cShaderProgram->parseParameters(xmlShader);
      cShaderProgram->loadFromXml(xmlShader);
   }
}

void packTextures()
{
   std::cout << "\n Packing textures...";

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Textures\\*.textures.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);
   
   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      packTextureFile(sFileData.cFileName);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void packTextureFile(const char* XmlFileName)
{
   char sBinFileName[MAX_PATH];
   getBinFileName(XmlFileName, sBinFileName);

   char sOutputPath[MAX_PATH];
   GetCurrentDirectory(MAX_PATH, sOutputPath);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\Textures", sOutputPath);
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

   std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
   GEAssert(sOutputFile.is_open());

   char sInputPath[MAX_PATH];
   sprintf(sInputPath, "%s\\Textures\\%s", ContentXmlDirName, XmlFileName);

   pugi::xml_document xml;
   xml.load_file(sInputPath);
   const pugi::xml_node& xmlTextures = xml.child("Textures");
   GE::byte iTexturesCount = 0;

   for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
      iTexturesCount++;

   Value(iTexturesCount).writeToStream(sOutputFile);

   for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
   {
      const char* sTextureName = xmlTexture.attribute("name").value();
      const char* sTextureFileName = xmlTexture.attribute("fileName").value();
      const char* sTextureFormat = xmlTexture.attribute("format").value();
      const bool bTextureAtlas = Parser::parseBool(xmlTexture.attribute("atlas").value());

      Value(sTextureName).writeToStream(sOutputFile);
      Value(sTextureFormat).writeToStream(sOutputFile);
      Value(bTextureAtlas).writeToStream(sOutputFile);

      std::string sTextureFilePath;
      sTextureFilePath.append(ContentXmlDirName);
      sTextureFilePath.append("\\Textures\\");
      sTextureFilePath.append(sTextureFileName);
      sTextureFilePath.append(".");
      sTextureFilePath.append(sTextureFormat);
      
      std::ifstream sTextureFile(sTextureFilePath, std::ios::in | std::ios::binary);
      GEAssert(sTextureFile.is_open());
      sTextureFile.seekg(0, std::ios::end);
      uint iTextureFileSize = (uint)sTextureFile.tellg();
      sTextureFile.seekg(0, std::ios::beg);

      Value(iTextureFileSize).writeToStream(sOutputFile);
      std::copy_n(std::istreambuf_iterator<char>(sTextureFile), iTextureFileSize, std::ostreambuf_iterator<char>(sOutputFile));

      if(bTextureAtlas)
      {
         std::string sTextureAtlasFilePath;
         sTextureAtlasFilePath.append(ContentXmlDirName);
         sTextureAtlasFilePath.append("\\Textures\\");
         sTextureAtlasFilePath.append(sTextureFileName);
         sTextureAtlasFilePath.append(".xml");

         pugi::xml_document xmlAtlas;
         xmlAtlas.load_file(sTextureAtlasFilePath.c_str());
         const pugi::xml_node& xmlChars = xmlAtlas.child("TextureAtlas");

         GE::byte iAtlasEntriesCount = 0;

         for(const pugi::xml_node& xmlChar : xmlChars.children("sprite"))
            iAtlasEntriesCount++;

         Value(iAtlasEntriesCount).writeToStream(sOutputFile);

         for(const pugi::xml_node& xmlChar : xmlChars.children("sprite"))
         {
            Value(ObjectName(xmlChar.attribute("n").value())).writeToStream(sOutputFile);
            Value((short)Parser::parseInt(xmlChar.attribute("x").value())).writeToStream(sOutputFile);
            Value((short)Parser::parseInt(xmlChar.attribute("y").value())).writeToStream(sOutputFile);
            Value((short)Parser::parseInt(xmlChar.attribute("w").value())).writeToStream(sOutputFile);
            Value((short)Parser::parseInt(xmlChar.attribute("h").value())).writeToStream(sOutputFile);
         }
      }

      sTextureFile.close();
   }

   sOutputFile.close();
}

void packMaterials()
{
   std::cout << "\n Packing materials...";

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

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Materials", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Materials\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlMaterials = xml.child("Materials");
      GE::byte iMaterialsCount = 0;

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
         iMaterialsCount++;

      Value(iMaterialsCount).writeToStream(sOutputFile);

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
      {
         const char* sMaterialName = xmlMaterial.attribute("name").value();
         Value(sMaterialName).writeToStream(sOutputFile);

         Material* cMaterial = new Material(sMaterialName);
         mManagerMaterials.add(cMaterial);
         cMaterial->loadFromXml(xmlMaterial);
         cMaterial->xmlToStream(xmlMaterial, sOutputFile);
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void packFonts()
{
   std::cout << "\n Packing fonts...";

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Fonts\\*.fonts.xml", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      packFontFile(sFileData.cFileName);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void packFontFile(const char* XmlFileName)
{
   char sBinFileName[MAX_PATH];
   getBinFileName(XmlFileName, sBinFileName);

   char sOutputPath[MAX_PATH];
   GetCurrentDirectory(MAX_PATH, sOutputPath);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\Fonts", sOutputPath);
   CreateDirectory(sOutputPath, NULL);
   sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

   std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
   GEAssert(sOutputFile.is_open());

   char sInputPath[MAX_PATH];
   sprintf(sInputPath, "%s\\Fonts\\%s", ContentXmlDirName, XmlFileName);

   pugi::xml_document xml;
   xml.load_file(sInputPath);
   const pugi::xml_node& xmlFonts = xml.child("Fonts");
   GE::byte iFontsCount = 0;

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
      iFontsCount++;

   Value(iFontsCount).writeToStream(sOutputFile);

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
   {
      const char* sFontName = xmlFont.attribute("name").value();
      Value(sFontName).writeToStream(sOutputFile);
   }

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
   {
      const char* sFontName = xmlFont.attribute("name").value();
      const char* sFontFileName = xmlFont.attribute("fileName").value();

      Value(sFontName).writeToStream(sOutputFile);

      std::string sFontFilePath;
      sFontFilePath.append(ContentXmlDirName);
      sFontFilePath.append("\\Fonts\\");
      sFontFilePath.append(sFontFileName);
      sFontFilePath.append(".fnt");

      pugi::xml_document xmlFontDesc;
      xmlFontDesc.load_file(sFontFilePath.c_str());
      const pugi::xml_node& xmlFontDescRoot = xmlFontDesc.child("font");

      const pugi::xml_node& xmlCommon = xmlFontDescRoot.child("common");
      Value((short)Parser::parseInt(xmlCommon.attribute("scaleW").value())).writeToStream(sOutputFile);
      Value((short)Parser::parseInt(xmlCommon.attribute("scaleH").value())).writeToStream(sOutputFile);

      const pugi::xml_node& xmlChars = xmlFontDescRoot.child("chars");
      short iFontCharsCount = (short)Parser::parseUInt(xmlChars.attribute("count").value());
      Value(iFontCharsCount).writeToStream(sOutputFile);

      for(const pugi::xml_node& xmlChar : xmlChars.children("char"))
      {
         Value((GE::byte)Parser::parseInt(xmlChar.attribute("id").value())).writeToStream(sOutputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("x").value())).writeToStream(sOutputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("y").value())).writeToStream(sOutputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("width").value())).writeToStream(sOutputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("height").value())).writeToStream(sOutputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("xoffset").value())).writeToStream(sOutputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("yoffset").value())).writeToStream(sOutputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("xadvance").value())).writeToStream(sOutputFile);
      }

      const pugi::xml_node& xmlKernings = xmlFontDescRoot.child("kernings");
      short iFontKerningsCount = (short)Parser::parseUInt(xmlKernings.attribute("count").value());
      Value(iFontKerningsCount).writeToStream(sOutputFile);

      for(const pugi::xml_node& xmlKerning : xmlKernings.children("kerning"))
      {
         GE::byte iKerningFirstCharId = (GE::byte)Parser::parseUInt(xmlKerning.attribute("first").value());
         GE::byte iKerningSecondCharId = (GE::byte)Parser::parseUInt(xmlKerning.attribute("second").value());
         int iKerningAmount = Parser::parseInt(xmlKerning.attribute("amount").value());

         Value(iKerningFirstCharId).writeToStream(sOutputFile);
         Value(iKerningSecondCharId).writeToStream(sOutputFile);
         Value((short)iKerningAmount).writeToStream(sOutputFile);
      }

      sFontFilePath[sFontFilePath.length() - 3] = 'p';
      sFontFilePath[sFontFilePath.length() - 2] = 'n';
      sFontFilePath[sFontFilePath.length() - 1] = 'g';

      std::ifstream sFontTextureFile(sFontFilePath, std::ios::in | std::ios::binary);
      GEAssert(sFontTextureFile.is_open());
      sFontTextureFile.seekg(0, std::ios::end);
      uint iFontFileSize = (uint)sFontTextureFile.tellg();
      sFontTextureFile.seekg(0, std::ios::beg);

      Value(iFontFileSize).writeToStream(sOutputFile);
      std::copy_n(std::istreambuf_iterator<char>(sFontTextureFile), iFontFileSize, std::ostreambuf_iterator<char>(sOutputFile));

      sFontTextureFile.close();
   }

   sOutputFile.close();
}

void packStrings()
{
   std::cout << "\n Packing strings...";

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
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Strings", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Strings\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlStrings = xml.child("Strings");
      GE::byte iStringsCount = 0;

      for(const pugi::xml_node& xmlString : xmlStrings.children("String"))
         iStringsCount++;

      Value(iStringsCount).writeToStream(sOutputFile);

      for(const pugi::xml_node& xmlString : xmlStrings.children("String"))
      {
         const char* sStringID = xmlString.attribute("id").value();
         Value(sStringID).writeToStream(sOutputFile);
         const char* sStringText = xmlString.attribute("text").value();
         Value(sStringText).writeToStream(sOutputFile);
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void packMeshes()
{
   std::cout << "\n Packing meshes...";

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Models\\*.mesh.ge", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      const char* sFileName = sFileData.cFileName;

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Models\\%s", ContentXmlDirName, sFileName);

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Models", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sFileName);

      CopyFile(sInputPath, sOutputPath, false);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void packSkeletons()
{
   std::cout << "\n Packing skeletons...";

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Models\\*.skeleton.xml", ContentXmlDirName);

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
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Models", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Models\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlSkeleton = xml.child("Skeleton");

      uint iBonesCount = Parser::parseUInt(xmlSkeleton.attribute("bonesCount").value());
      Value((GE::byte)iBonesCount).writeToStream(sOutputFile);

      for(pugi::xml_node_iterator it = xmlSkeleton.begin(); it != xmlSkeleton.end(); it++)
      {
         const pugi::xml_node& xmlBone = *it;
         
         ObjectName cBoneName = ObjectName(xmlBone.attribute("name").value());
         Value(cBoneName).writeToStream(sOutputFile);

         pugi::xml_attribute xmlBoneParentIndex = xmlBone.attribute("parentIndex");
         uint iBoneParentIndex = !xmlBoneParentIndex.empty()
            ? Parser::parseInt(xmlBoneParentIndex.value())
            : 0;
         Value((GE::byte)iBoneParentIndex).writeToStream(sOutputFile);

         Value(Parser::parseVector3(xmlBone.attribute("bindT").value())).writeToStream(sOutputFile);
         Value(Parser::parseVector3(xmlBone.attribute("bindR").value())).writeToStream(sOutputFile);
         Value(Parser::parseVector3(xmlBone.attribute("bindS").value())).writeToStream(sOutputFile);
         Value(Parser::parseFloat(xmlBone.attribute("size").value())).writeToStream(sOutputFile);

         const pugi::xml_node& xmlBoneChildren = xmlBone.child("Children");
         uint iBoneChildrenCount = 0;

         for(pugi::xml_node_iterator it2 = xmlBoneChildren.begin(); it2 != xmlBoneChildren.end(); it2++)
            iBoneChildrenCount++;

         Value((GE::byte)iBoneChildrenCount).writeToStream(sOutputFile);

         for(pugi::xml_node_iterator it2 = xmlBoneChildren.begin(); it2 != xmlBoneChildren.end(); it2++)
         {
            const pugi::xml_node& xmlBoneChild = *it2;
            Value((GE::byte)Parser::parseInt(xmlBoneChild.attribute("index").value())).writeToStream(sOutputFile);
         }
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void packAnimations()
{
   std::cout << "\n Packing animations...";

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
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Animations", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Animations\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlAnimationSet = xml.child("AnimationSet");
      GE::byte iAnimationsCount = 0;

      for(const pugi::xml_node& xmlAnimation : xmlAnimationSet.children("Animation"))
         iAnimationsCount++;

      Value(iAnimationsCount).writeToStream(sOutputFile);

      for(const pugi::xml_node& xmlAnimation : xmlAnimationSet.children("Animation"))
      {
         Value(xmlAnimation.attribute("name").value()).writeToStream(sOutputFile);
         Value(xmlAnimation.attribute("fileName").value()).writeToStream(sOutputFile);

         pugi::xml_attribute xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionX");
         bool bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOutputFile);

         xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionY");
         bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOutputFile);

         xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionZ");
         bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOutputFile);
      }

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);

   sprintf(sFindString, "%s\\Animations\\*.animation.ge", ContentXmlDirName);
   hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do 
   {
      const char* sFileName = sFileData.cFileName;

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Animations\\%s", ContentXmlDirName, sFileName);

      char sOutputPath[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sOutputPath);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Animations", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sFileName);

      CopyFile(sInputPath, sOutputPath, false);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void packEntity(const ObjectName& cName, const pugi::xml_node& xmlNode, std::ofstream& sOutputFile, Entity* cParent)
{
   Value(cName).writeToStream(sOutputFile);

   Entity* cEntity = cDummyScene.addEntity(cName, cParent);
   cEntity->xmlToStream(xmlNode, sOutputFile);

   xml_object_range<xml_named_node_iterator> xmlComponents = xmlNode.children("Component");
   GE::byte iComponentsCount = 0;

   for(const pugi::xml_node& xmlComponent : xmlComponents)
      iComponentsCount++;

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
   GE::byte iChildrenCount = 0;

   for(const pugi::xml_node& xmlChild : xmlChildren)
      iChildrenCount++;

   Value(iChildrenCount).writeToStream(sOutputFile);

   for(const pugi::xml_node& xmlChild : xmlChildren)
   {
      ObjectName cChildName = ObjectName(xmlChild.attribute("name").value());
      packEntity(cChildName, xmlChild, sOutputFile, cEntity);
   }

   cDummyScene.removeEntity(cName);
   cDummyScene.update();
}

void packPrefabs()
{
   std::cout << "\n Packing prefabs...";

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
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Prefabs", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Prefabs\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlPrefab = xml.child("Prefab");
      packEntity(ObjectName::Empty, xmlPrefab, sOutputFile, 0);

      sOutputFile.close();
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void packScenes()
{
   std::cout << "\n Packing scenes...";

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
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Scenes", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);

      std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOutputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Scenes\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      pugi::xml_node xmlScene = xml.child("Scene");
      cDummyScene.xmlToStream(xmlScene, sOutputFile);
      
      xml_object_range<xml_named_node_iterator> xmlRootEntities = xmlScene.children("Entity");
      GE::byte iRootEntitiesCount = 0;

      for(const pugi::xml_node& xmlRootEntity : xmlRootEntities)
         iRootEntitiesCount++;

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

void compileScripts()
{
   std::cout << "\n Compiling scripts...";

   char sFindString[MAX_PATH];
   sprintf(sFindString, "%s\\Scripts\\*.lua", ContentXmlDirName);

   WIN32_FIND_DATA sFileData;
   HANDLE hFile = FindFirstFile(sFindString, &sFileData);

   if(hFile == INVALID_HANDLE_VALUE)
      return;

   do
   {
      const char* sScriptFileName = sFileData.cFileName;

      char sCurrentDirectory[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, sCurrentDirectory);

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\%s\\Scripts\\%s", sCurrentDirectory, ContentXmlDirName, sScriptFileName);

      char sOutputPath[MAX_PATH];
      sprintf(sOutputPath, "%s\\%s", sCurrentDirectory, ContentBinDirName);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\Scripts", sOutputPath);
      CreateDirectory(sOutputPath, NULL);
      sprintf(sOutputPath, "%s\\%sbc", sOutputPath, sScriptFileName);

      char sParameters[1024];
      sprintf(sParameters, "-o %s -s %s", sOutputPath, sInputPath);

      SHELLEXECUTEINFO sShellExecuteInfo;
      sShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
      sShellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
      sShellExecuteInfo.hwnd = GetActiveWindow();
      sShellExecuteInfo.lpVerb = "open";
      sShellExecuteInfo.lpFile = "..\\..\\GameEngine\\Tools\\Externals\\lua53\\luac53.exe";
      sShellExecuteInfo.lpParameters = sParameters;
      sShellExecuteInfo.lpDirectory = sCurrentDirectory;
      sShellExecuteInfo.nShow = SW_HIDE;
      sShellExecuteInfo.hInstApp = NULL;
      ShellExecuteEx(&sShellExecuteInfo);
   }
   while(FindNextFile(hFile, &sFileData));

   FindClose(hFile);
}

void getBinFileName(const char* XmlFileName, char* BinFileName)
{
   uint iXmlFileNameLength = (uint)strlen(XmlFileName);
   strcpy(BinFileName, XmlFileName);
   BinFileName[iXmlFileNameLength - 3] = 'g';
   BinFileName[iXmlFileNameLength - 2] = 'e';
   BinFileName[iXmlFileNameLength - 1] = '\0';
}
