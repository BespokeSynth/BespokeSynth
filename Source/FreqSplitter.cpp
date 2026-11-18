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

    FreqSplitter.cpp
    Created: Nov 18 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "FreqSplitter.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

FreqSplitter::FreqSplitter()
: IAudioProcessor(gBufferSize)
, mVizBuffer2(VIZ_BUFFER_SECONDS * gSampleRate)
{
   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
   {
      mBiquad[i].SetFilterParams(2000, sqrt(2) / 2);
      mBiquad[i].SetFilterType(kFilterType_Highpass);
      mBiquad[i].UpdateFilterCoeff();
   }
}

void FreqSplitter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK(120);
   FLOATSLIDER(mFSlider, "F", &mBiquad[0].mF, 10, 7000);
   FLOATSLIDER_DIGITS(mQSlider, "Q", &mBiquad[0].mQ, .1f, 18, 3);
   UICONTROL_CUSTOM(mTypeSelector, new RadioButton(UICONTROL_BASICS("type"), (int*)(&mBiquad[0].mType), kRadioHorizontal));
   ENDUIBLOCK(mWidth, mHeight);

   mTypeSelector->AddLabel("lowpass", kFilterType_Lowpass);
   mTypeSelector->AddLabel("highpass", kFilterType_Highpass);

   mFSlider->SetMode(FloatSlider::kSquare);
   mQSlider->SetMode(FloatSlider::kSquare);

   mFSlider->SetMinValueDisplay("high only");
   mFSlider->SetMaxValueDisplay("low only");

   float w, h;
   GetDimensions(w, h);
   GetPatchCableSource()->SetManualPosition(w / 2 - 15, h + 3);
   GetPatchCableSource()->SetManualSide(PatchCableSource::Side::kBottom);

   mPatchCableSource2 = new PatchCableSource(this, kConnectionType_Audio);
   mPatchCableSource2->SetManualPosition(w / 2 + 15, h + 3);
   mPatchCableSource2->SetOverrideVizBuffer(&mVizBuffer2);
   mPatchCableSource2->SetManualSide(PatchCableSource::Side::kBottom);
   AddPatchCableSource(mPatchCableSource2);
}

FreqSplitter::~FreqSplitter()
{
}

void FreqSplitter::Process(double time)
{
   PROFILER(FreqSplitter);

   ComputeSliders(0);

   SyncBuffers();

   int bufferSize = GetBuffer()->BufferSize();
   int numChannels = GetBuffer()->NumActiveChannels();

   mVizBuffer2.SetNumChannels(numChannels);

   for (int ch = 0; ch < numChannels; ch++)
   {
      float* channelData = GetBuffer()->GetChannel(ch);

      float* lowBuffer;
      float* highBuffer;

      if (mBiquad[0].mF == mFSlider->GetMin())
      {
         lowBuffer = gZeroBuffer;
         highBuffer = channelData;
      }
      else if (mBiquad[0].mF == mFSlider->GetMax())
      {
         lowBuffer = channelData;
         highBuffer = gZeroBuffer;
      }
      else
      {
         BufferCopy(gWorkBuffer, channelData, bufferSize);
         mBiquad[ch].Filter(channelData, bufferSize);
         Subtract(gWorkBuffer, channelData, bufferSize);

         lowBuffer = mBiquad[ch].mType == kFilterType_Lowpass ? channelData : gWorkBuffer;
         highBuffer = mBiquad[ch].mType == kFilterType_Highpass ? channelData : gWorkBuffer;
      }

      if (IAudioReceiver* targetLow = GetTarget(0))
      {
         ChannelBuffer* out = targetLow->GetBuffer();
         Add(out->GetChannel(ch), lowBuffer, bufferSize);
         GetVizBuffer()->WriteChunk(lowBuffer, bufferSize, ch);
      }

      if (IAudioReceiver* targetHigh = GetTarget(1))
      {
         ChannelBuffer* out = targetHigh->GetBuffer();
         Add(out->GetChannel(ch), highBuffer, bufferSize);
         mVizBuffer2.WriteChunk(highBuffer, GetBuffer()->BufferSize(), ch);
      }

      for (int i = 0; i < bufferSize; ++i)
      {
         mLowLevel = std::max(mLowLevel, fabsf(lowBuffer[i]));
         mHighLevel = std::max(mHighLevel, fabsf(highBuffer[i]));
      }
   }

   GetBuffer()->Reset();
}

void FreqSplitter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mFSlider->Draw();
   mQSlider->Draw();
   mTypeSelector->Draw();

   ofRectangle sliderRect = mFSlider->GetRect(K(local));

   float span = sliderRect.width - 4;
   float xStart = sliderRect.x + 2;
   float xEnd = sliderRect.x + sliderRect.width - 2;

   ofPushStyle();
   ofSetLineWidth(2);
   ofSetColor(ofColor::green);
   ofLine(xStart, sliderRect.y + 2, xStart + span * mLowLevel, sliderRect.y + 2);
   ofSetColor(ofColor::red);
   ofLine(xEnd, sliderRect.y + sliderRect.height - 2, xEnd - span * mHighLevel, sliderRect.y + sliderRect.height - 2);
   ofPopStyle();

   mLowLevel = 0;
   mHighLevel = 0;
}

void FreqSplitter::UpdateFilter()
{
   mBiquad[0].UpdateFilterCoeff();
   for (int ch = 1; ch < ChannelBuffer::kMaxNumChannels; ++ch)
   {
      mBiquad[ch].mF = mBiquad[0].mF;
      mBiquad[ch].mQ = mBiquad[0].mQ;
      mBiquad[ch].mType = mBiquad[0].mType;
      mBiquad[ch].CopyCoeffFrom(mBiquad[0]);
   }
}

void FreqSplitter::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mFSlider || slider == mQSlider)
      UpdateFilter();
}

void FreqSplitter::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
   if (radio == mTypeSelector)
      UpdateFilter();
}

void FreqSplitter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("target2", moduleInfo);

   SetUpFromSaveData();
}

void FreqSplitter::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   IClickable* target2 = TheSynth->FindModule(mModuleSaveData.GetString("target2"));
   if (target2)
      mPatchCableSource2->AddPatchCable(target2);
}
