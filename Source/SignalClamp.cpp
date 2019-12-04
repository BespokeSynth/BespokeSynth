/*
  ==============================================================================

    SignalClamp.cpp
    Created: 1 Dec 2019 3:24:55pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SignalClamp.h"
#include "ModularSynth.h"
#include "Profiler.h"

SignalClamp::SignalClamp()
: IAudioProcessor(gBufferSize)
, mMin(-1)
, mMinSlider(nullptr)
, mMax(1)
, mMaxSlider(nullptr)
{
}

void SignalClamp::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMinSlider = new FloatSlider(this,"min",5,2,110,15,&mMin,-2,2);
   mMaxSlider = new FloatSlider(this,"max",mMinSlider,kAnchor_Below,110,15,&mMax,-2,2);
}

SignalClamp::~SignalClamp()
{
}

void SignalClamp::Process(double time)
{
   PROFILER(SignalClamp);

   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   if (GetTarget())
   {
      int bufferSize = GetBuffer()->BufferSize();
      
      ChannelBuffer* out = GetTarget()->GetBuffer();
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         float* buffer = GetBuffer()->GetChannel(ch);
         for (int i=0; i<bufferSize; ++i)
         {
            ComputeSliders(i);
            buffer[i] = ofClamp(buffer[i], mMin, mMax);
         }
         Add(out->GetChannel(ch), buffer, bufferSize);
         GetVizBuffer()->WriteChunk(buffer, bufferSize, ch);
      }
   }
   
   GetBuffer()->Reset();
}

void SignalClamp::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mMinSlider->Draw();
   mMaxSlider->Draw();
}

void SignalClamp::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void SignalClamp::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
