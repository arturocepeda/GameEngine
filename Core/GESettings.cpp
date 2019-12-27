
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GESettings.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GESettings.h"
#include "GEDevice.h"
#include "Content/GEContentData.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Content;

const ObjectName ClassName("Settings");

Settings::Settings()
   : Serializable(ClassName)
   , mTargetFPS(60u)
   , mFullscreen(false)
   , mVSync(false)
   , mWindowSizeX(1024u)
   , mWindowSizeY(600u)
{
   GERegisterProperty(UInt, TargetFPS);
   GERegisterProperty(Bool, Fullscreen);
   GERegisterProperty(Bool, VSync);
   GERegisterProperty(UInt, WindowSizeX);
   GERegisterProperty(UInt, WindowSizeY);
}

Settings::~Settings()
{
}

void Settings::load()
{
   ContentData settingsData;
   Device::readContentFile(ContentType::GenericTextData, ".", "settings", "xml", &settingsData);

   pugi::xml_document xml;
   xml.load_buffer(settingsData.getData(), settingsData.getDataSize());
   pugi::xml_node xmlSettings = xml.child("Settings");

   loadFromXml(xmlSettings);
}
