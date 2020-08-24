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
   
   SyncBuffers();
   int bufferSize = GetBuffer()->BufferSize();
   
   if (GetTarget())
   {
      ChannelBuffer* out = GetTarget()->GetBuffer();
      for (int ch=0; ch<out->NumActiveChannels(); ++ch)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            ComputeSliders(i);
            gWorkBuffer[i] = GetBuffer()->GetChannel(ch)[i] * mGain;
         }
         Add(out->GetChannel(ch), gWorkBuffer, GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(gWorkBuffer, GetBuffer()->BufferSize(), ch);
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



