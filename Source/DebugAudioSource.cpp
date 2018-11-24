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
   PROFILER(DebugAudioSource);
   
   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   for (int i=0; i<bufferSize; ++i)
   {
      float sample = 1;
      out[i] += sample;
      GetVizBuffer()->Write(sample, 0);
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
