/*
  ==============================================================================

    Inverter.cpp
    Created: 13 Nov 2019 10:16:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Inverter.h"
#include "ModularSynth.h"
#include "Profiler.h"

Inverter::Inverter()
: IAudioProcessor(gBufferSize)
{
}

void Inverter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();\
}

Inverter::~Inverter()
{
}

void Inverter::Process(double time)
{
   PROFILER(Inverter);

   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   if (GetTarget())
   {
      ChannelBuffer* out = GetTarget()->GetBuffer();
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         Mult(GetBuffer()->GetChannel(ch), -1, out->BufferSize());
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
      }
   }
   
   GetBuffer()->Reset();
}

void Inverter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void Inverter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Inverter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
