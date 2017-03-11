
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio System (OpenAL)
//
//  --- GEAudioSystemOpenAL.mm ---
//
//////////////////////////////////////////////////////////////////

#include "GEAudioSystemOpenAL.h"
#include <iostream>
#include <fstream>

namespace GE { namespace Audio
{
   void AudioSystemOpenAL::internalInit()
   {
      alDevice = alcOpenDevice(NULL);
      alContext = alcCreateContext(alDevice, NULL);
      alcMakeContextCurrent(alContext);
         
      alGenBuffers(iSounds, alBuffers);
      alGenSources(iChannels, alSourcesAlloc);
      
      for(int i = 0; i < iChannels; i++)
         alSources[i].Source = alSourcesAlloc[i];
      
      setListenerPosition(Vector3(0.0f, 0.0f, 0.0f));
   }

   void AudioSystemOpenAL::update()
   {
      for(unsigned int i = 0; i < iChannels; i++)
      {
         if(!sChannels[i].Free)
         {
            ALenum alState;
            alGetSourcei(alSources[i].Source, AL_SOURCE_STATE, &alState);
            
            if(alState == AL_STOPPED)
               sChannels[i].Free = true;
         }
      }
   }

   void AudioSystemOpenAL::release()
   {
      alDeleteSources(iSounds, alSourcesAlloc);
      alDeleteBuffers(iChannels, alBuffers);

      alcMakeContextCurrent(NULL);
      alcDestroyContext(alContext);
      alcCloseDevice(alDevice);
   }

   void AudioSystemOpenAL::loadSound(unsigned int SoundIndex, const char* FileName, const char* FileExtension)
   {
      if(SoundIndex >= iChannels)
         return;
      
      char sFileName[32];
      sprintf(sFileName, "%s.%s", FileName, FileExtension);
      
      NSString* nsSoundFile = [NSString stringWithUTF8String: sFileName];   
      NSString* nsSoundFilePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: nsSoundFile];
      const char* sSoundFile = [nsSoundFilePath UTF8String];
      std::ifstream iFile(sSoundFile);
      
      if(!iFile)
         return;

      CFURLRef cfFileURL = (CFURLRef)[NSURL fileURLWithPath: nsSoundFilePath];

      ALenum alFormat;
      ALsizei alSize;
      ALsizei alFrequency;
      ALchar* alData = NULL;
      bool bSuccess;

      // load audio file
      bSuccess = loadAudioFile(cfFileURL, &alFormat, (void**)&alData, &alSize, &alFrequency);
      
      // load audio data into the specified buffer
      if(bSuccess)
         alBufferData(alBuffers[SoundIndex], alFormat, alData, alSize, alFrequency);

      if(alData)
         free(alData);
   }

   void AudioSystemOpenAL::unloadSound(unsigned int SoundIndex)
   {
      if(SoundIndex < iSounds)
         alBufferData(alBuffers[SoundIndex], 0, NULL, 0, 0);
   }

   void AudioSystemOpenAL::internalPlaySound(unsigned int SoundIndex, unsigned int SourceIndex)
   {
      alSourcei(alSources[SourceIndex].Source, AL_BUFFER, alBuffers[SoundIndex]);
      alSourcePlay(alSources[SourceIndex].Source);
   }

   void AudioSystemOpenAL::internalStop(unsigned int SourceIndex)
   {
      alSourceStop(alSources[SourceIndex].Source);
   }

   bool AudioSystemOpenAL::isPlaying(unsigned int SourceIndex)
   {
      ALenum alState;    
      alGetSourcei(SourceIndex, AL_SOURCE_STATE, &alState);
       
      return alState == AL_PLAYING;
   }

   void AudioSystemOpenAL::moveListener(const Vector3& Delta)
   {
      float fCurrentX, fCurrentY, fCurrentZ;

      alGetListener3f(AL_POSITION, &fCurrentX, &fCurrentY, &fCurrentZ);
      
      fCurrentX += Delta.X;
      fCurrentY += Delta.Y;
      fCurrentZ += Delta.Z;
      
      alListener3f(AL_POSITION, fCurrentX, fCurrentY, fCurrentZ);
   }

   void AudioSystemOpenAL::moveSource(unsigned int SourceIndex, const Vector3& Delta)
   {
      float fCurrentX, fCurrentY, fCurrentZ;

      alGetSource3f(alSources[SourceIndex].Source, AL_POSITION, &fCurrentX, &fCurrentY, &fCurrentZ);
      
      fCurrentX += Delta.X;
      fCurrentY += Delta.Y;
      fCurrentZ += Delta.Z;
      
      alSource3f(alSources[SourceIndex].Source, AL_POSITION, fCurrentX, fCurrentY, fCurrentZ);
   }

   void AudioSystemOpenAL::setListenerPosition(const Vector3& Position)
   {
      alListener3f(AL_POSITION, Position.X, Position.Y, Position.Z);
   }

   void AudioSystemOpenAL::setVolume(unsigned int SourceIndex, float Volume)
   {
      alSourcef(alSources[SourceIndex].Source, AL_GAIN, Volume);
   }

   void AudioSystemOpenAL::setPosition(unsigned int SourceIndex, const Vector3& Position)
   {
      alSource3f(alSources[SourceIndex].Source, AL_POSITION, Position.X, Position.Y, Position.Z);
   }

   void AudioSystemOpenAL::setDirection(unsigned int SourceIndex, const Vector3& Direction)
   {
      alSource3f(alSources[SourceIndex].Source, AL_DIRECTION, Direction.X, Direction.Y, Direction.Z);
   }

   bool AudioSystemOpenAL::loadAudioFile(CFURLRef cfURL, ALenum* alFormat, ALvoid** alData, ALsizei* alSize,
                                         ALsizei* alFrequency)
   {
      ExtAudioFileRef sAudioFileRef;
      OSStatus osError;
      
      // get file reference
      osError = ExtAudioFileOpenURL(cfURL, &sAudioFileRef);

      if(osError != noErr)
         return false;

      // get the number of audio frames from the file
      UInt32 iSize;
      UInt64 iFileLengthFrames; 

      iSize = sizeof(UInt64);

      osError = ExtAudioFileGetProperty(sAudioFileRef, kExtAudioFileProperty_FileLengthFrames, 
                                        &iSize, (void*)&iFileLengthFrames);

      // get the description of the audio file
      AudioStreamBasicDescription sDescription;

      iSize = sizeof(AudioStreamBasicDescription);

      osError = ExtAudioFileGetProperty(sAudioFileRef, kExtAudioFileProperty_FileDataFormat, 
                                        &iSize, (void*)&sDescription);

      // define the new PCM format
      sDescription.mFormatID = kAudioFormatLinearPCM;
      sDescription.mFormatFlags = kAudioFormatFlagsNativeEndian |
                                  kAudioFormatFlagIsSignedInteger |
                                  kAudioFormatFlagIsPacked;

      if(sDescription.mChannelsPerFrame > 2)
         sDescription.mChannelsPerFrame = 2;

      sDescription.mBitsPerChannel = 16;   
      sDescription.mBytesPerFrame = sDescription.mChannelsPerFrame * sDescription.mBitsPerChannel / 8;
      sDescription.mFramesPerPacket = 1;
      sDescription.mBytesPerPacket = sDescription.mBytesPerFrame * sDescription.mFramesPerPacket;
      
      // apply the new format
      osError = ExtAudioFileSetProperty(sAudioFileRef, kExtAudioFileProperty_ClientDataFormat,
                                        iSize, &sDescription);
      if(osError != noErr)
         return false;

      // load audio data from the file
      UInt32 iStreamSizeInBytes = (UInt32)(sDescription.mBytesPerFrame * iFileLengthFrames);

      *alData = malloc(iStreamSizeInBytes);

      if(!(*alData))
         return false;

      AudioBufferList sBufferList;
      UInt32 iNumFramesRead = (UInt32)iFileLengthFrames;
      
      sBufferList.mNumberBuffers = 1;
      sBufferList.mBuffers[0].mNumberChannels = sDescription.mChannelsPerFrame;  
      sBufferList.mBuffers[0].mDataByteSize = sDescription.mBytesPerFrame * iNumFramesRead;
      sBufferList.mBuffers[0].mData = (char*)*alData;
      
      osError = ExtAudioFileRead(sAudioFileRef, &iNumFramesRead, &sBufferList);
      
      if(osError != noErr)
      {
         free(*alData);
         return false;
      }
      
      // information for OpenAL buffer
      *alFormat = (sDescription.mChannelsPerFrame == 2)? AL_FORMAT_STEREO16: AL_FORMAT_MONO16;
      *alFrequency = (ALsizei)sDescription.mSampleRate;
      *alSize = iNumFramesRead * sDescription.mBytesPerFrame;

      return true;
   }   
}}
