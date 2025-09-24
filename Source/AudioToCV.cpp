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

    AudioToCV.cpp
    Created: 18 Nov 2017 10:46:05pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AudioToCV.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

AudioToCV::AudioToCV()
: IAudioProcessor(gBufferSize)
{
   mModulationBuffer = new float[gBufferSize];
}

void AudioToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mGainSlider = new FloatSlider(this, "gain", 3, 2, 100, 15, &mGain, 1, 10);
   mMinSlider = new FloatSlider(this, "min", mGainSlider, kAnchor_Below, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

AudioToCV::~AudioToCV()
{
   delete[] mModulationBuffer;
}

void AudioToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGainSlider->Draw();
   mMinSlider->Draw();
   mMaxSlider->Draw();

   ofPushStyle();
   ofSetColor(0, 255, 0, gModuleDrawAlpha);
   ofBeginShape();
   float x, y;
   float w, h;
   mGainSlider->GetPosition(x, y, K(local));
   mGainSlider->GetDimensions(w, h);
   for (int i = 0; i < gBufferSize; ++i)
   {
      ofVertex(ofMap(mModulationBuffer[i], -1, 1, x, x + w, K(clamp)), ofMap(i, 0, gBufferSize, y, y + h), K(clamp));
   }
   ofEndShape();
   ofPopStyle();
}

void AudioToCV::Process(double time)
{
   PROFILER(AudioToCV);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   assert(GetBuffer()->BufferSize());
   Clear(gWorkBuffer, gBufferSize);
   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      Add(gWorkBuffer, GetBuffer()->GetChannel(ch), gBufferSize);
   BufferCopy(mModulationBuffer, gWorkBuffer, gBufferSize);
   Mult(mModulationBuffer, mGain, gBufferSize);

   GetBuffer()->Reset();
}

void AudioToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

float AudioToCV::Value(int samplesIn)
{
   return ofMap(mModulationBuffer[samplesIn] / 2 + .5f, 0, 1, GetMin(), GetMax(), K(clamp));
}

void AudioToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void AudioToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void AudioToCV::SetUpFromSaveData()
{
}

void AudioToCV::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void AudioToCV::LoadState(FileStreamIn& in, int rev)
{
   if (rev < 1)
   {
      // Temporary additional cable source
      mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
      mTargetCableSource->SetModulatorOwner(this);
      AddPatchCableSource(mTargetCableSource);
   }

   IDrawableModule::LoadState(in, rev);

   if (rev < 1)
   {
      const auto target = GetPatchCableSource(1)->GetTarget();
      if (target != nullptr)
         GetPatchCableSource()->SetTarget(target);
      RemovePatchCableSource(GetPatchCableSource(1));
      mTargetCableSource = GetPatchCableSource();
   }

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());
}
