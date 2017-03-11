
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
    
   public:
      AudioData();

      virtual void load(unsigned int Size, const char* Data) override;
    
      int getSampleRate();
      short getBitDepth();
      short getNumberOfChannels();
   };
}}
