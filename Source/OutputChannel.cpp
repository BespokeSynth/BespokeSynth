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
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   ClearBuffer();
}

OutputChannel::~OutputChannel()
{
   delete[] mInputBuffer;
}

void OutputChannel::ClearBuffer()
{
   Clear(mInputBuffer, mInputBufferSize);
}

float* OutputChannel::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
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

