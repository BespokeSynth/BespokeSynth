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
/*
  ==============================================================================

    ZeroCrossRate.cpp
    Created: 9 Oct 2018 10:26:30pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ZeroCrossRate.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ZeroCrossRate::ZeroCrossRate()
: IAudioProcessor(gBufferSize)
{
   mModulationBuffer = new float[gBufferSize];
   mZCRBuffer = new bool[ZCR_WINDOW_SIZE];
   for (int i = 0; i < ZCR_WINDOW_SIZE; i++)
      mZCRBuffer[i] = false;
}

void ZeroCrossRate::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

ZeroCrossRate::~ZeroCrossRate()
{
   delete[] mModulationBuffer;
   delete[] mZCRBuffer;
}

void ZeroCrossRate::DrawModule()
{
}

void ZeroCrossRate::Process(double time)
{
   PROFILER(ZeroCrossRate);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   for (int i = 0; i < gBufferSize; ++i)
   {
      bool sign = GetBuffer()->GetChannel(0)[i] >= 0;
      bool sign_changed = sign != mLastSign;
      mZCRCount = mZCRCount + sign_changed - mZCRBuffer[mZCRBufferPosition];
      mZCRBuffer[mZCRBufferPosition] = sign_changed;
      mZCRBufferPosition = (mZCRBufferPosition + 1) % ZCR_WINDOW_SIZE;
      mLastSign = sign;
      mModulationBuffer[i] = (float)mZCRCount / ZCR_WINDOW_SIZE;
   }

   GetBuffer()->Reset();
}

void ZeroCrossRate::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

float ZeroCrossRate::Value(int samplesIn)
{
   return ofMap(mModulationBuffer[samplesIn], 0, 1, GetMin(), GetMax(), K(clamp));
}

void ZeroCrossRate::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ZeroCrossRate::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ZeroCrossRate::SetUpFromSaveData()
{
}

void ZeroCrossRate::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void ZeroCrossRate::LoadState(FileStreamIn& in, int rev)
{
   if (rev < 1)
   {
      // Temporary additional cable source
      mTargetCableSource = new PatchCableSource(this, kConnectionType_Audio);
      mTargetCableSource->SetModulatorOwner(this);
      AddPatchCableSource(mTargetCableSource);
   }

   IDrawableModule::LoadState(in, rev);

   if (rev < 1)
   {
      auto target = GetPatchCableSource(1)->GetTarget();
      if (target != nullptr)
         GetPatchCableSource()->SetTarget(target);
      RemovePatchCableSource(GetPatchCableSource(1));
      mTargetCableSource = GetPatchCableSource();
   }

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());
}
