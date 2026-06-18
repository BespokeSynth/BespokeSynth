/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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
//  DJFilterEffect.cpp
//
//  Created by Ryan Challinor on 5/29/26.
//
//

#include "DJFilterEffect.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "Profiler.h"

DJFilterEffect::DJFilterEffect()
: mDryBuffer(gBufferSize)
{
   for (int ch = 0; ch < ChannelBuffer::kMaxNumChannels; ++ch)
   {
      mBiquadLow[ch].mType = kFilterType_Lowpass;
      mBiquadHigh[ch].mType = kFilterType_Highpass;
   }
}

void DJFilterEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTiltSlider = new FloatSlider(this, "tilt", 4, 4, 80, 15, &mTilt, -1, 1);
   mQSlider = new FloatSlider(this, "Q", 4, 20, 80, 15, &mQ, static_cast<float>(sqrt(2.0f) / 2), 18, 3);
   mCenterButton = new ClickButton(this, "center", 87, 4);

   mTiltSlider->SetControlVisualizer(this);
   mQSlider->SetControlVisualizer(this);
   mQSlider->SetMode(FloatSlider::kSquare);
}

DJFilterEffect::~DJFilterEffect()
{
}

void DJFilterEffect::Init()
{
   IDrawableModule::Init();
}

void DJFilterEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(DJFilterEffect);

   if (!mEnabled)
      return;

   bool updateCoefficients = false;

   float bufferSize = buffer->BufferSize();
   if (buffer->NumActiveChannels() != mDryBuffer.NumActiveChannels())
   {
      //force filters for other channels to get updated
      mBiquadLow[0].UpdateFilterCoeff();
      mBiquadHigh[0].UpdateFilterCoeff();
      for (int ch = 1; ch < buffer->NumActiveChannels(); ++ch)
      {
         mBiquadLow[ch].CopyCoeffFrom(mBiquadLow[0]);
         mBiquadHigh[ch].CopyCoeffFrom(mBiquadHigh[0]);
      }
   }
   mDryBuffer.SetNumActiveChannels(buffer->NumActiveChannels());

   float dryAmount = GetDrySignalLevel();
   float volumeLevel = GetVolumeFadeLevel();

   mDryBuffer.CopyFrom(buffer);

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);

      float fLow = ofLerp(4000, 10, powf(std::clamp(-mTilt, 0.0f, 1.0f), 0.5f));
      float fHigh = ofLerp(10, 5000, powf(std::clamp(mTilt, 0.0f, 1.0f), 2.0f));

      if (fLow != mBiquadLow[0].mF || mQ != mBiquadLow[0].mQ)
      {
         mBiquadLow[0].mF = fLow;
         mBiquadLow[0].mQ = mQ;
         mBiquadLow[0].UpdateFilterCoeff();
         for (int ch = 1; ch < buffer->NumActiveChannels(); ++ch)
            mBiquadLow[ch].CopyCoeffFrom(mBiquadLow[0]);
      }

      if (fHigh != mBiquadHigh[0].mF || mQ != mBiquadHigh[0].mQ)
      {
         mBiquadHigh[0].mF = fHigh;
         mBiquadHigh[0].mQ = mQ;
         mBiquadHigh[0].UpdateFilterCoeff();
         for (int ch = 1; ch < buffer->NumActiveChannels(); ++ch)
            mBiquadHigh[ch].CopyCoeffFrom(mBiquadHigh[0]);
      }

      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
      {
         float low = mBiquadLow[ch].Filter(buffer->GetChannel(ch)[i]) * GetLowSignalLevel();
         float high = mBiquadHigh[ch].Filter(buffer->GetChannel(ch)[i]) * GetHighSignalLevel();
         float dry = buffer->GetChannel(ch)[i] * GetDrySignalLevel();
         buffer->GetChannel(ch)[i] = (low + high + dry) * volumeLevel;
      }
   }
}

namespace
{
   float FreqForPos(float pos)
   {
      return 20.0 * std::pow(2.0, pos * 10);
   }

   constexpr float kCrossoverMargin = 0.1f;
   constexpr float kSilentMargin = 0.1f;
}

float DJFilterEffect::GetLowSignalLevel() const
{
   float lowAmount = std::clamp((-mTilt / kCrossoverMargin), 0.0f, 1.0f);
   return lowAmount;
}

float DJFilterEffect::GetHighSignalLevel() const
{
   float highAmount = std::clamp((mTilt / kCrossoverMargin), 0.0f, 1.0f);
   return highAmount;
}

float DJFilterEffect::GetDrySignalLevel() const
{
   float dryAmount = 1.0f - std::clamp(fabsf(mTilt / kCrossoverMargin), 0.0f, 1.0f);
   return dryAmount;
}

float DJFilterEffect::GetVolumeFadeLevel() const
{
   float volumeLevel = std::clamp((1.0f - fabsf(mTilt)) / kSilentMargin, 0.0f, 1.0f);
   return volumeLevel;
}

float DJFilterEffect::GetSignalResponseAt(float freq) const
{
   return mBiquadLow[0].GetMagnitudeResponseAt(freq) * GetLowSignalLevel() + mBiquadHigh[0].GetMagnitudeResponseAt(freq) * GetHighSignalLevel() + GetDrySignalLevel();
}

void DJFilterEffect::DrawModule()
{
   mTiltSlider->Draw();
   mCenterButton->Draw();
   mQSlider->Draw();

   float w, h;
   GetModuleDimensions(w, h);
   ofSetColor(52, 204, 235);
   ofSetLineWidth(1);
   ofBeginShape();
   const int kPixelStep = 1;
   if (mTilt > -1)
   {
      for (int x = 0; x < w + kPixelStep; x += kPixelStep)
      {
         float freq = FreqForPos(x / w);
         if (freq < gSampleRate / 2)
         {
            float response = GetSignalResponseAt(freq);
            response *= GetVolumeFadeLevel();
            ofVertex(x, (.5f - .666f * log10(response)) * h);
         }
      }
      ofEndShape(false);
   }
}

void DJFilterEffect::DrawVisualizationToScreen(AbletonMoveLCD* screen, IUIControl* control)
{
   for (float x = 0; x < AbletonMoveLCD::kMoveDisplayWidth; ++x)
   {
      float freq = FreqForPos(x / AbletonMoveLCD::kMoveDisplayWidth);
      if (freq < gSampleRate / 2)
      {
         float response = GetSignalResponseAt(freq);
         response *= GetVolumeFadeLevel();
         float screenEnvelopeY = AbletonMoveLCD::kMoveDisplayHeight;
         if (response > 0)
            screenEnvelopeY = (.5f - .666f * log10(response)) * AbletonMoveLCD::kMoveDisplayHeight;
         for (int screenY = screenEnvelopeY; screenY < AbletonMoveLCD::kMoveDisplayHeight; ++screenY)
            screen->DrawPixel(x, screenY, LCDDrawMode::Toggle);
      }
   }
}

float DJFilterEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return fabsf(mTilt);
}

void DJFilterEffect::GetModuleDimensions(float& width, float& height)
{
   width = 131;
   height = 69;
}

void DJFilterEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
      {
         mBiquadLow[i].Clear();
         mBiquadHigh[i].Clear();
      }
   }
}

void DJFilterEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void DJFilterEffect::ButtonClicked(ClickButton* button, double time)
{
   if (button == mCenterButton)
   {
      mTilt = 0.0f;
   }
}

void DJFilterEffect::LoadLayout(const ofxJSONElement& info)
{
}

void DJFilterEffect::SetUpFromSaveData()
{
}

void DJFilterEffect::SaveLayout(ofxJSONElement& info)
{
   mModuleSaveData.Save(info);
}
