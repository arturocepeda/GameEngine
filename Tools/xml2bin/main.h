
#pragma once

#include "Types/GETypes.h"

void packTextures();
void packTextureFile(const char* XmlFileName);

void packMaterials();

void packFonts();
void packFontFile(const char* XmlFileName);

void packStrings();

void packMeshes();
void packSkeletons();
void packAnimations();

void getBinFileName(const char* XmlFileName, char* BinFileName);
