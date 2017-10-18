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
, mDryWetSlider(NULL)
{
   mDryBuffer = new float[GetBuffer()->BufferSize()];

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
   delete[] mDryBuffer;
}

void FreqDelay::Process(double time)
{
   Profiler profiler("FreqDelay");

   if (GetTarget() == NULL)
      return;
   
   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();

   memcpy(mDryBuffer, GetBuffer()->GetChannel(0), sizeof(float)*bufferSize);
   mDelayEffect.ProcessAudio(time, GetBuffer()->GetChannel(0), bufferSize);
   
   Mult(mDryBuffer, (1-mDryWet), bufferSize);
   Mult(GetBuffer()->GetChannel(0), mDryWet, bufferSize);
   Add(GetBuffer()->GetChannel(0), mDryBuffer, bufferSize);

   Add(GetTarget()->GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(0), bufferSize);

   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0),bufferSize);

   GetBuffer()->Clear();
}

void FreqDelay::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
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

