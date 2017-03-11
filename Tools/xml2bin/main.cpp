
#undef UNICODE

#include <windows.h>
#include <Shlobj.h>

#include "main.h"

#include "Core/GEApplication.h"
#include "Core/GEAllocator.h"
#include "Core/GEValue.h"
#include "Core/GEParser.h"
#include "Core/GEObject.h"
#include "Rendering/GEMaterial.h"
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
using namespace pugi;

const char ContentXmlDirName[] = "content";
const char ContentBinDirName[] = "contentBin";

int main(int argc, char* argv[])
{
   std::cout << "\n Game Engine\n Arturo Cepeda\n Tools\n";

   Application::startUp();

   ObjectManager<ShaderProgram> mDummyManagerShaderPrograms;
   ObjectManager<Texture> mDummyManagerTextures;

   ObjectManagers::getInstance()->registerObjectManager<ShaderProgram>("ShaderProgram", &mDummyManagerShaderPrograms);
   ObjectManagers::getInstance()->registerObjectManager<Texture>("Texture", &mDummyManagerTextures);

   packTextures();
   packMaterials();
   packFonts();
   packStrings();
   packMeshes();
   packSkeletons();
   packAnimations();

   Application::shutDown();

   return 0;
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

   std::ofstream sOuputFile(sOutputPath, std::ios::out | std::ios::binary);
   GEAssert(sOuputFile.is_open());

   char sInputPath[MAX_PATH];
   sprintf(sInputPath, "%s\\Textures\\%s", ContentXmlDirName, XmlFileName);

   pugi::xml_document xml;
   xml.load_file(sInputPath);
   const pugi::xml_node& xmlTextures = xml.child("Textures");
   GE::byte iTexturesCount = 0;

   for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
      iTexturesCount++;

   Value(iTexturesCount).writeToStream(sOuputFile);

   for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
   {
      const char* sTextureName = xmlTexture.attribute("name").value();
      Value(sTextureName).writeToStream(sOuputFile);
   }

   for(const pugi::xml_node& xmlTexture : xmlTextures.children("Texture"))
   {
      const char* sTextureName = xmlTexture.attribute("name").value();
      const char* sTextureFileName = xmlTexture.attribute("fileName").value();
      const char* sTextureFormat = xmlTexture.attribute("format").value();
      const bool bTextureAtlas = Parser::parseBool(xmlTexture.attribute("atlas").value());

      Value(sTextureName).writeToStream(sOuputFile);
      Value(sTextureFormat).writeToStream(sOuputFile);
      Value(bTextureAtlas).writeToStream(sOuputFile);

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

      Value(iTextureFileSize).writeToStream(sOuputFile);
      std::copy_n(std::istreambuf_iterator<char>(sTextureFile), iTextureFileSize, std::ostreambuf_iterator<char>(sOuputFile));

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

         Value(iAtlasEntriesCount).writeToStream(sOuputFile);

         for(const pugi::xml_node& xmlChar : xmlChars.children("sprite"))
         {
            Value(ObjectName(xmlChar.attribute("n").value())).writeToStream(sOuputFile);
            Value((short)Parser::parseInt(xmlChar.attribute("x").value())).writeToStream(sOuputFile);
            Value((short)Parser::parseInt(xmlChar.attribute("y").value())).writeToStream(sOuputFile);
            Value((short)Parser::parseInt(xmlChar.attribute("w").value())).writeToStream(sOuputFile);
            Value((short)Parser::parseInt(xmlChar.attribute("h").value())).writeToStream(sOuputFile);
         }
      }

      sTextureFile.close();
   }

   sOuputFile.close();
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

      std::ofstream sOuputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOuputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Materials\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlMaterials = xml.child("Materials");
      GE::byte iMaterialsCount = 0;

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
         iMaterialsCount++;

      Value(iMaterialsCount).writeToStream(sOuputFile);

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
      {
         const char* sMaterialName = xmlMaterial.attribute("name").value();
         Value(sMaterialName).writeToStream(sOuputFile);
      }

      for(const pugi::xml_node& xmlMaterial : xmlMaterials.children("Material"))
      {
         const char* sMaterialName = xmlMaterial.attribute("name").value();
         Value(sMaterialName).writeToStream(sOuputFile);

         Material cMaterial(sMaterialName);
         cMaterial.xmlToStream(xmlMaterial, sOuputFile);
      }

      sOuputFile.close();
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

   std::ofstream sOuputFile(sOutputPath, std::ios::out | std::ios::binary);
   GEAssert(sOuputFile.is_open());

   char sInputPath[MAX_PATH];
   sprintf(sInputPath, "%s\\Fonts\\%s", ContentXmlDirName, XmlFileName);

   pugi::xml_document xml;
   xml.load_file(sInputPath);
   const pugi::xml_node& xmlFonts = xml.child("Fonts");
   GE::byte iFontsCount = 0;

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
      iFontsCount++;

   Value(iFontsCount).writeToStream(sOuputFile);

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
   {
      const char* sFontName = xmlFont.attribute("name").value();
      Value(sFontName).writeToStream(sOuputFile);
   }

   for(const pugi::xml_node& xmlFont : xmlFonts.children("Font"))
   {
      const char* sFontName = xmlFont.attribute("name").value();
      const char* sFontFileName = xmlFont.attribute("fileName").value();

      Value(sFontName).writeToStream(sOuputFile);

      std::string sFontFilePath;
      sFontFilePath.append(ContentXmlDirName);
      sFontFilePath.append("\\Fonts\\");
      sFontFilePath.append(sFontFileName);
      sFontFilePath.append(".fnt");

      pugi::xml_document xmlFontDesc;
      xmlFontDesc.load_file(sFontFilePath.c_str());
      const pugi::xml_node& xmlFontDescRoot = xmlFontDesc.child("font");

      const pugi::xml_node& xmlCommon = xmlFontDescRoot.child("common");
      Value((short)Parser::parseInt(xmlCommon.attribute("scaleW").value())).writeToStream(sOuputFile);
      Value((short)Parser::parseInt(xmlCommon.attribute("scaleH").value())).writeToStream(sOuputFile);

      const pugi::xml_node& xmlChars = xmlFontDescRoot.child("chars");
      short iFontCharsCount = (short)Parser::parseUInt(xmlChars.attribute("count").value());
      Value(iFontCharsCount).writeToStream(sOuputFile);

      for(const pugi::xml_node& xmlChar : xmlChars.children("char"))
      {
         Value((GE::byte)Parser::parseInt(xmlChar.attribute("id").value())).writeToStream(sOuputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("x").value())).writeToStream(sOuputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("y").value())).writeToStream(sOuputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("width").value())).writeToStream(sOuputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("height").value())).writeToStream(sOuputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("xoffset").value())).writeToStream(sOuputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("yoffset").value())).writeToStream(sOuputFile);
         Value((short)Parser::parseInt(xmlChar.attribute("xadvance").value())).writeToStream(sOuputFile);
      }

      const pugi::xml_node& xmlKernings = xmlFontDescRoot.child("kernings");
      short iFontKerningsCount = (short)Parser::parseUInt(xmlKernings.attribute("count").value());
      Value(iFontKerningsCount).writeToStream(sOuputFile);

      for(const pugi::xml_node& xmlKerning : xmlKernings.children("kerning"))
      {
         GE::byte iKerningFirstCharId = (GE::byte)Parser::parseUInt(xmlKerning.attribute("first").value());
         GE::byte iKerningSecondCharId = (GE::byte)Parser::parseUInt(xmlKerning.attribute("second").value());
         int iKerningAmount = Parser::parseInt(xmlKerning.attribute("amount").value());

         Value(iKerningFirstCharId).writeToStream(sOuputFile);
         Value(iKerningSecondCharId).writeToStream(sOuputFile);
         Value((short)iKerningAmount).writeToStream(sOuputFile);
      }

      sFontFilePath[sFontFilePath.length() - 3] = 'p';
      sFontFilePath[sFontFilePath.length() - 2] = 'n';
      sFontFilePath[sFontFilePath.length() - 1] = 'g';

      std::ifstream sFontTextureFile(sFontFilePath, std::ios::in | std::ios::binary);
      GEAssert(sFontTextureFile.is_open());
      sFontTextureFile.seekg(0, std::ios::end);
      uint iFontFileSize = (uint)sFontTextureFile.tellg();
      sFontTextureFile.seekg(0, std::ios::beg);

      Value(iFontFileSize).writeToStream(sOuputFile);
      std::copy_n(std::istreambuf_iterator<char>(sFontTextureFile), iFontFileSize, std::ostreambuf_iterator<char>(sOuputFile));

      sFontTextureFile.close();
   }

   sOuputFile.close();
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

      std::ofstream sOuputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOuputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Strings\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlStrings = xml.child("Strings");
      GE::byte iStringsCount = 0;

      for(const pugi::xml_node& xmlString : xmlStrings.children("String"))
         iStringsCount++;

      Value(iStringsCount).writeToStream(sOuputFile);

      for(const pugi::xml_node& xmlString : xmlStrings.children("String"))
      {
         const char* sStringID = xmlString.attribute("id").value();
         Value(sStringID).writeToStream(sOuputFile);
         const char* sStringText = xmlString.attribute("text").value();
         Value(sStringText).writeToStream(sOuputFile);
      }

      sOuputFile.close();
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

      std::ofstream sOuputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOuputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Models\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlSkeleton = xml.child("Skeleton");

      uint iBonesCount = Parser::parseUInt(xmlSkeleton.attribute("bonesCount").value());
      Value((GE::byte)iBonesCount).writeToStream(sOuputFile);

      for(pugi::xml_node_iterator it = xmlSkeleton.begin(); it != xmlSkeleton.end(); it++)
      {
         const pugi::xml_node& xmlBone = *it;
         
         ObjectName cBoneName = ObjectName(xmlBone.attribute("name").value());
         Value(cBoneName).writeToStream(sOuputFile);

         pugi::xml_attribute xmlBoneParentIndex = xmlBone.attribute("parentIndex");
         uint iBoneParentIndex = !xmlBoneParentIndex.empty()
            ? Parser::parseInt(xmlBoneParentIndex.value())
            : 0;
         Value((GE::byte)iBoneParentIndex).writeToStream(sOuputFile);

         Value(Parser::parseVector3(xmlBone.attribute("bindT").value())).writeToStream(sOuputFile);
         Value(Parser::parseVector3(xmlBone.attribute("bindR").value())).writeToStream(sOuputFile);
         Value(Parser::parseVector3(xmlBone.attribute("bindS").value())).writeToStream(sOuputFile);
         Value(Parser::parseFloat(xmlBone.attribute("size").value())).writeToStream(sOuputFile);

         const pugi::xml_node& xmlBoneChildren = xmlBone.child("Children");
         uint iBoneChildrenCount = 0;

         for(pugi::xml_node_iterator it2 = xmlBoneChildren.begin(); it2 != xmlBoneChildren.end(); it2++)
            iBoneChildrenCount++;

         Value((GE::byte)iBoneChildrenCount).writeToStream(sOuputFile);

         for(pugi::xml_node_iterator it2 = xmlBoneChildren.begin(); it2 != xmlBoneChildren.end(); it2++)
         {
            const pugi::xml_node& xmlBoneChild = *it2;
            Value((GE::byte)Parser::parseInt(xmlBoneChild.attribute("index").value())).writeToStream(sOuputFile);
         }
      }

      sOuputFile.close();
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

      std::ofstream sOuputFile(sOutputPath, std::ios::out | std::ios::binary);
      GEAssert(sOuputFile.is_open());

      char sInputPath[MAX_PATH];
      sprintf(sInputPath, "%s\\Animations\\%s", ContentXmlDirName, sXmlFileName);

      pugi::xml_document xml;
      xml.load_file(sInputPath);
      const pugi::xml_node& xmlAnimationSet = xml.child("AnimationSet");
      GE::byte iAnimationsCount = 0;

      for(const pugi::xml_node& xmlAnimation : xmlAnimationSet.children("Animation"))
         iAnimationsCount++;

      Value(iAnimationsCount).writeToStream(sOuputFile);

      for(const pugi::xml_node& xmlAnimation : xmlAnimationSet.children("Animation"))
      {
         Value(xmlAnimation.attribute("name").value()).writeToStream(sOuputFile);
         Value(xmlAnimation.attribute("fileName").value()).writeToStream(sOuputFile);

         pugi::xml_attribute xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionX");
         bool bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOuputFile);

         xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionY");
         bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOuputFile);

         xmlApplyRootMotion = xmlAnimation.attribute("applyRootMotionZ");
         bApplyRootMotion = xmlApplyRootMotion.empty() || Parser::parseBool(xmlApplyRootMotion.value());
         Value(bApplyRootMotion).writeToStream(sOuputFile);
      }

      sOuputFile.close();
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

void getBinFileName(const char* XmlFileName, char* BinFileName)
{
   uint iXmlFileNameLength = (uint)strlen(XmlFileName);
   strcpy(BinFileName, XmlFileName);
   BinFileName[iXmlFileNameLength - 3] = 'g';
   BinFileName[iXmlFileNameLength - 2] = 'e';
   BinFileName[iXmlFileNameLength - 1] = '\0';
}
