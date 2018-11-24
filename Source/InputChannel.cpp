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
{
}

InputChannel::~InputChannel()
{
}

void InputChannel::Process(double time)
{
   PROFILER(InputChannel);

   if (!mEnabled)
      return;
   
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   if (GetTarget())
   {
      float* out = GetTarget()->GetBuffer()->GetChannel(0);

      Mult(GetBuffer()->GetChannel(0), 4, bufferSize);
      Add(out, GetBuffer()->GetChannel(0), bufferSize);
   }
   
   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0),bufferSize, 0);
   
   //Clear(mInputBuffer, mInputBufferSize);
}

void InputChannel::DrawModule()
{

}

void InputChannel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("channel", moduleInfo, 1, 1, MAX_INPUT_CHANNELS, true);

   SetUpFromSaveData();
}

void InputChannel::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mChannel = mModuleSaveData.GetInt("channel");
   if (TheSynth->SetInputChannel(mChannel, this) == false)
      mChannel = -1;
}

