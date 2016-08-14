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
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
}

InputChannel::~InputChannel()
{
   delete[] mInputBuffer;
}

float* InputChannel::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void InputChannel::Process(double time)
{
   Profiler profiler("InputChannel");

   if (!mEnabled)
      return;
   
   int bufferSize = gBufferSize;
   if (GetTarget())
   {
      float* out = GetTarget()->GetBuffer(bufferSize);
      assert(bufferSize == gBufferSize);

      Mult(mInputBuffer, 4, bufferSize);
      Add(out, mInputBuffer, bufferSize);
   }
   
   GetVizBuffer()->WriteChunk(mInputBuffer,bufferSize);
   
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

