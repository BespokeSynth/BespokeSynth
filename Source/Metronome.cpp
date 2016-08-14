//
//  Metronome.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/16/13.
//
//

#include "Metronome.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"

Metronome::Metronome()
: mOsc(kOsc_Sin)
, mPhase(0)
, mPhaseInc(0)
, mVolume(.5f)
, mVolumeSlider(NULL)
{
   SetEnabled(false);
   
   TheTransport->AddListener(this, kInterval_4n);
}

void Metronome::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"vol",5,18,70,15,&mVolume,0,1);
}

Metronome::~Metronome()
{
   TheTransport->RemoveListener(this);
}

void Metronome::Process(double time)
{
   Profiler profiler("Metronome");

   if (!mEnabled || GetTarget() == NULL)
      return;

   int bufferSize;   
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);

   for (int i=0; i<bufferSize; ++i)
   {
      float sample = mOsc.Audio(time, mPhase) * mVolume / 10;
      out[i] += sample;
      GetVizBuffer()->Write(sample);

      mPhase += mPhaseInc;
      while (mPhase > FTWO_PI) { mPhase -= FTWO_PI; }

      time += gInvSampleRateMs;
   }
}

void Metronome::OnTimeEvent(int samplesTo)
{
   int step = TheTransport->GetQuantized(0,kInterval_4n);
   if (step == 0)
   {
      mPhaseInc = GetPhaseInc(880);
      mOsc.Start(gTime,1,0,100,0,0);
   }
   else if (step == 2)
   {
      mPhaseInc = GetPhaseInc(480);
      mOsc.Start(gTime,1,0,70,0,0);
   }
   else
   {
      mPhaseInc = GetPhaseInc(440);
      mOsc.Start(gTime,.8f,0,50,0,0);
   }
}

void Metronome::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mVolumeSlider->Draw();
}

void Metronome::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Metronome::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

