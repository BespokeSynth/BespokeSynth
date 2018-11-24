//
//  RingModulator.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#include "RingModulator.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"

RingModulator::RingModulator()
: IAudioProcessor(gBufferSize)
, mDryWet(1)
, mVolume(1)
, mFreqSlider(nullptr)
, mDryWetSlider(nullptr)
, mVolumeSlider(nullptr)
, mPhase(0)
, mModOsc(kOsc_Sin)
, mGlideTime(0)
, mGlideSlider(nullptr)
, mDryBuffer(gBufferSize)
, mFreq(220)
{
   mModOsc.Start(gTime, 1);
   mFreqRamp.Start(gTime, 220, gTime + mGlideTime);
}

void RingModulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mFreqSlider = new FloatSlider(this,"freq", 5, 4, 120, 15, &mFreq, 20, 2000);
   mDryWetSlider = new FloatSlider(this,"dry/wet", 5, 20, 120, 15, &mDryWet, 0, 1);
   mVolumeSlider = new FloatSlider(this,"volume", 5, 36, 120, 15, &mVolume, 0, 2);
   mGlideSlider = new FloatSlider(this,"glide",5,52,120,15,&mGlideTime,0,1000);
   
   mFreqSlider->SetMode(FloatSlider::kLogarithmic);
}

RingModulator::~RingModulator()
{
}

void RingModulator::Process(double time)
{
   PROFILER(RingModulator);

   if (GetTarget() == nullptr)
      return;
   
   SyncBuffers();
   mDryBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());

   int bufferSize = GetTarget()->GetBuffer()->BufferSize();

   if (mEnabled)
   {
      mDryBuffer.CopyFrom(GetBuffer());
      
      for (int i=0; i<bufferSize; ++i)
      {
         ComputeSliders(0);
         
         for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
            GetBuffer()->GetChannel(ch)[i] *= mModOsc.Audio(time, mPhase);

         float phaseInc = GetPhaseInc(mFreqRamp.Value(time));
         mPhase += phaseInc;
         while (mPhase > FTWO_PI) { mPhase -= FTWO_PI; }

         time += gInvSampleRateMs;
      }
   }
   
   for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
   {
      if (mEnabled)
      {
         Mult(mDryBuffer.GetChannel(ch), (1-mDryWet)*mVolume*mVolume, GetBuffer()->BufferSize());
         Mult(GetBuffer()->GetChannel(ch), mDryWet*mVolume*mVolume, GetBuffer()->BufferSize());
         Add(GetBuffer()->GetChannel(ch), mDryBuffer.GetChannel(ch), GetBuffer()->BufferSize());
      }
      
      Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
   }

   GetBuffer()->Reset();
}

void RingModulator::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mFreqSlider->Draw();
   mVolumeSlider->Draw();
   mDryWetSlider->Draw();
   
   mGlideSlider->Draw();
}

void RingModulator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      float freq = TheScale->PitchToFreq(pitch);
      mFreqRamp.Start(gTime, freq, gTime + mGlideTime);
   }
}

void RingModulator::ButtonClicked(ClickButton* button)
{
}

void RingModulator::CheckboxUpdated(Checkbox* checkbox)
{
}

void RingModulator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mFreqSlider)
   {
      mFreqRamp.SetValue(mFreq);
   }
}

void RingModulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void RingModulator::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}


