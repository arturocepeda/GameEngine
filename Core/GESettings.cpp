
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

Settings::Settings()
   : Serializable("Settings")
   , mTargetFPS(0u)
   , mFullscreen(false)
   , mVSync(false)
   , mFullscreenSizeX(0u)
   , mFullscreenSizeY(0u)
   , mWindowSizeX(0u)
   , mWindowSizeY(0u)
   , mErrorPopUps(false)
   , mDumpLogs(false)
{
   mLanguage[0] = '\0';

   // General
   GERegisterProperty(UInt, TargetFPS);
   GERegisterProperty(String, Language);

   // Video
   GERegisterProperty(Bool, Fullscreen);
   GERegisterProperty(Bool, VSync);
   GERegisterProperty(UInt, FullscreenSizeX);
   GERegisterProperty(UInt, FullscreenSizeY);
   GERegisterProperty(UInt, WindowSizeX);
   GERegisterProperty(UInt, WindowSizeY);

   // Development
   GERegisterProperty(Bool, ErrorPopUps);
   GERegisterProperty(Bool, DumpLogs);
}

Settings::~Settings()
{
}

void Settings::load()
{
   const bool userSettingsLoaded = SerializableIO::loadFromXmlFile(this, ".", "settings");

   if(!userSettingsLoaded)
   {
      ContentData settingsData;
      Device::readContentFile(ContentType::GenericTextData, ".", "settings", "xml", &settingsData);

      pugi::xml_document xml;
      xml.load_buffer(settingsData.getData(), settingsData.getDataSize());
      pugi::xml_node xmlSettings = xml.child("Settings");

      loadFromXml(xmlSettings);
      save();
   }
}

void Settings::save()
{
   SerializableIO::saveToXmlFile(this, ".", "settings");
}
