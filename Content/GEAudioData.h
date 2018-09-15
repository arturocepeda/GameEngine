
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Content
//
//  --- GEAudioData.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "GEContentData.h"

namespace GE { namespace Content
{
   class AudioData : public ContentData
   {
   private:
      int iSampleRate;
      short iBitDepth;
      short iNumberOfChannels;

      void loadWAVData(uint32_t Size, const char* Data);
      void loadOggData(uint32_t Size, const char* Data);
    
   public:
      AudioData();

      virtual void load(uint Size, const char* Data) override;
    
      int getSampleRate();
      short getBitDepth();
      short getNumberOfChannels();
   };
}}
