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

    AudioLevelToCV.cpp
    Created: 9 Oct 2018 10:26:30pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AudioLevelToCV.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

AudioLevelToCV::AudioLevelToCV()
: IAudioProcessor(gBufferSize)
{
   mModulationBuffer = new float[gBufferSize];
}

void AudioLevelToCV::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mGainSlider = new FloatSlider(this, "gain", 3, 2, 100, 15, &mGain, 1, 100);
   mAttackSlider = new FloatSlider(this, "attack", mGainSlider, kAnchor_Below, 100, 15, &mAttack, .01f, 1000);
   mReleaseSlider = new FloatSlider(this, "release", mAttackSlider, kAnchor_Below, 100, 15, &mRelease, .01f, 1000);
   mMinSlider = new FloatSlider(this, "min", mReleaseSlider, kAnchor_Below, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "max", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);

   mGainSlider->SetMode(FloatSlider::kSquare);
   mAttackSlider->SetMode(FloatSlider::kSquare);
   mReleaseSlider->SetMode(FloatSlider::kSquare);

   //update mAttackFactor and mReleaseFactor
   FloatSliderUpdated(mAttackSlider, 0, gTime);
   FloatSliderUpdated(mReleaseSlider, 0, gTime);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

AudioLevelToCV::~AudioLevelToCV()
{
   delete[] mModulationBuffer;
}

void AudioLevelToCV::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGainSlider->Draw();
   mAttackSlider->Draw();
   mReleaseSlider->Draw();
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
      ofVertex(ofMap(mModulationBuffer[i], 0, 1, x, x + w, K(clamp)), ofMap(i, 0, gBufferSize, y, y + h), K(clamp));
   }
   ofEndShape();
   ofPopStyle();
}

void AudioLevelToCV::Process(double time)
{
   PROFILER(AudioLevelToCV);

   if (!mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   assert(GetBuffer()->BufferSize());
   Clear(gWorkBuffer, gBufferSize);
   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      Add(gWorkBuffer, GetBuffer()->GetChannel(ch), gBufferSize);
   for (int i = 0; i < gBufferSize; ++i)
   {
      float sample = fabsf(gWorkBuffer[i]);
      if (sample > mVal)
         mVal = mAttackFactor * (mVal - sample) + sample;
      else
         mVal = mReleaseFactor * (mVal - sample) + sample;
      mModulationBuffer[i] = mVal * mGain;
   }

   GetBuffer()->Reset();
}

void AudioLevelToCV::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

float AudioLevelToCV::Value(int samplesIn)
{
   return ofMap(mModulationBuffer[samplesIn], 0, 1, GetMin(), GetMax(), K(clamp));
}

void AudioLevelToCV::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mAttackSlider)
      mAttackFactor = powf(.01f, 1.0f / (mAttack * gSampleRateMs));
   if (slider == mReleaseSlider)
      mReleaseFactor = powf(.01f, 1.0f / (mRelease * gSampleRateMs));
}

void AudioLevelToCV::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void AudioLevelToCV::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void AudioLevelToCV::SetUpFromSaveData()
{
}

void AudioLevelToCV::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void AudioLevelToCV::LoadState(FileStreamIn& in, int rev)
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
