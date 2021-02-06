
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Tools
//
//  --- GEContentCompiler.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEPlatform.h"

#if defined (GE_EDITOR_SUPPORT)

#define GE_HASH_OUTPUT_PATHS 1

#undef UNICODE

#include <windows.h>
#include <Shlobj.h>

#include "Core/GEApplication.h"

#include <functional>
#include <iostream>
#include <fstream>

namespace GE { namespace Tools
{
   class ContentCompiler
   {
   private:
      static const char* ContentXmlDirName;
      static const char* ContentBinDirName;

      static void getBinFileName(const char* pXmlFileName, char* pBinFileName);

      static void packShaders(Core::ApplicationRenderingAPI pRenderingAPI);
      static void packTextures();
      static void packTextureFile(const char* pXmlFileName);
      static void packMaterials();
      static void packFonts();
      static void packFontFile(const char* pXmlFileName);
      static void packStrings();
      static void packMeshes();
      static void packMeshFile(const char* pXmlFileName);
      static void packSkeletons();
      static void packAnimations();
      static void packSounds();
      static void packPrefabs();
      static void packScenes();
      static void packData();

      static void compileScripts();

   public:
      static std::function<void()> packAppResources;

      template<typename T>
      static void packSerializableResources()
      {
         char sFindString[MAX_PATH];
         sprintf(sFindString, "%s\\%s\\*.%s.xml", ContentXmlDirName, T::SubDir, T::Extension);

         WIN32_FIND_DATAA sFileData;
         HANDLE hFile = FindFirstFileA(sFindString, &sFileData);

         if(hFile == INVALID_HANDLE_VALUE)
            return;

         do
         {
            const char* sXmlFileName = sFileData.cFileName;

            char sInputPath[MAX_PATH];
            sprintf(sInputPath, "%s\\%s\\%s", ContentXmlDirName, T::SubDir, sXmlFileName);

            char extension[32];
            char extensionPattern[32];
            sprintf(extension, "%s.xml", T::Extension);
            sprintf(extensionPattern, ".%s.xml", T::Extension);              

            std::string groupName(sXmlFileName);
            const size_t extPos = groupName.find(extensionPattern);
            groupName.replace(extPos, groupName.length(), "");

            Content::ContentData contentData;
            Core::Device::readContentFile(ContentType::GenericTextData,
               T::SubDir, groupName.c_str(), extension, &contentData);

            char sRootNode[32];
            sprintf(sRootNode, "%sList", T::TypeName.getString());

            pugi::xml_document xml;
            xml.load_buffer(contentData.getData(), contentData.getDataSize());
            pugi::xml_node xmlEntries = xml.child(sRootNode);
            uint16_t entriesCount = 0u;

            for(const pugi::xml_node& xmlEntry : xmlEntries.children(T::TypeName.getString()))
            {
               entriesCount++;
            }

            char sBinFileName[MAX_PATH];
            getBinFileName(sXmlFileName, sBinFileName);

            char sOutputPath[MAX_PATH];
            GetCurrentDirectoryA(MAX_PATH, sOutputPath);
            sprintf(sOutputPath, "%s\\%s", sOutputPath, ContentBinDirName);
#if GE_HASH_OUTPUT_PATHS
            size_t outputPathOffset = strlen(sOutputPath) + 1u;
#endif
            CreateDirectoryA(sOutputPath, NULL);
            sprintf(sOutputPath, "%s\\%s", sOutputPath, T::SubDir);
#if GE_HASH_OUTPUT_PATHS
            Core::toHashPath(sOutputPath + outputPathOffset);
            outputPathOffset = strlen(sOutputPath) + 1u;
#endif
            CreateDirectoryA(sOutputPath, NULL);
            sprintf(sOutputPath, "%s\\%s", sOutputPath, sBinFileName);
#if GE_HASH_OUTPUT_PATHS
            Core::toHashPath(sOutputPath + outputPathOffset);
#endif

            std::ofstream sOutputFile(sOutputPath, std::ios::out | std::ios::binary);
            GEAssert(sOutputFile.is_open());

            Core::Value(entriesCount).writeToStream(sOutputFile);

            for(const pugi::xml_node& xmlEntry : xmlEntries.children(T::TypeName.getString()))
            {
               const char* entryNameStr = xmlEntry.attribute("name").value();
               const Core::ObjectName entryName(entryNameStr);

               Core::Value(entryName).writeToStream(sOutputFile);

               T entry(entryNameStr, groupName.c_str());
               entry.xmlToStream(xmlEntry, sOutputFile);
            }

            sOutputFile.close();
         }
         while(FindNextFileA(hFile, &sFileData));

         FindClose(hFile);
      }

      static void compileContent();
   };
}}

#endif
