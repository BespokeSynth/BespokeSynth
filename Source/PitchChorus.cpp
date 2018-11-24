//
//  PitchChorus.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/19/15.
//
//

#include "PitchChorus.h"
#include "Profiler.h"
#include "Scale.h"
#include "SynthGlobals.h"

PitchChorus::PitchChorus()
: IAudioProcessor(gBufferSize)
, mPassthrough(true)
, mPassthroughCheckbox(nullptr)
{
   mOutputBuffer = new float[gBufferSize];
   Clear(mOutputBuffer, gBufferSize);
}

void PitchChorus::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPassthroughCheckbox = new Checkbox(this,"passthrough",4,4,&mPassthrough);
}

PitchChorus::~PitchChorus()
{
   delete[] mOutputBuffer;
}

void PitchChorus::Process(double time)
{
   PROFILER(PitchChorus);
   
   if (!mEnabled)
      return;
   
   ComputeSliders(0);
   SyncBuffers();
   
   int bufferSize = GetBuffer()->BufferSize();
   if (GetTarget())
   {
      Clear(mOutputBuffer, gBufferSize);
      for (int i=0; i<kNumShifters; ++i)
      {
         if (mShifters[i].mOn || mShifters[i].mRamp.Value(time) > 0)
         {
            BufferCopy(gWorkBuffer, GetBuffer()->GetChannel(0), bufferSize);
            mShifters[i].mShifter.Process(gWorkBuffer, bufferSize);
            double timeCopy = time;
            for (int j=0; j<bufferSize; ++j)
            {
               mOutputBuffer[j] += gWorkBuffer[j] * mShifters[i].mRamp.Value(timeCopy);
               timeCopy += gInvSampleRateMs;
            }
         }
      }
      
      if (mPassthrough)
         Add(mOutputBuffer, GetBuffer()->GetChannel(0), bufferSize);
      Add(GetTarget()->GetBuffer()->GetChannel(0), mOutputBuffer, bufferSize);
   }
   
   GetVizBuffer()->WriteChunk(mOutputBuffer,bufferSize, 0);
   
   GetBuffer()->Reset();
}

void PitchChorus::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   for (int i=0; i<kNumShifters; ++i)
   {
      if (velocity > 0 && mShifters[i].mOn == false)
      {
         float ratio = TheScale->PitchToFreq(pitch) / TheScale->PitchToFreq(60);
         mShifters[i].mOn = true;
         mShifters[i].mShifter.SetRatio(ratio);
         mShifters[i].mPitch = pitch;
         mShifters[i].mRamp.Start(time, 1, time + 100);
         break;
      }
      if (velocity == 0 && mShifters[i].mOn == true && mShifters[i].mPitch == pitch)
      {
         mShifters[i].mOn = false;
         mShifters[i].mRamp.Start(time, 0, time + 100);
         break;
      }
   }
}

void PitchChorus::DrawModule()
{

   
   if (Minimized() || IsVisible() == false)
      return;
   
   mPassthroughCheckbox->Draw();
}
