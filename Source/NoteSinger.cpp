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
//  NoteSinger.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 5/23/13.
//
//

#include "NoteSinger.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

NoteSinger::NoteSinger()
: IAudioReceiver(gBufferSize)
{
   TheScale->AddListener(this);

   mWorkBuffer = new float[GetBuffer()->BufferSize()];
   Clear(mWorkBuffer, GetBuffer()->BufferSize());

   OnScaleChanged();
}

void NoteSinger::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void NoteSinger::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mOctaveSlider = new IntSlider(this, "oct", 35, 32, 60, 15, &mOctave, -2, 2);
}

NoteSinger::~NoteSinger()
{
   TheTransport->RemoveAudioPoller(this);
   TheScale->RemoveListener(this);
}

void NoteSinger::OnTransportAdvanced(float amount)
{
   PROFILER(NoteSinger);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncInputBuffer();

   int pitch = -1;

   int bestBucket = -1;
   float bestPeak = -1;
   for (int i = 0; i < mNumBuckets; ++i)
   {
      BufferCopy(mWorkBuffer, GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());
      mBands[i].Filter(mWorkBuffer, GetBuffer()->BufferSize());
      mPeaks[i].Process(mWorkBuffer, GetBuffer()->BufferSize());

      float peak = mPeaks[i].GetPeak();
      if (peak > bestPeak)
      {
         int numPitchesInScale = TheScale->NumTonesInScale();
         if (bestBucket != -1 &&
             peak < 2 * bestPeak &&
             i % numPitchesInScale == bestBucket % numPitchesInScale)
            continue; //don't let a harmonic beat a fundamental

         bestBucket = i;
         bestPeak = peak;
      }
   }
   assert(bestBucket != -1);

   pitch = GetPitchForBucket(bestBucket) + mOctave * 12;

   if (pitch != mPitch && bestPeak > .01f)
   {
      PlayNoteOutput(NoteMessage(gTime, pitch, 80));
      PlayNoteOutput(NoteMessage(gTime, mPitch, 0));
      mPitch = pitch;
   }

   GetBuffer()->Reset();
}

void NoteSinger::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mOctaveSlider->Draw();

   for (int i = 0; i < mNumBuckets; ++i)
   {
      float x = ofMap(i, 0, mNumBuckets, 0, 100);
      ofSetColor(255, 0, 255);
      ofLine(x, 50, x, 50 - ofMap(MIN(mPeaks[i].GetPeak(), 1), 0, 1, 0, 50));
   }
}

void NoteSinger::OnScaleChanged()
{
   mNumBuckets = MIN(TheScale->NumTonesInScale() * 4, NOTESINGER_MAX_BUCKETS);

   for (int i = 0; i < mNumBuckets; ++i)
   {
      int pitch = GetPitchForBucket(i);
      float f = TheScale->PitchToFreq(pitch);

      mBands[i].SetFilterType(kFilterType_Bandpass);
      mBands[i].SetFilterParams(f, 40 + mNumBuckets * 2 - i * 2);

      mPeaks[i].SetDecayTime(.05f);
   }
}

int NoteSinger::GetPitchForBucket(int bucket)
{
   return TheScale->GetPitchFromTone(bucket + TheScale->NumTonesInScale() * 2);
}

void NoteSinger::ButtonClicked(ClickButton* button, double time)
{
}

void NoteSinger::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      PlayNoteOutput(NoteMessage(time, mPitch, 0));
      mPitch = -1;
   }
}

void NoteSinger::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void NoteSinger::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
