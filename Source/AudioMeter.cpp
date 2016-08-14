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
: mLevel(0)
, mLevelSlider(NULL)
, mMaxLevel(1)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
}

void AudioMeter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLevelSlider = new FloatSlider(this,"level",5,2,110,15,&mLevel,0,mMaxLevel);
}

AudioMeter::~AudioMeter()
{
   delete[] mInputBuffer;
}

float* AudioMeter::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void AudioMeter::Process(double time)
{
   Profiler profiler("AudioMeter");
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = gBufferSize;
   if (GetTarget())
   {
      float* out = GetTarget()->GetBuffer(bufferSize);
      assert(bufferSize == gBufferSize);
      
      Add(out, mInputBuffer, bufferSize);
   }
   
   mPeakTracker.Process(mInputBuffer, mInputBufferSize);
   mLevel = sqrtf(mPeakTracker.GetPeak());
   
   GetVizBuffer()->WriteChunk(mInputBuffer,bufferSize);
   
   Clear(mInputBuffer, mInputBufferSize);
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
