//
//  InputChannel.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/16/12.
//
//

#include "InputChannel.h"
#include "ModularSynth.h"
#include "Profiler.h"

InputChannel::InputChannel()
: IAudioProcessor(gBufferSize)
, mChannelSelectionIndex(0)
{
}

InputChannel::~InputChannel()
{
}

void InputChannel::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mChannelSelector = new DropdownList(this, "ch", 3, 3, &mChannelSelectionIndex);

   for (int i = 0; i < TheSynth->GetNumInputChannels(); ++i)
      mChannelSelector->AddLabel(ofToString(i + 1), i);
   mStereoSelectionOffset = mChannelSelector->GetNumValues(); //after this, the stereo pairs
   for (int i = 0; i < TheSynth->GetNumInputChannels() - 1; ++i)
      mChannelSelector->AddLabel(ofToString(i + 1) + "&" + ofToString(i + 2), mChannelSelector->GetNumValues());
   mChannelSelector->DrawLabel(true);
}

void InputChannel::Process(double time)
{
   PROFILER(InputChannel);

   if (!mEnabled)
      return;
   
   int channelSelectionIndex = mChannelSelectionIndex;

   int numChannels = 1;
   if (mChannelSelectionIndex >= mStereoSelectionOffset)
      numChannels = 2;

   SyncBuffers(numChannels);
   
   if (mChannelSelectionIndex < mStereoSelectionOffset)  //mono
   {
      float* buffer = gZeroBuffer;
      int channel = mChannelSelectionIndex;
      if (channel >= 0 && channel < TheSynth->GetNumInputChannels())
         buffer = TheSynth->GetInputBuffer(channel);

      if (GetTarget())
         Add(GetTarget()->GetBuffer()->GetChannel(0), buffer, gBufferSize);

      GetVizBuffer()->WriteChunk(buffer, gBufferSize, 0);
   }
   else  //stereo
   {
      float* buffer1 = gZeroBuffer;
      float* buffer2 = gZeroBuffer;

      int channel1 = channelSelectionIndex - mStereoSelectionOffset;
      if (channel1 >= 0 && channel1 < TheSynth->GetNumInputChannels())
         buffer1 = TheSynth->GetInputBuffer(channel1);
      int channel2 = channel1 + 1;
      if (channel2 >= 0 && channel2 < TheSynth->GetNumInputChannels())
         buffer2 = TheSynth->GetInputBuffer(channel2);

      if (GetTarget())
      {
         Add(GetTarget()->GetBuffer()->GetChannel(0), buffer1, gBufferSize);
         Add(GetTarget()->GetBuffer()->GetChannel(1), buffer2, gBufferSize);
      }

      GetVizBuffer()->WriteChunk(buffer1, gBufferSize, 0);
      GetVizBuffer()->WriteChunk(buffer2, gBufferSize, 1);
   }
}

void InputChannel::DrawModule()
{
   mChannelSelector->Draw();
}

void InputChannel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void InputChannel::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

