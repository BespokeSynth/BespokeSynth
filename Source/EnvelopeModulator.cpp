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
#include "UIControlMacros.h"

EnvelopeModulator::EnvelopeModulator()
{
   mAdsr.GetFreeReleaseLevel() = true;
}

void EnvelopeModulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);

   mAdsrDisplay = new ADSRDisplay(this, "adsr", 105, 2, 100, 66, &mAdsr);

   UIBLOCK0();
   FLOATSLIDER(mMinSlider, "low", &mDummyMin, 0, 1);
   FLOATSLIDER(mMaxSlider, "high", &mDummyMax, 0, 1);
   CHECKBOX(mUseVelocityCheckbox, "use velocity", &mUseVelocity);
   ENDUIBLOCK0();
}

EnvelopeModulator::~EnvelopeModulator()
{
}

void EnvelopeModulator::DrawModule()
{
   if (Minimized())
      return;

   mMinSlider->Draw();
   mMaxSlider->Draw();
   mUseVelocityCheckbox->Draw();

   mAdsrDisplay->Draw();
}

void EnvelopeModulator::Start(double time, const ::ADSR& adsr)
{
   mAdsr.Start(time, 1, adsr);
}

void EnvelopeModulator::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);

   if (mNoteOutput.HasHeldNotes() == false)
   {
      mAdsr.Stop(note.time);
   }
   else if (note.velocity > 0)
   {
      mAdsr.Start(note.time, mUseVelocity ? note.velocity / 127.0f : 1);
   }
}

void EnvelopeModulator::OnPulse(double time, float velocity, int flags)
{
   mAdsr.Start(time, mUseVelocity ? velocity / 127.0f : 1);
}

void EnvelopeModulator::GetModuleDimensions(float& width, float& height)
{
   width = 208;
   height = 73;
}

void EnvelopeModulator::Resize(float w, float h)
{
   mWidth = MAX(w, 250);
   mHeight = MAX(h, 102);
}

void EnvelopeModulator::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void EnvelopeModulator::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool EnvelopeModulator::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   return false;
}

float EnvelopeModulator::Value(int samplesIn /*= 0*/)
{
   ComputeSliders(samplesIn);
   if (GetSliderTarget())
      return ofClamp(Interp(mAdsr.Value(gTime + samplesIn * gInvSampleRateMs), GetMin(), GetMax()), GetSliderTarget()->GetMin(), GetSliderTarget()->GetMax());
   return 0;
}

void EnvelopeModulator::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
}

void EnvelopeModulator::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void EnvelopeModulator::ButtonClicked(ClickButton* button, double time)
{
}

void EnvelopeModulator::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void EnvelopeModulator::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void EnvelopeModulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void EnvelopeModulator::SetUpFromSaveData()
{
}

void EnvelopeModulator::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mAdsr.SaveState(out);
}

void EnvelopeModulator::LoadState(FileStreamIn& in, int rev)
{
   if (rev < 1)
   {
      // Temporary additional cable source
      mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
      mTargetCable->SetModulatorOwner(this);
      AddPatchCableSource(mTargetCable);
   }

   IDrawableModule::LoadState(in, rev);

   if (rev < 1)
   {
      const auto target = GetPatchCableSource(1)->GetTarget();
      if (target != nullptr)
         GetPatchCableSource()->SetTarget(target);
      RemovePatchCableSource(GetPatchCableSource(1));
      mTargetCable = GetPatchCableSource();
   }

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mAdsr.LoadState(in);
}
