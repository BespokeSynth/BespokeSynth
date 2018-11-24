//
//  ControlTactileFeedback.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/9/14.
//
//

#include "ControlTactileFeedback.h"
#include "IAudioReceiver.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

ControlTactileFeedback::ControlTactileFeedback()
: mPhase(0)
, mPhaseInc(0)
, mVolume(.5f)
, mVolumeSlider(nullptr)
{
   mPhaseInc = GetPhaseInc(50);
}

void ControlTactileFeedback::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"vol",5,43,70,15,&mVolume,0,1);
}

ControlTactileFeedback::~ControlTactileFeedback()
{
}

void ControlTactileFeedback::Process(double time)
{
   PROFILER(ControlTactileFeedback);

   if (!mEnabled || GetTarget() == nullptr)
      return;
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   for (int i=0; i<bufferSize; ++i)
   {
      float sample = sample = (mPhase/FTWO_PI * 2 - 1) * gControlTactileFeedback * mVolume;
      out[i] += sample;
      GetVizBuffer()->Write(sample, 0);
      
      mPhase += mPhaseInc;
      while (mPhase > FTWO_PI) { mPhase -= FTWO_PI; }
      
      const float decayTime = .005f;
      float decay = powf( 0.5f, 1.0f/(decayTime * gSampleRate));
      gControlTactileFeedback *= decay;
      if (gControlTactileFeedback <= FLT_EPSILON)
         gControlTactileFeedback = 0;
      
      time += gInvSampleRateMs;
   }
}

void ControlTactileFeedback::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mVolumeSlider->Draw();
}


void ControlTactileFeedback::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ControlTactileFeedback::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

