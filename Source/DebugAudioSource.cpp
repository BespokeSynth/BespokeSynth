//
//  DebugAudioSource.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 7/1/14.
//
//

#include "DebugAudioSource.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"

DebugAudioSource::DebugAudioSource()
{
}

DebugAudioSource::~DebugAudioSource()
{
}

void DebugAudioSource::Process(double time)
{
   Profiler profiler("DebugAudioSource");
   
   if (!mEnabled || GetTarget() == NULL)
      return;
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   for (int i=0; i<bufferSize; ++i)
   {
      float sample = 1;
      out[i] += sample;
      GetVizBuffer()->Write(sample);
   }
}

void DebugAudioSource::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
}

void DebugAudioSource::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void DebugAudioSource::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
