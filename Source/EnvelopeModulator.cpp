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

    EnvelopeModulator.cpp
    Created: 16 Nov 2017 10:28:34pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "EnvelopeModulator.h"
#include "PatchCableSource.h"
#include "ModularSynth.h"

EnvelopeModulator::EnvelopeModulator()
: mEnvelopeControl(ofVec2f(105, 5), ofVec2f(mWidth - 110, mHeight - 10))
{
   mEnvelopeControl.SetViewLength(mADSRViewLength);
   mEnvelopeControl.SetADSR(&mAdsr);
   mAdsr.GetFreeReleaseLevel() = true;
   mAdsr.Set(10, 100, .5f, 100);
}

void EnvelopeModulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   GetPatchCableSource()->SetEnabled(false);

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);

   mAdvancedDisplayCheckbox = new Checkbox(this, "advanced", 2, 2, &mAdvancedDisplay);
   mAdsrDisplay = new ADSRDisplay(this, "adsr", 105, 2, 100, 66, &mAdsr);
   mMinSlider = new FloatSlider(this, "low", mAdvancedDisplayCheckbox, kAnchor_Below, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(this, "high", mMinSlider, kAnchor_Below, 100, 15, &mDummyMax, 0, 1);
   mADSRViewLengthSlider = new FloatSlider(this, "length", mMaxSlider, kAnchor_Below, 100, 15, &mADSRViewLength, 100, 10000);
   mUseVelocityCheckbox = new Checkbox(this, "use velocity", mADSRViewLengthSlider, kAnchor_Below, &mUseVelocity);
   mHasSustainStageCheckbox = new Checkbox(this, "has sustain", mUseVelocityCheckbox, kAnchor_Below, &mAdsr.GetHasSustainStage());
   mSustainStageSlider = new IntSlider(this, "sustain stage", mHasSustainStageCheckbox, kAnchor_Below, 100, 15, &mAdsr.GetSustainStage(), 1, MAX_ADSR_STAGES - 1);
   mMaxSustainSlider = new FloatSlider(this, "max sustain", mSustainStageSlider, kAnchor_Below, 100, 15, &mAdsr.GetMaxSustain(), -1, 5000);

   mADSRViewLengthSlider->SetMode(FloatSlider::kSquare);
   mMaxSustainSlider->SetMode(FloatSlider::kSquare);

   mSustainStageSlider->SetShowing(mAdsr.GetHasSustainStage());
   mMaxSustainSlider->SetShowing(mAdsr.GetHasSustainStage());

   mHasSustainStageCheckbox->SetShowing(mAdvancedDisplay);
   mSustainStageSlider->SetShowing(mAdvancedDisplay);
   mMaxSustainSlider->SetShowing(mAdvancedDisplay);
}

EnvelopeModulator::~EnvelopeModulator()
{
}

void EnvelopeModulator::DrawModule()
{
   if (Minimized())
      return;

   mSustainStageSlider->SetExtents(1, mAdsr.GetNumStages() - 2);

   mAdvancedDisplayCheckbox->Draw();
   mMinSlider->Draw();
   mMaxSlider->Draw();
   mADSRViewLengthSlider->Draw();
   mUseVelocityCheckbox->Draw();
   mHasSustainStageCheckbox->Draw();
   mSustainStageSlider->Draw();
   mMaxSustainSlider->Draw();

   mAdsrDisplay->Draw();

   if (mAdvancedDisplay)
      mEnvelopeControl.Draw();
}

void EnvelopeModulator::Start(double time, const ::ADSR& adsr)
{
   mAdsr.Start(time, 1, adsr);
}

void EnvelopeModulator::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

   if (mNoteOutput.HasHeldNotes() == false)
   {
      mAdsr.Stop(time);
   }
   else if (velocity > 0)
   {
      mAdsr.Start(time, mUseVelocity ? velocity / 127.0f : 1);
   }
}

void EnvelopeModulator::OnPulse(double time, float velocity, int flags)
{
   mAdsr.Start(time, mUseVelocity ? velocity / 127.0f : 1);
}

void EnvelopeModulator::GetModuleDimensions(float& width, float& height)
{
   if (mAdvancedDisplay)
   {
      width = mWidth;
      height = mHeight;
   }
   else
   {
      width = 208;
      height = 90;
   }
}

void EnvelopeModulator::Resize(float w, float h)
{
   mWidth = MAX(w, 250);
   mHeight = MAX(h, 102);
   mEnvelopeControl.SetDimensions(ofVec2f(mWidth - 110, mHeight - 10));
}

void EnvelopeModulator::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mEnvelopeControl.OnClicked(x, y, right);
}

void EnvelopeModulator::MouseReleased()
{
   IDrawableModule::MouseReleased();

   mEnvelopeControl.MouseReleased();
}

bool EnvelopeModulator::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   mEnvelopeControl.MouseMoved(x, y);

   return false;
}

float EnvelopeModulator::Value(int samplesIn /*= 0*/)
{
   ComputeSliders(samplesIn);
   if (mTarget)
      return ofClamp(Interp(mAdsr.Value(gTime + samplesIn * gInvSampleRateMs), GetMin(), GetMax()), mTarget->GetMin(), mTarget->GetMax());
   return 0;
}

void EnvelopeModulator::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void EnvelopeModulator::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mAdvancedDisplayCheckbox)
   {
      mAdsrDisplay->SetShowing(!mAdvancedDisplay);
      mHasSustainStageCheckbox->SetShowing(mAdvancedDisplay);
      mSustainStageSlider->SetShowing(mAdvancedDisplay);
      mMaxSustainSlider->SetShowing(mAdvancedDisplay);
   }
   if (checkbox == mHasSustainStageCheckbox)
   {
      mSustainStageSlider->SetShowing(mAdsr.GetHasSustainStage());
      mMaxSustainSlider->SetShowing(mAdsr.GetHasSustainStage());
   }
}

void EnvelopeModulator::ButtonClicked(ClickButton* button)
{
}

void EnvelopeModulator::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mADSRViewLengthSlider)
   {
      mEnvelopeControl.SetViewLength(mADSRViewLength);
      mAdsrDisplay->SetMaxTime(mADSRViewLength);
   }
}

void EnvelopeModulator::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);

   std::string targetPath = "";
   if (mTarget)
      targetPath = mTarget->Path();

   moduleInfo["target"] = targetPath;
}

void EnvelopeModulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void EnvelopeModulator::SetUpFromSaveData()
{
   mTargetCable->SetTarget(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}

namespace
{
   const int kSaveStateRev = 0;
}

void EnvelopeModulator::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);

   out << kSaveStateRev;

   mAdsr.SaveState(out);
}

void EnvelopeModulator::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);

   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);

   mAdsr.LoadState(in);
}
