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
, mDryWetSlider(NULL)
, mVolumeSlider(NULL)
, mPhase(0)
, mModOsc(kOsc_Sin)
, mGlideTime(0)
, mGlideSlider(NULL)
{
   mDryBuffer = new float[gBufferSize];

   mModOsc.Start(gTime, 1);
   mFreq.Start(gTime, 220, gTime + mGlideTime);
}

void RingModulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDryWetSlider = new FloatSlider(this,"dry/wet", 5, 4, 120, 15, &mDryWet, 0, 1);
   mVolumeSlider = new FloatSlider(this,"volume", 5, 20, 120, 15, &mVolume, 0, 2);
   mGlideSlider = new FloatSlider(this,"glide",5,36,120,15,&mGlideTime,0,1000);
}

RingModulator::~RingModulator()
{
   delete[] mDryBuffer;
}

void RingModulator::Process(double time)
{
   Profiler profiler("RingModulator");

   if (GetTarget() == NULL)
      return;
   
   SyncBuffers();

   memcpy(mDryBuffer, GetBuffer()->GetChannel(0), GetBuffer()->BufferSize()*sizeof(float));

   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);

   if (mEnabled)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         ComputeSliders(0);
         
         GetBuffer()->GetChannel(0)[i] *= mModOsc.Audio(time, mPhase);

         float phaseInc = GetPhaseInc(mFreq.Value(time));
         mPhase += phaseInc;
         while (mPhase > FTWO_PI) { mPhase -= FTWO_PI; }

         time += gInvSampleRateMs;
      }

      Mult(mDryBuffer, (1-mDryWet)*mVolume*mVolume, GetBuffer()->BufferSize());
      Mult(GetBuffer()->GetChannel(0), mDryWet*mVolume*mVolume, GetBuffer()->BufferSize());
      Add(GetBuffer()->GetChannel(0), mDryBuffer, GetBuffer()->BufferSize());
   }

   Add(out, GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());

   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());

   GetBuffer()->Clear();
}

void RingModulator::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mVolumeSlider->Draw();
   mDryWetSlider->Draw();
   
   mGlideSlider->Draw();
}

void RingModulator::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (velocity > 0)
   {
      float freq = TheScale->PitchToFreq(pitch);
      mFreq.Start(gTime, freq, gTime + mGlideTime);
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
}

void RingModulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("enabled", moduleInfo,true);

   SetUpFromSaveData();
}

void RingModulator::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetEnabled(mModuleSaveData.GetBool("enabled"));
}


