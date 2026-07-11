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
//  BiquadFilterEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/29/12.
//
//

#include "BiquadFilterEffect.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "Profiler.h"

BiquadFilterEffect::BiquadFilterEffect()
: mDryBuffer(gBufferSize)
{
}

void BiquadFilterEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTypeSelector = new RadioButton(this, "type", 4, 52, (int*)(&mBiquad[0].mType), kRadioHorizontal);
   mFSlider = new FloatSlider(this, "F", 4, 4, 80, 15, &mBiquad[0].mF, 10, 4000);
   mQSlider = new FloatSlider(this, "Q", 4, 20, 80, 15, &mBiquad[0].mQ, .1f, 18, 3);
   mGSlider = new FloatSlider(this, "G", 4, 36, 80, 15, &mBiquad[0].mDbGain, -96, 96, 1);

   mTypeSelector->AddLabel("lp", kFilterType_Lowpass);
   mTypeSelector->AddLabel("hp", kFilterType_Highpass);
   mTypeSelector->AddLabel("bp", kFilterType_Bandpass);
   mTypeSelector->AddLabel("pk", kFilterType_Peak);
   mTypeSelector->AddLabel("ap", kFilterType_Allpass);
   mTypeSelector->AddLabel("comb", kFilterType_Comb);

   mFSlider->SetMaxValueDisplay("inf");
   mFSlider->SetMode(FloatSlider::kSquare);
   mQSlider->SetMode(FloatSlider::kSquare);
   mQSlider->SetShowing(mBiquad[0].UsesQ());
   mGSlider->SetShowing(mBiquad[0].UsesGain());

   mTypeSelector->SetControlVisualizer(this);
   mFSlider->SetControlVisualizer(this);
   mQSlider->SetControlVisualizer(this);
   mGSlider->SetControlVisualizer(this);
}

BiquadFilterEffect::~BiquadFilterEffect()
{
}

void BiquadFilterEffect::Init()
{
   IDrawableModule::Init();
}

void BiquadFilterEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(BiquadFilterEffect);

   if (!mEnabled)
      return;

   float bufferSize = buffer->BufferSize();
   if (buffer->NumActiveChannels() != mDryBuffer.NumActiveChannels())
      mCoefficientsHaveChanged = true; //force filters for other channels to get updated
   mDryBuffer.SetNumActiveChannels(buffer->NumActiveChannels());

   const float fadeOutStart = mFSlider->GetMax() * .75f;
   const float fadeOutEnd = mFSlider->GetMax();
   bool fadeOut = mBiquad[0].mF > fadeOutStart && mBiquad[0].mType == kFilterType_Lowpass;
   if (fadeOut)
      mDryBuffer.CopyFrom(buffer);

   bool isComb = (mBiquad[0].mType == kFilterType_Comb);

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);

      if (isComb)
      {
         //F slider repurposed as the comb's fundamental frequency (sets delay length), Q slider repurposed as feedback amount
         int delaySamples = ofClamp((int)(gSampleRate / MAX(mBiquad[0].mF, 1.0f)), 1, kCombBufferSize - 1);
         float feedback = ofClamp(mBiquad[0].mQ / mQSlider->GetMax(), 0, 0.98f);
         for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
            buffer->GetChannel(ch)[i] = ProcessComb(ch, buffer->GetChannel(ch)[i], feedback, delaySamples);
      }
      else
      {
         if (mCoefficientsHaveChanged)
         {
            mBiquad[0].UpdateFilterCoeff();
            for (int ch = 1; ch < buffer->NumActiveChannels(); ++ch)
               mBiquad[ch].CopyCoeffFrom(mBiquad[0]);
            mCoefficientsHaveChanged = false;
         }
         for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
            buffer->GetChannel(ch)[i] = mBiquad[ch].Filter(buffer->GetChannel(ch)[i]);
      }
   }

   if (fadeOut)
   {
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
      {
         float dryness = ofMap(mBiquad[0].mF, fadeOutStart, fadeOutEnd, 0, 1);
         Mult(buffer->GetChannel(ch), 1 - dryness, bufferSize);
         Mult(mDryBuffer.GetChannel(ch), dryness, bufferSize);
         Add(buffer->GetChannel(ch), mDryBuffer.GetChannel(ch), bufferSize);
      }
   }
}

namespace
{
   float FreqForPos(float pos)
   {
      return 20.0 * std::pow(2.0, pos * 10);
   }
}

void BiquadFilterEffect::DrawModule()
{
   mTypeSelector->Draw();

   mFSlider->Draw();
   mQSlider->Draw();
   mGSlider->Draw();

   float w, h;
   GetModuleDimensions(w, h);

   if (mBiquad[0].mType == kFilterType_Comb)
   {
      //comb filters have teeth spaced at multiples of the fundamental rather than a smooth biquad response curve;
      //draw a simple comb-tooth pattern instead of the (meaningless here) biquad magnitude response
      ofSetColor(52, 204, 235);
      ofSetLineWidth(1);
      const int kNumTeeth = 10;
      for (int t = 0; t < kNumTeeth; ++t)
      {
         float x = w * (t + 0.5f) / kNumTeeth;
         ofLine(x, h, x, h * .15f);
      }
   }
   else
   {
      ofSetColor(52, 204, 235);
      ofSetLineWidth(1);
      ofBeginShape();
      const int kPixelStep = 1;
      for (int x = 0; x < w + kPixelStep; x += kPixelStep)
      {
         float freq = FreqForPos(x / w);
         if (freq < gSampleRate / 2)
         {
            float response = mBiquad[0].GetMagnitudeResponseAt(freq);
            ofVertex(x, (.5f - .666f * log10(response)) * h);
         }
      }
      ofEndShape(false);
   }
}

void BiquadFilterEffect::DrawVisualizationToScreen(AbletonMoveLCD* screen, IUIControl* control)
{
   for (float x = 0; x < AbletonMoveLCD::kMoveDisplayWidth; ++x)
   {
      float freq = FreqForPos(x / AbletonMoveLCD::kMoveDisplayWidth);
      if (freq < gSampleRate / 2)
      {
         float response = mBiquad[0].GetMagnitudeResponseAt(freq);
         float screenEnvelopeY = (.5f - .666f * log10(response)) * AbletonMoveLCD::kMoveDisplayHeight;
         for (int screenY = screenEnvelopeY; screenY < AbletonMoveLCD::kMoveDisplayHeight; ++screenY)
            screen->DrawPixel(x, screenY, LCDDrawMode::Toggle);
      }
   }
}

float BiquadFilterEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   if (mBiquad[0].mType == kFilterType_Lowpass)
      return ofClamp(1 - (mBiquad[0].mF / (mFSlider->GetMax() * .75f)), 0, 1);
   if (mBiquad[0].mType == kFilterType_Highpass)
      return ofClamp(mBiquad[0].mF / (mFSlider->GetMax() * .75f), 0, 1);
   if (mBiquad[0].mType == kFilterType_Bandpass)
      return ofClamp(.3f + (mBiquad[0].mQ / mQSlider->GetMax()), 0, 1);
   if (mBiquad[0].mType == kFilterType_Peak)
      return ofClamp(fabsf(mBiquad[0].mDbGain / 96), 0, 1);
   if (mBiquad[0].mType == kFilterType_Comb)
      return ofClamp(mBiquad[0].mQ / mQSlider->GetMax(), 0, 1);
   return 0;
}

void BiquadFilterEffect::GetModuleDimensions(float& width, float& height)
{
   width = 120;
   if (mTypeSelector != nullptr)
      width = MAX(width, mTypeSelector->GetPosition(true).x + mTypeSelector->GetRect().width + 4);
   height = 69;
}

void BiquadFilterEffect::Clear()
{
   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
   {
      mBiquad[i].Clear();
      for (int j = 0; j < kCombBufferSize; ++j)
         mCombBuffer[i][j] = 0;
      mCombWritePos[i] = 0;
   }
}

float BiquadFilterEffect::ProcessComb(int channel, float input, float feedback, int delaySamples)
{
   int writePos = mCombWritePos[channel];
   int readPos = writePos - delaySamples;
   while (readPos < 0)
      readPos += kCombBufferSize;
   readPos %= kCombBufferSize;

   float delayed = mCombBuffer[channel][readPos];
   float output = input + feedback * delayed;
   output = ofClamp(output, -10.0f, 10.0f); //safety clamp against runaway feedback

   mCombBuffer[channel][writePos] = output;
   mCombWritePos[channel] = (writePos + 1) % kCombBufferSize;
   return output;
}

void BiquadFilterEffect::ResetFilter()
{
   if (mBiquad[0].mType == kFilterType_Lowpass)
      mBiquad[0].SetFilterParams(mFSlider->GetMax(), sqrt(2) / 2);
   if (mBiquad[0].mType == kFilterType_Highpass)
      mBiquad[0].SetFilterParams(mFSlider->GetMin(), sqrt(2) / 2);

   mBiquad[0].UpdateFilterCoeff();

   for (int ch = 1; ch < ChannelBuffer::kMaxNumChannels; ++ch)
      mBiquad[ch].CopyCoeffFrom(mBiquad[0]);

   Clear();
}

void BiquadFilterEffect::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void BiquadFilterEffect::RadioButtonUpdated(RadioButton* list, int oldVal, double time)
{
   if (list == mTypeSelector)
   {
      ResetFilter();
      mQSlider->SetShowing(mBiquad[0].UsesQ());
      mGSlider->SetShowing(mBiquad[0].UsesGain());
   }
}

bool BiquadFilterEffect::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   if (mMouseControl)
   {
      float thisx, thisy;
      GetPosition(thisx, thisy);
      x += thisx;
      y += thisy;
      mFSlider->SetValue(x * 2 + 150, NextBufferTime(false));
      mQSlider->SetValue(y / 100.0f, NextBufferTime(false));
   }

   return false;
}

void BiquadFilterEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
         mBiquad[i].Clear();
   }
}

void BiquadFilterEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mFSlider || slider == mQSlider || slider == mGSlider)
      mCoefficientsHaveChanged = true;
}

void BiquadFilterEffect::LoadLayout(const ofxJSONElement& info)
{
}

void BiquadFilterEffect::SetUpFromSaveData()
{
   ResetFilter();
}

void BiquadFilterEffect::SaveLayout(ofxJSONElement& info)
{
   mModuleSaveData.Save(info);
}
