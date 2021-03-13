//
//  OutputChannel.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/17/12.
//
//

#include "OutputChannel.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

OutputChannel::OutputChannel()
: IAudioProcessor(gBufferSize)
, mChannelSelectionIndex(0)
{
}

OutputChannel::~OutputChannel()
{
}

void OutputChannel::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mChannelSelector = new DropdownList(this, "ch", 3, 3, &mChannelSelectionIndex);

   for (int i = 0; i < TheSynth->GetNumOutputChannels(); ++i)
      mChannelSelector->AddLabel(ofToString(i + 1), i);
   mStereoSelectionOffset = mChannelSelector->GetNumValues(); //after this, the stereo pairs
   for (int i = 0; i < TheSynth->GetNumOutputChannels()-1; ++i)
      mChannelSelector->AddLabel(ofToString(i + 1) + "&" + ofToString(i + 2), mChannelSelector->GetNumValues());
   mChannelSelector->DrawLabel(true);
   mChannelSelector->SetWidth(43);

   GetPatchCableSource()->SetEnabled(false);
}

void OutputChannel::Process(double time)
{
   SyncBuffers();

   int channelSelectionIndex = mChannelSelectionIndex;
   if (mChannelSelectionIndex < mStereoSelectionOffset)  //mono
   {
      int channel = channelSelectionIndex;
      if (channel >= 0 && channel < TheSynth->GetNumOutputChannels())
         Add(TheSynth->GetOutputBuffer(channel), GetBuffer()->GetChannel(0), gBufferSize);
   }
   else  //stereo
   {
      int channel1 = channelSelectionIndex - mStereoSelectionOffset;
      if (channel1 >= 0 && channel1 < TheSynth->GetNumOutputChannels())
         Add(TheSynth->GetOutputBuffer(channel1), GetBuffer()->GetChannel(0), gBufferSize);
      int channel2 = channel1 + 1;
      if (channel2 >= 0 && channel2 < TheSynth->GetNumOutputChannels())
         Add(TheSynth->GetOutputBuffer(channel2), GetBuffer()->GetChannel(1), gBufferSize);
   }

   GetBuffer()->Reset();
}

void OutputChannel::DrawModule()
{
   mChannelSelector->Draw();
}

void OutputChannel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("channel", moduleInfo, 0, 0, TheSynth->GetNumOutputChannels()-1);

   SetUpFromSaveData();
}

void OutputChannel::SetUpFromSaveData()
{
   mChannelSelectionIndex = mModuleSaveData.GetInt("channel");
}

