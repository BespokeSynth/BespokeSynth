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

OutputChannel::OutputChannel()
: IAudioReceiver(gBufferSize)
{
}

OutputChannel::~OutputChannel()
{
}

void OutputChannel::Process()
{
   SyncInputBuffer();
}

void OutputChannel::ClearBuffer()
{
   GetBuffer()->Reset();
}

void OutputChannel::DrawModule()
{
}

void OutputChannel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("channel", moduleInfo, 1, 1, MAX_OUTPUT_CHANNELS);

   SetUpFromSaveData();
}

void OutputChannel::SetUpFromSaveData()
{
   mChannel = mModuleSaveData.GetInt("channel");
   if (TheSynth->SetOutputChannel(mChannel, this) == false)
      mChannel = -1;
}

