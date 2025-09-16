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

    PulseTrain.cpp
    Created: 10 Mar 2020 9:15:47pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PulseTrain.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

PulseTrain::PulseTrain()
{
   for (int i = 0; i < kMaxSteps; ++i)
      mVels[i] = 1;
}

void PulseTrain::Init()
{
   IDrawableModule::Init();

   TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), false);
   TheTransport->AddAudioPoller(this);
}

void PulseTrain::CreateUIControls()
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

   mVelocityGrid->SetGridMode(UIGrid::kMultisliderBipolar);
   mVelocityGrid->SetRequireShiftForMultislider(true);
   mVelocityGrid->SetListener(this);
   for (int i = 0; i < kMaxSteps; ++i)
      mVelocityGrid->SetVal(i, 0, mVels[i], !K(notifyListener));

   for (int i = 0; i < kMaxSteps; ++i)
   {
      mStepCables[i] = new PatchCableSource(this, kConnectionType_Pulse);
      mStepCables[i]->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
      AddPatchCableSource(mStepCables[i]);
   }
}

PulseTrain::~PulseTrain()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

void PulseTrain::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofSetColor(255, 255, 255, gModuleDrawAlpha);

   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mVelocityGrid->Draw();

   for (int i = 0; i < kMaxSteps; ++i)
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

void PulseTrain::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void PulseTrain::OnTransportAdvanced(float amount)
{
   PROFILER(PulseTrain);

   ComputeSliders(0);
}

void PulseTrain::OnTimeEvent(double time)
{
   Step(time, 1, 0);
}

void PulseTrain::OnPulse(double time, float velocity, int flags)
{
   mStep = 0;
}

void PulseTrain::Step(double time, float velocity, int flags)
{
   if (!mEnabled)
      return;

   const bool isReset = (flags & kPulseFlag_Reset);
   if (isReset)
      mStep = 0;

   if (mStep >= mLength)
      return;

   if (mStep < mLength)
   {
      float v = mVels[mStep] * velocity;

      int new_flags = 0;
      if (mResetOnStart && mStep == 0)
         new_flags = kPulseFlag_Reset;

      if (v > 0)
      {
         DispatchPulse(GetPatchCableSource(), time, v, new_flags);

         if (mStep < kMaxSteps)
            DispatchPulse(mStepCables[mStep], time, v, new_flags);
      }
   }

   if (mStep < mLength)
      mVelocityGrid->SetHighlightCol(time, mStep);
   else
      mVelocityGrid->SetHighlightCol(time, -1);

   ++mStep;
}

void PulseTrain::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void PulseTrain::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mVelocityGrid->TestClick(x, y, right);
}

void PulseTrain::Resize(float w, float h)
{
   mWidth = MAX(w, 254);
   mHeight = MAX(h, 58);
   mVelocityGrid->SetDimensions(mWidth - 6, mHeight - 38);
}

void PulseTrain::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mVelocityGrid->MouseReleased();
}

bool PulseTrain::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   mVelocityGrid->NotifyMouseMoved(x, y);
   return false;
}

bool PulseTrain::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   mVelocityGrid->NotifyMouseScrolled(x, y, scrollX, scrollY, isSmoothScroll, isInvertedScroll);
   return false;
}

void PulseTrain::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
}

void PulseTrain::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void PulseTrain::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mLengthSlider)
   {
      if (mLength > kMaxSteps)
         mLength = kMaxSteps;
      mVelocityGrid->SetGrid(mLength, 1);
      GridUpdated(mVelocityGrid, 0, 0, 0, 0);
   }
}

void PulseTrain::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mVelocityGrid)
   {
      for (int i = 0; i < mVelocityGrid->GetCols(); ++i)
         mVels[i] = mVelocityGrid->GetVal(i, 0);
   }
}

void PulseTrain::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mVelocityGrid->SaveState(out);
}

void PulseTrain::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mVelocityGrid->LoadState(in);
   GridUpdated(mVelocityGrid, 0, 0, 0, 0);
}

void PulseTrain::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void PulseTrain::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("width", moduleInfo, 254, 254, 999999, K(isTextField));
   mModuleSaveData.LoadInt("height", moduleInfo, 58, 58, 999999, K(isTextField));

   SetUpFromSaveData();
}

void PulseTrain::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
