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
//  VinylTempoControl.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/18/14.
//
//

#include "VinylTempoControl.h"
#include "OpenFrameworksPort.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "Profiler.h"

VinylTempoControl* TheVinylTempoControl = nullptr;

VinylTempoControl::VinylTempoControl()
: IAudioProcessor(gBufferSize)
, mVinylProcessor(gSampleRate)
{
   //mModulationBuffer = new float[gBufferSize];
}

VinylTempoControl::~VinylTempoControl()
{
   //delete[] mModulationBuffer;
}

void VinylTempoControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mUseVinylControlCheckbox = new Checkbox(this, "control", 4, 2, &mUseVinylControl);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

void VinylTempoControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mUseVinylControlCheckbox->Draw();

   if (CanStartVinylControl())
      DrawTextNormal(ofToString(mVinylProcessor.GetPitch(), 2), 60, 14);
}

void VinylTempoControl::Process(double time)
{
   PROFILER(VinylTempoControl);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   assert(GetBuffer()->BufferSize());

   if (GetBuffer()->NumActiveChannels() >= 2)
   {
      mVinylProcessor.Process(GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(1), gBufferSize);

      if (mUseVinylControl)
      {
         float speed = mVinylProcessor.GetPitch() / mReferencePitch;
         if (speed == 0 || mVinylProcessor.GetStopped())
            speed = .0001f;
         mSpeed = speed;
      }
      else
      {
         mReferencePitch = mVinylProcessor.GetPitch();
      }
   }

   GetBuffer()->Reset();
}

void VinylTempoControl::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

float VinylTempoControl::Value(int samplesIn)
{
   //return mModulationBuffer[samplesIn];
   return mSpeed;
}

bool VinylTempoControl::CanStartVinylControl()
{
   return !mVinylProcessor.GetStopped() && fabsf(mVinylProcessor.GetPitch()) > .001f;
}

void VinylTempoControl::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mUseVinylControlCheckbox)
   {
      if (!CanStartVinylControl())
         mUseVinylControl = false;
   }
}

void VinylTempoControl::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void VinylTempoControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void VinylTempoControl::SetUpFromSaveData()
{
}

void VinylTempoControl::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void VinylTempoControl::LoadState(FileStreamIn& in, int rev)
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

////////////////////////////////////////////////////////////////////////////////

VinylProcessor::VinylProcessor(int sampleRate)
: mSampleRate(sampleRate)
{
   struct timecode_def* def;

   def = timecoder_find_definition("serato_2a");
   assert(def != NULL);

   timecoder_init(&mTimecoder, def, 1.0, gSampleRate, false);
}

VinylProcessor::~VinylProcessor()
{
   timecoder_clear(&mTimecoder);
   timecoder_free_lookup();
}

//@TODO(Noxy): Warning C6262 Function uses '16448' bytes of stack : exceeds / analyze : stacksize '16384'. Consider moving some data to heap.
void VinylProcessor::Process(float* left, float* right, int numSamples)
{
   float* in[2];
   in[0] = left;
   in[1] = right;
   const float kConvert = (float)(1 << 15);
   signed short data[8196];

   for (int n = 0; n < numSamples; n++)
   {
      for (int ch = 0; ch < 2; ch++)
         data[n * 2 + ch] = (signed short)(kConvert * (float)in[ch][n]);
   }

   timecoder_submit(&mTimecoder, data, numSamples);

   mPitch = timecoder_get_pitch(&mTimecoder);
}
