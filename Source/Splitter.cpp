/*
  ==============================================================================

    Splitter.cpp
    Created: 10 Oct 2017 9:50:02pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "Splitter.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

Splitter::Splitter()
: IAudioProcessor(gBufferSize)
, mVizBuffer2(VIZ_BUFFER_SECONDS*gSampleRate)
{
}

void Splitter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   float w,h;
   GetDimensions(w, h);
   GetPatchCableSource()->SetManualPosition(w/2-15,h+3);
   GetPatchCableSource()->SetManualSide(PatchCableSource::kBottom);
   
   mPatchCableSource2 = new PatchCableSource(this, kConnectionType_Audio);
   mPatchCableSource2->SetManualPosition(w/2+15,h+3);
   mPatchCableSource2->SetOverrideVizBuffer(&mVizBuffer2);
   mPatchCableSource2->SetManualSide(PatchCableSource::kBottom);
   AddPatchCableSource(mPatchCableSource2);
}

Splitter::~Splitter()
{
}

void Splitter::Process(double time)
{
   PROFILER(Splitter);
   
   if (!mEnabled)
      return;
   
   if (GetTarget(0))
   {
      ChannelBuffer* out = GetTarget(0)->GetBuffer();
      Add(out->GetChannel(0), GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize(), 0);
   }
   
   int secondChannel = 1;
   if (GetBuffer()->NumActiveChannels() == 1)
      secondChannel = 0;
   if (GetTarget(1))
   {
      ChannelBuffer* out2 = GetTarget(1)->GetBuffer();
      Add(out2->GetChannel(0), GetBuffer()->GetChannel(secondChannel), GetBuffer()->BufferSize());
      mVizBuffer2.WriteChunk(GetBuffer()->GetChannel(secondChannel), GetBuffer()->BufferSize(), 0);
   }
   
   GetBuffer()->Reset();
}

void Splitter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void Splitter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("target2", moduleInfo);
   
   SetUpFromSaveData();
}

void Splitter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   IClickable* target2 = TheSynth->FindModule(mModuleSaveData.GetString("target2"));
   if (target2)
      mPatchCableSource2->AddPatchCable(target2);
}


