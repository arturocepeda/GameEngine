
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Audio (XAudio2)
//
//  --- GEAudioSystemXAudio2.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEAudioSystemXAudio2.h"
#include "Content/GEAudioData.h"
#include "Core/GEDevice.h"

using namespace GE;
using namespace GE::Core;
using namespace GE::Audio;
using namespace GE::Content;

static const UINT32 InputSampleRate = 44100;


//
//  SourceVoiceCallback
//
SourceVoiceCallback::SourceVoiceCallback(SourceVoice& SourceVoiceRef)
   : sSourceVoiceRef(SourceVoiceRef)
{
}

SourceVoiceCallback::~SourceVoiceCallback()
{
}

void SourceVoiceCallback::OnStreamEnd()
{
   sSourceVoiceRef.Playing = false;
}

void SourceVoiceCallback::OnVoiceProcessingPassEnd()
{
}

void SourceVoiceCallback::OnVoiceProcessingPassStart(UINT32 SamplesRequired)
{
}

void SourceVoiceCallback::OnBufferEnd(void* pBufferContext)
{
}

void SourceVoiceCallback::OnBufferStart(void* pBufferContext)
{
}

void SourceVoiceCallback::OnLoopEnd(void* pBufferContext)
{
}

void SourceVoiceCallback::OnVoiceError(void* pBufferContext, HRESULT Error)
{
}


//
//  AudioSystemXAudio2
//
AudioSystemXAudio2::AudioSystemXAudio2()
{
   XAudio2Create(&xaEngine);
   pHandler = xaEngine;
   xaEngine->CreateMasteringVoice(&xaMasteringVoice, XAUDIO2_DEFAULT_CHANNELS, InputSampleRate);
}

AudioSystemXAudio2::~AudioSystemXAudio2()
{
   xaMasteringVoice->DestroyVoice();
   xaEngine->Release();
}

void AudioSystemXAudio2::internalInit()
{
   memset(xaBuffers, 0, GE_AUDIO_SOUNDS * sizeof(XAUDIO2_BUFFER));

   WAVEFORMATEX xaWaveFormat;
   xaWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
   xaWaveFormat.nChannels = 1;
   xaWaveFormat.nSamplesPerSec = InputSampleRate;
   xaWaveFormat.nAvgBytesPerSec = InputSampleRate * 2;
   xaWaveFormat.nBlockAlign = 2;
   xaWaveFormat.wBitsPerSample = 16;
   xaWaveFormat.cbSize = 0;

   for(uint i = 0; i < GE_AUDIO_CHANNELS; i++)
   {
      xaSourceVoices[i].Callback = new SourceVoiceCallback(xaSourceVoices[i]);
      xaEngine->CreateSourceVoice(&xaSourceVoices[i].Handle, &xaWaveFormat, 0, 2.0f, xaSourceVoices[i].Callback);
   }
}

void AudioSystemXAudio2::release()
{
   unloadAllSounds();

   for(uint i = 0; i < GE_AUDIO_CHANNELS; i++)
   {
      xaSourceVoices[i].Handle->DestroyVoice();
      delete xaSourceVoices[i].Callback;
   }
}
   
void AudioSystemXAudio2::loadSound(unsigned int SoundIndex, const char* FileName, const char* FileExtension)
{
   AudioData cAudioData;
   Device::readContentFile(ContentType::Audio, "Sounds", FileName, FileExtension, &cAudioData);

   xaBuffers[SoundIndex].AudioBytes = cAudioData.getDataSize();
   xaBuffers[SoundIndex].pAudioData = new BYTE[cAudioData.getDataSize()];
   memcpy((void*)xaBuffers[SoundIndex].pAudioData, cAudioData.getData(), cAudioData.getDataSize());
   xaBuffers[SoundIndex].Flags = XAUDIO2_END_OF_STREAM;
}

void AudioSystemXAudio2::unloadSound(unsigned int SoundIndex)
{
   if(xaBuffers[SoundIndex].pAudioData)
      delete[] xaBuffers[SoundIndex].pAudioData;

   memset(&xaBuffers[SoundIndex], 0, sizeof(XAUDIO2_BUFFER));
}

void AudioSystemXAudio2::unloadAllSounds()
{
   for(uint i = 0; i < GE_AUDIO_SOUNDS; i++)
      unloadSound(i);
}
   
void AudioSystemXAudio2::internalPlaySound(unsigned int SoundIndex, unsigned int SourceIndex)
{
   internalStop(SourceIndex);

   xaSourceVoices[SourceIndex].Handle->SubmitSourceBuffer(&xaBuffers[SoundIndex]);
   xaSourceVoices[SourceIndex].Handle->Start();
   xaSourceVoices[SourceIndex].Playing = true;
}

void AudioSystemXAudio2::internalStop(unsigned int SourceIndex)
{
   xaSourceVoices[SourceIndex].Handle->Stop();
   xaSourceVoices[SourceIndex].Handle->FlushSourceBuffers();
   xaSourceVoices[SourceIndex].Playing = false;
}

bool AudioSystemXAudio2::isPlaying(unsigned int SourceIndex)
{
   return xaSourceVoices[SourceIndex].Playing;
}
   
void AudioSystemXAudio2::moveListener(const Vector3& Delta)
{
}

void AudioSystemXAudio2::moveSource(unsigned int SourceIndex, const Vector3& Delta)
{
}
   
void AudioSystemXAudio2::setListenerPosition(const Vector3& Position)
{
}

void AudioSystemXAudio2::setVolume(unsigned int SourceIndex, float Volume)
{
   xaSourceVoices[SourceIndex].Handle->SetVolume(Volume);
}

void AudioSystemXAudio2::setPosition(unsigned int SourceIndex, const Vector3& Position)
{
}

void AudioSystemXAudio2::setDirection(unsigned int SourceIndex, const Vector3& Direction)
{
}
