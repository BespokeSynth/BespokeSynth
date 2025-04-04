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

    LooperGranulator.cpp
    Created: 13 Mar 2021 1:55:54pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "LooperGranulator.h"
#include "ModularSynth.h"
#include "Slider.h"
#include "PatchCableSource.h"
#include "Looper.h"
#include "FillSaveDropdown.h"
#include "UIControlMacros.h"

LooperGranulator::LooperGranulator()
{
}

LooperGranulator::~LooperGranulator()
{
}

void LooperGranulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 3, 120);
   CHECKBOX(mOnCheckbox, "on", &mOn);
   FLOATSLIDER(mGranOverlap, "overlap", &mGranulator.mGrainOverlap, .5f, MAX_GRAINS);
   FLOATSLIDER(mGranSpeed, "speed", &mGranulator.mSpeed, -3, 3);
   FLOATSLIDER(mGranLengthMs, "len ms", &mGranulator.mGrainLengthMs, 1, 1000);
   FLOATSLIDER(mPosSlider, "loop pos", &mDummyPos, 0, 1);
   CHECKBOX(mFreezeCheckbox, "freeze", &mFreeze);
   FLOATSLIDER(mGranPosRandomize, "pos rand", &mGranulator.mPosRandomizeMs, 0, 200);
   FLOATSLIDER(mGranSpeedRandomize, "speed rand", &mGranulator.mSpeedRandomize, 0, .3f);
   FLOATSLIDER(mGranSpacingRandomize, "spacing rand", &mGranulator.mSpacingRandomize, 0, 1);
   CHECKBOX(mGranOctaveCheckbox, "octaves", &mGranulator.mOctaves);
   FLOATSLIDER(mGranWidthSlider, "width", &mGranulator.mWidth, 0, 1);
   ENDUIBLOCK(mWidth, mHeight);

   mLooperCable = new PatchCableSource(this, kConnectionType_Special);
   //mLooperCable->SetManualPosition(99, 10);
   mLooperCable->AddTypeFilter("looper");
   AddPatchCableSource(mLooperCable);

   mGranPosRandomize->SetMode(FloatSlider::kSquare);
   mGranLengthMs->SetMode(FloatSlider::kSquare);
}

void LooperGranulator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (mLooper != nullptr)
      mPosSlider->SetExtents(0, mLooper->GetLoopLength());

   mOnCheckbox->Draw();
   mGranOverlap->Draw();
   mGranSpeed->Draw();
   mGranLengthMs->Draw();
   mPosSlider->Draw();
   mFreezeCheckbox->Draw();
   mGranPosRandomize->Draw();
   mGranSpeedRandomize->Draw();
   mGranSpacingRandomize->Draw();
   mGranOctaveCheckbox->Draw();
   mGranWidthSlider->Draw();
}

void LooperGranulator::DrawOverlay(ofRectangle bufferRect, int loopLength)
{
   if (mOn)
      mGranulator.Draw(bufferRect.x, bufferRect.y, bufferRect.width, bufferRect.height, 0, loopLength, loopLength);
}

void LooperGranulator::ProcessFrame(double time, float bufferOffset, float* output)
{
   if (mLooper != nullptr)
   {
      int bufferLength;
      auto* buffer = mLooper->GetLoopBuffer(bufferLength);
      mGranulator.ProcessFrame(time, buffer, bufferLength, bufferOffset, 1.0f, output);
   }
}

void LooperGranulator::OnCommit()
{
   mOn = false;
   mFreeze = false;
   if (mPosSlider->GetLFO())
      mPosSlider->GetLFO()->SetLFOEnabled(false);
}

void LooperGranulator::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mLooperCable)
   {
      mLooper = dynamic_cast<Looper*>(mLooperCable->GetTarget());
      if (mLooper != nullptr)
      {
         mLooper->SetGranulator(this);
         mPosSlider->SetVar(mLooper->GetLoopPosVar());
      }
   }
}

void LooperGranulator::ButtonClicked(ClickButton* button, double time)
{
}

void LooperGranulator::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void LooperGranulator::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void LooperGranulator::SaveLayout(ofxJSONElement& moduleInfo)
{
   std::string targetPath = "";
   if (mLooperCable->GetTarget())
      targetPath = mLooperCable->GetTarget()->Path();

   moduleInfo["looper"] = targetPath;
}

void LooperGranulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("looper", moduleInfo, "", FillDropdown<Looper*>);

   SetUpFromSaveData();
}

void LooperGranulator::SetUpFromSaveData()
{
   mLooperCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("looper"), false));
}
