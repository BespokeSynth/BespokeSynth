//
//  AudioMeter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/18/15.
//
//

#include "AudioMeter.h"
#include "Profiler.h"
#include "ModularSynth.h"

AudioMeter::AudioMeter()
: IAudioProcessor(gBufferSize)
, mLevel(0)
, mLevelSlider(nullptr)
, mMaxLevel(1)
{
   mAnalysisBuffer = new float[gBufferSize];
}

void AudioMeter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLevelSlider = new FloatSlider(this,"level",5,2,110,15,&mLevel,0,mMaxLevel);
}

AudioMeter::~AudioMeter()
{
   delete mAnalysisBuffer;
}

void AudioMeter::Process(double time)
{
   PROFILER(AudioMeter);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   Clear(mAnalysisBuffer, gBufferSize);
   
   for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
   {
      if (GetTarget())
         Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      Add(mAnalysisBuffer, GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
   }
   
   mPeakTracker.Process(mAnalysisBuffer, gBufferSize);
   mLevel = sqrtf(mPeakTracker.GetPeak());
   
   GetBuffer()->Reset();
}

void AudioMeter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mLevelSlider->Draw();
}

void AudioMeter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("maxlevel", moduleInfo, 1);
   
   SetUpFromSaveData();
}

void AudioMeter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mMaxLevel = mModuleSaveData.GetFloat("maxlevel");
   mLevelSlider->SetExtents(0, mMaxLevel);
}
