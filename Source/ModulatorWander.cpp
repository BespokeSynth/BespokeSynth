/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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

    ModulatorWander.cpp
    Created: 16 Mar 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModulatorWander.h"
#include "Profiler.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"
#include "Transport.h"

ModulatorWander::ModulatorWander()
{
   mPerlinSeed = gRandom() % 10000;
}

void ModulatorWander::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void ModulatorWander::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(3, 50);
   FLOATSLIDER(mCenterSlider, "center", &mCenter, 0.0, 1.0);
   FLOATSLIDER(mRangeSlider, "range", &mRange, 0.0, 1.0);
   FLOATSLIDER(mSpeedSlider, "speed", &mSpeed, 0.0, 20.0);
   FLOATSLIDER(mBiasSlider, "bias", &mBias, 0.0, 1.0);
   ENDUIBLOCK(mWidth, mHeight);

   mSpeedSlider->SetMode(FloatSlider::kBezier);
   mSpeedSlider->SetBezierControl(1);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

ModulatorWander::~ModulatorWander()
{
   TheTransport->RemoveAudioPoller(this);
}

void ModulatorWander::OnTransportAdvanced(double amount)
{
   mPerlinPos += gBufferSizeMs * mSpeed * .001;
}

void ModulatorWander::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mCenterSlider->Draw();
   mRangeSlider->Draw();
   mSpeedSlider->Draw();
   mBiasSlider->Draw();

   {
      int x = 3;
      int y = 3;
      int height = 44;
      int width = 100;

      ofSetColor(100, 100, 204, .8 * gModuleDrawAlpha);
      ofSetLineWidth(.5);
      ofRect(x, y, width, height, 0);

      ofFill();
      ofSetColor(200, 200, 200, .1 * gModuleDrawAlpha);
      double scaledMin = ofMap(GetMin(), mCenterSlider->GetMin(), mCenterSlider->GetMax(), height, y);
      double scaledMax = ofMap(GetMax(), mCenterSlider->GetMin(), mCenterSlider->GetMax(), height, y);
      double scaledCenter = ofMap((GetMin() + GetMax()) / 2.0, mCenterSlider->GetMin(), mCenterSlider->GetMax(), height, y);
      ofRect(x, scaledMax, width, scaledMin - scaledMax);
      ofLine(x, scaledCenter, x + width, scaledCenter);
      ofNoFill();

      ofSetColor(245, 58, 0, gModuleDrawAlpha);
      ofSetLineWidth(1);

      ofBeginShape();
      for (double i = 0; i < width; i += (.25 / gDrawScale))
      {
         double phase = i / width;
         double value = Interp(GetPerlin(phase * 100000), GetMin(), GetMax());
         ofVertex(i + x, ofMap(value, mCenterSlider->GetMin(), mCenterSlider->GetMax(), height, y));
      }
      ofEndShape(false);

      ofCircle(x, ofMap(Value(), mCenterSlider->GetMin(), mCenterSlider->GetMax(), height, y), 2);
   }
}

double ModulatorWander::GetPerlin(double sampleOffset)
{
   double pos = mPerlinPos + sampleOffset * gInvSampleRateMs * mSpeed * .001;
   double value = mPerlinNoise.noise(pos, mPerlinSeed, -pos);
   return Bias(value, 1 - mBias);
}

void ModulatorWander::FloatSliderUpdated(FloatSlider* slider, double oldVal, double time)
{
   if (slider == mCenterSlider || slider == mRangeSlider)
      UpdateRange();
}

void ModulatorWander::UpdateRange()
{
   double sliderExtent = mCenterSlider->GetMax() - mCenterSlider->GetMin();
   double scaledRange = (mRange * 0.5) * sliderExtent;
   if (mCenter - scaledRange < mCenterSlider->GetMin())
      scaledRange = mCenter - mCenterSlider->GetMin();
   if (mCenter + scaledRange > mCenterSlider->GetMax())
      scaledRange = mCenterSlider->GetMax() - mCenter;
   GetMin() = mCenter - scaledRange;
   GetMax() = mCenter + scaledRange;
}

void ModulatorWander::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
   if (GetSliderTarget() && fromUserClick)
   {
      mCenter = GetSliderTarget()->GetValue();
      mCenterSlider->SetExtents(GetSliderTarget()->GetMin(), GetSliderTarget()->GetMax());
      mCenterSlider->SetMode(GetSliderTarget()->GetMode());
      UpdateRange();
   }
}

double ModulatorWander::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return Interp(GetPerlin(samplesIn), GetMin(), GetMax());
}

void ModulatorWander::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ModulatorWander::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ModulatorWander::SetUpFromSaveData()
{
}
