/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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
   mPassthroughCheckbox = new Checkbox(this, "passthrough", 4, 4, &mPassthrough);
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
   IAudioReceiver* target = GetTarget();
   if (target)
   {
      Clear(mOutputBuffer, gBufferSize);
      for (int i = 0; i < kNumShifters; ++i)
      {
         if (mShifters[i].mOn || mShifters[i].mRamp.Value(time) > 0)
         {
            BufferCopy(gWorkBuffer, GetBuffer()->GetChannel(0), bufferSize);
            mShifters[i].mShifter.Process(gWorkBuffer, bufferSize);
            double timeCopy = time;
            for (int j = 0; j < bufferSize; ++j)
            {
               mOutputBuffer[j] += gWorkBuffer[j] * mShifters[i].mRamp.Value(timeCopy);
               timeCopy += gInvSampleRateMs;
            }
         }
      }

      if (mPassthrough)
         Add(mOutputBuffer, GetBuffer()->GetChannel(0), bufferSize);
      Add(target->GetBuffer()->GetChannel(0), mOutputBuffer, bufferSize);
   }

   GetVizBuffer()->WriteChunk(mOutputBuffer, bufferSize, 0);

   GetBuffer()->Reset();
}

void PitchChorus::PlayNote(NoteMessage note)
{
   for (int i = 0; i < kNumShifters; ++i)
   {
      if (note.velocity > 0 && mShifters[i].mOn == false)
      {
         float ratio = TheScale->PitchToFreq(note.pitch) / TheScale->PitchToFreq(60);
         mShifters[i].mOn = true;
         mShifters[i].mShifter.SetRatio(ratio);
         mShifters[i].mPitch = note.pitch;
         mShifters[i].mRamp.Start(note.time, 1, note.time + 100);
         break;
      }
      if (note.velocity == 0 && mShifters[i].mOn == true && mShifters[i].mPitch == note.pitch)
      {
         mShifters[i].mOn = false;
         mShifters[i].mRamp.Start(note.time, 0, note.time + 100);
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
