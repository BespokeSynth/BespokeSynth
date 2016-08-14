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
: mDryWet(1)
, mVolume(1)
, mDryWetSlider(NULL)
, mVolumeSlider(NULL)
, mPhase(0)
, mModOsc(kOsc_Sin)
, mGlideTime(0)
, mGlideSlider(NULL)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
   mDryBuffer = new float[mInputBufferSize];

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
   delete[] mInputBuffer;
   delete[] mDryBuffer;
}

float* RingModulator::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void RingModulator::Process(double time)
{
   Profiler profiler("RingModulator");

   if (GetTarget() == NULL)
      return;

   memcpy(mDryBuffer, mInputBuffer, mInputBufferSize*sizeof(float));

   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);

   if (mEnabled)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         ComputeSliders(0);
         
         mInputBuffer[i] *= mModOsc.Audio(time, mPhase);

         float phaseInc = GetPhaseInc(mFreq.Value(time));
         mPhase += phaseInc;
         while (mPhase > FTWO_PI) { mPhase -= FTWO_PI; }

         time += gInvSampleRateMs;
      }

      Mult(mDryBuffer, (1-mDryWet)*mVolume*mVolume, mInputBufferSize);
      Mult(mInputBuffer, mDryWet*mVolume*mVolume, mInputBufferSize);
      Add(mInputBuffer, mDryBuffer, bufferSize);
   }

   Add(out, mInputBuffer, bufferSize);

   GetVizBuffer()->WriteChunk(mInputBuffer, bufferSize);

   Clear(mInputBuffer, mInputBufferSize);
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


