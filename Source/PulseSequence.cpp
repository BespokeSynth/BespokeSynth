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

    PulseSequence.cpp
    Created: 21 Oct 2018 11:26:09pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseSequence.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

PulseSequence::PulseSequence()
{
   for (int i = 0; i < kMaxSteps; ++i)
      mVels[i] = 1;
}

void PulseSequence::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), false);
   TheTransport->AddAudioPoller(this);
}

void PulseSequence::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLengthSlider = new IntSlider(this, "length", 3, 2, 96, 15, &mLength, 1, kMaxSteps);
   mIntervalSelector = new DropdownList(this, "interval", mLengthSlider, kAnchor_Right, (int*)(&mInterval));

   mVelocityGrid = new UIGrid(this, "uigrid", 3, 20, 248, 20, mLength, 1);

   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mIntervalSelector->AddLabel("none", kInterval_None);

   mAdvanceBackwardButton = new ClickButton(this, " - ", mIntervalSelector, kAnchor_Right);
   mAdvanceForwardButton = new ClickButton(this, "+", mAdvanceBackwardButton, kAnchor_Right);
   mPulseOnAdvanceCheckbox = new Checkbox(this, "pulse", mAdvanceForwardButton, kAnchor_Right, &mPulseOnAdvance);
   mShiftLeftButton = new ClickButton(this, "<", mPulseOnAdvanceCheckbox, kAnchor_Right);
   mShiftRightButton = new ClickButton(this, ">", mShiftLeftButton, kAnchor_Right);

   mVelocityGrid->SetGridMode(UIGrid::kMultisliderBipolar);
   mVelocityGrid->SetListener(this);
   mVelocityGrid->SetRequireShiftForMultislider(true);
   for (int i = 0; i < kMaxSteps; ++i)
      mVelocityGrid->SetVal(i, 0, mVels[i], !K(notifyListener));

   for (int i = 0; i < kIndividualStepCables; ++i)
   {
      mStepCables[i] = new PatchCableSource(this, kConnectionType_Pulse);
      mStepCables[i]->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
      AddPatchCableSource(mStepCables[i]);
   }
}

PulseSequence::~PulseSequence()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

void PulseSequence::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofSetColor(255, 255, 255, gModuleDrawAlpha);

   mIntervalSelector->SetShowing(!mHasExternalPulseSource);
   mAdvanceBackwardButton->SetShowing(mHasExternalPulseSource);
   mAdvanceForwardButton->SetShowing(mHasExternalPulseSource);
   mPulseOnAdvanceCheckbox->SetShowing(mHasExternalPulseSource);

   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mVelocityGrid->Draw();
   mAdvanceBackwardButton->Draw();
   mAdvanceForwardButton->Draw();
   mPulseOnAdvanceCheckbox->Draw();
   mShiftLeftButton->Draw();
   mShiftRightButton->Draw();

   for (int i = 0; i < kIndividualStepCables; ++i)
   {
      if (i < mLength)
      {
         ofVec2f pos = mVelocityGrid->GetCellPosition(i, 0) + mVelocityGrid->GetPosition(true);
         pos.x += mVelocityGrid->GetWidth() / float(mLength) * .5f;
         pos.y += mVelocityGrid->GetHeight() + 8;
         mStepCables[i]->SetManualPosition(pos.x, pos.y);
         mStepCables[i]->SetEnabled(true);
      }
      else
      {
         mStepCables[i]->SetEnabled(false);
      }
   }
}

void PulseSequence::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void PulseSequence::OnTransportAdvanced(float amount)
{
   PROFILER(PulseSequence);

   ComputeSliders(0);
}

void PulseSequence::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 1, 0);
}

void PulseSequence::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;
   Step(time, velocity, flags);
}

void PulseSequence::Step(double time, float velocity, int flags)
{
   if (!mEnabled)
      return;

   int direction = 1;
   if (flags & kPulseFlag_Backward)
      direction = -1;

   mStep = (mStep + direction + mLength) % mLength;

   if (flags & kPulseFlag_Reset)
      mStep = 0;
   else if (flags & kPulseFlag_Random)
      mStep = gRandom() % mLength;

   if (!mHasExternalPulseSource || (flags & kPulseFlag_SyncToTransport))
   {
      mStep = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, mLength);
   }

   if (flags & kPulseFlag_Align)
   {
      int stepsPerMeasure = TheTransport->GetStepsPerMeasure(this);
      int numMeasures = ceil(float(mLength) / stepsPerMeasure);
      int measure = TheTransport->GetMeasure(time) % numMeasures;
      mStep = ((TheTransport->GetQuantized(time, mTransportListenerInfo) % stepsPerMeasure) + measure * stepsPerMeasure) % mLength;
   }

   float v = mVels[mStep] * velocity;

   if (v > 0)
   {
      DispatchPulse(GetPatchCableSource(), time, v, 0);

      if (mStep < kIndividualStepCables)
         DispatchPulse(mStepCables[mStep], time, v, 0);
   }

   mVelocityGrid->SetHighlightCol(time, mStep);
}

void PulseSequence::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void PulseSequence::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mVelocityGrid->TestClick(x, y, right);
}

void PulseSequence::Resize(float w, float h)
{
   mWidth = MAX(w, 254);
   mHeight = MAX(h, 58);
   mVelocityGrid->SetDimensions(mWidth - 6, mHeight - 38);
}

void PulseSequence::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mVelocityGrid->MouseReleased();
}

bool PulseSequence::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mVelocityGrid->NotifyMouseMoved(x, y);
   return false;
}

bool PulseSequence::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   mVelocityGrid->NotifyMouseScrolled(x, y, scrollX, scrollY, isSmoothScroll, isInvertedScroll);
   return false;
}

void PulseSequence::ButtonClicked(ClickButton* button, double time)
{
   if (button == mAdvanceBackwardButton)
      Step(time, mPulseOnAdvance ? 1 : 0, kPulseFlag_Backward);
   if (button == mAdvanceForwardButton)
      Step(time, mPulseOnAdvance ? 1 : 0, 0);
   if (button == mShiftLeftButton || button == mShiftRightButton)
   {
      const int shift = (button == mShiftRightButton) ? 1 : -1;
      for (int row = 0; row < mVelocityGrid->GetRows(); ++row)
      {
         const int start = (shift == 1) ? mVelocityGrid->GetCols() - 1 : 0;
         const int end = (shift == 1) ? 0 : mVelocityGrid->GetCols() - 1;
         const float startVal = mVelocityGrid->GetVal(start, row);
         for (int col = start; col != end; col -= shift)
            mVelocityGrid->SetVal(col, row, mVelocityGrid->GetVal(col - shift, row));
         mVelocityGrid->SetVal(end, row, startVal);
      }
   }
}

void PulseSequence::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
}

void PulseSequence::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void PulseSequence::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mLengthSlider)
   {
      mLength = MIN(mLength, kMaxSteps);
      mVelocityGrid->SetGrid(mLength, 1);
      GridUpdated(mVelocityGrid, 0, 0, 0, 0);
   }
}

void PulseSequence::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mVelocityGrid)
   {
      for (int i = 0; i < mVelocityGrid->GetCols(); ++i)
         mVels[i] = mVelocityGrid->GetVal(i, 0);
   }
}

void PulseSequence::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mVelocityGrid->SaveState(out);
   out << mHasExternalPulseSource;
}

void PulseSequence::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mVelocityGrid->LoadState(in);
   GridUpdated(mVelocityGrid, 0, 0, 0, 0);

   if (rev >= 2)
      in >> mHasExternalPulseSource;
}

void PulseSequence::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void PulseSequence::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("width", moduleInfo, 254, 254, 999999, K(isTextField));
   mModuleSaveData.LoadInt("height", moduleInfo, 58, 58, 999999, K(isTextField));

   SetUpFromSaveData();
}

void PulseSequence::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   PulseSequence::Resize(mModuleSaveData.GetInt("width"),
                         mModuleSaveData.GetInt("height"));
}
