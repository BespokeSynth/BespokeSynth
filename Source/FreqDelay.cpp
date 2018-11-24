//
//  FreqDelay.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 5/10/13.
//
//

#include "FreqDelay.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"

FreqDelay::FreqDelay()
: IAudioProcessor(gBufferSize)
, mDryWet(1)
, mDryWetSlider(nullptr)
, mDryBuffer(gBufferSize)
{
   AddChild(&mDelayEffect);
   mDelayEffect.SetPosition(5,30);
   mDelayEffect.SetEnabled(true);
   mDelayEffect.SetDelay(15);
}

void FreqDelay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDryWetSlider = new FloatSlider(this, "dry/wet", 7, 2, 100, 15, &mDryWet, 0, 1);
   mDelayEffect.CreateUIControls();
   mDelayEffect.SetShortMode(true);
}

FreqDelay::~FreqDelay()
{
}

void FreqDelay::Process(double time)
{
   PROFILER(FreqDelay);

   if (GetTarget() == nullptr)
      return;
   
   SyncBuffers();
   mDryBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());

   int bufferSize = GetBuffer()->BufferSize();

   mDryBuffer.CopyFrom(GetBuffer());
   mDelayEffect.ProcessAudio(time, GetBuffer());
   
   for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
   {
      Mult(mDryBuffer.GetChannel(ch), (1-mDryWet), bufferSize);
      Mult(GetBuffer()->GetChannel(ch), mDryWet, bufferSize);
      Add(GetBuffer()->GetChannel(ch), mDryBuffer.GetChannel(ch), bufferSize);
      Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),bufferSize, ch);
   }

   GetBuffer()->Reset();
}

void FreqDelay::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      float freq = TheScale->PitchToFreq(pitch);
      float ms = 1000/freq;
      mDelayEffect.SetDelay(ms);
   }
}

void FreqDelay::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mDelayEffect.Draw();
   mDryWetSlider->Draw();
}

void FreqDelay::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   
}

void FreqDelay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void FreqDelay::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

