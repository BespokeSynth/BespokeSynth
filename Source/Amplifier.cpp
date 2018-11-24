//
//  Amplifier.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 7/13/13.
//
//

#include "Amplifier.h"
#include "ModularSynth.h"
#include "Profiler.h"

Amplifier::Amplifier()
: IAudioProcessor(gBufferSize)
, mGain(1)
, mGainSlider(nullptr)
{
}

void Amplifier::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGainSlider = new FloatSlider(this,"gain",5,2,110,15,&mGain,0,4);
}

Amplifier::~Amplifier()
{
}

void Amplifier::Process(double time)
{
   PROFILER(Amplifier);

   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   if (GetTarget())
   {
      ChannelBuffer* out = GetTarget()->GetBuffer();
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         Mult(GetBuffer()->GetChannel(ch), mGain*mGain, out->BufferSize());
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
      }
   }
   
   GetBuffer()->Reset();
}

void Amplifier::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGainSlider->Draw();
}

void Amplifier::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Amplifier::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}



