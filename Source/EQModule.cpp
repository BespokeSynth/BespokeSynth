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

    EQModule.cpp
    Created: 2 Nov 2020 10:47:17pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "EQModule.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"
#include "Checkbox.h"

#include "juce_core/juce_core.h"

EQModule::EQModule()
: IAudioProcessor(gBufferSize)
, mFFT(kNumFFTBins)
, mFFTData(kNumFFTBins, kNumFFTBins / 2 + 1)
, mRollingInputBuffer(kNumFFTBins)
{
   // Generate a window with a single raised cosine from N/4 to 3N/4
   mWindower = new float[kNumFFTBins];
   for (int i = 0; i < kNumFFTBins; ++i)
      mWindower[i] = -.5f * cos(FTWO_PI * i / kNumFFTBins) + .5f;
   mSmoother = new float[kNumFFTBins / 2 + 1 - kBinIgnore];
   for (int i = 0; i < kNumFFTBins / 2 + 1 - kBinIgnore; ++i)
      mSmoother[i] = 0;

   assert(mFilters.size() == 8);
   auto types = new FilterType[8]{ kFilterType_LowShelf, kFilterType_Peak, kFilterType_Peak, kFilterType_HighShelf, kFilterType_Peak, kFilterType_Peak, kFilterType_Peak, kFilterType_Peak };
   auto cutoffs = new float[8]{ 30, 200, 1000, 5000, 100, 10000, 5000, 18000 };
   for (size_t i = 0; i < mFilters.size(); ++i)
   {
      auto& filter = mFilters[i];
      filter.mEnabled = i < 4;
      for (auto& biquad : filter.mFilter)
      {
         biquad.SetFilterParams(cutoffs[i], sqrtf(2) / 2);
         biquad.SetFilterType(types[i]);
      }
      filter.mNeedToCalculateCoefficients = true;
   }
}

void EQModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (size_t i = 0; i < mFilters.size(); ++i)
   {
      auto& filter = mFilters[i];

      CHECKBOX(filter.mEnabledCheckbox, ("enabled" + ofToString(i)).c_str(), &filter.mEnabled);
      DROPDOWN(filter.mTypeSelector, ("type" + ofToString(i)).c_str(), (int*)(&filter.mFilter[0].mType), 45);
      FLOATSLIDER(filter.mFSlider, ("f" + ofToString(i)).c_str(), &filter.mFilter[0].mF, 20, 20000);
      FLOATSLIDER(filter.mGSlider, ("g" + ofToString(i)).c_str(), &filter.mFilter[0].mDbGain, -15, 15);
      FLOATSLIDER_DIGITS(filter.mQSlider, ("q" + ofToString(i)).c_str(), &filter.mFilter[0].mQ, .1f, 18, 3);
      UIBLOCK_NEWCOLUMN();

      filter.mTypeSelector->AddLabel("lp", kFilterType_Lowpass);
      filter.mTypeSelector->AddLabel("hp", kFilterType_Highpass);
      filter.mTypeSelector->AddLabel("bp", kFilterType_Bandpass);
      filter.mTypeSelector->AddLabel("pk", kFilterType_Peak);
      filter.mTypeSelector->AddLabel("nt", kFilterType_Notch);
      filter.mTypeSelector->AddLabel("hs", kFilterType_HighShelf);
      filter.mTypeSelector->AddLabel("ls", kFilterType_LowShelf);

      filter.mFSlider->SetMode(FloatSlider::kSquare);
      filter.mQSlider->SetMode(FloatSlider::kSquare);
      filter.mGSlider->SetShowing(filter.mFilter[0].UsesGain());
      filter.mQSlider->SetShowing(filter.mFilter[0].UsesQ());
   }
   ENDUIBLOCK0();
}

EQModule::~EQModule()
{
   delete[] mWindower;
   delete[] mSmoother;
}

void EQModule::Process(double time)
{
   PROFILER(EQModule);

   if (mLiteCpuModulation)
   {
      ComputeSliders(0);

      for (auto& filter : mFilters)
      {
         if (filter.mEnabled)
         {
            bool updated = filter.UpdateCoefficientsIfNecessary();
            if (updated)
               mNeedToUpdateFrequencyResponseGraph = true;
         }
      }
   }

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();

   if (mEnabled)
   {
      Clear(gWorkBuffer, GetBuffer()->BufferSize());

      ChannelBuffer* out = target->GetBuffer();
      gWorkChannelBuffer.SetNumActiveChannels(out->NumActiveChannels());

      if (!mLiteCpuModulation) //should we try to recalculate filters every sample?
      {
         for (int i = 0; i < GetBuffer()->BufferSize(); ++i)
         {
            ComputeSliders(i);

            for (auto& filter : mFilters)
            {
               if (filter.mEnabled)
               {
                  bool updated = filter.UpdateCoefficientsIfNecessary();
                  if (updated)
                     mNeedToUpdateFrequencyResponseGraph = true;
               }
            }

            for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
            {
               float sample = GetBuffer()->GetChannel(ch)[i];
               for (auto& filter : mFilters)
               {
                  if (filter.mEnabled)
                     sample = filter.mFilter[ch].Filter(sample);
               }
               gWorkChannelBuffer.GetChannel(ch)[i] = sample;
            }
         }
      }
      else
      {
         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         {
            BufferCopy(gWorkChannelBuffer.GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
            for (auto& filter : mFilters)
            {
               if (filter.mEnabled)
                  filter.mFilter[ch].Filter(gWorkChannelBuffer.GetChannel(ch), GetBuffer()->BufferSize());
            }
         }
      }

      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(out->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         Add(gWorkBuffer, gWorkChannelBuffer.GetChannel(ch), GetBuffer()->BufferSize());

      mRollingInputBuffer.WriteChunk(gWorkBuffer, GetBuffer()->BufferSize(), 0);

      //copy rolling input buffer into working buffer and window it
      mRollingInputBuffer.ReadChunk(mFFTData.mTimeDomain, kNumFFTBins, 0, 0);
      Mult(mFFTData.mTimeDomain, mWindower, kNumFFTBins);

      mFFT.Forward(mFFTData.mTimeDomain,
                   mFFTData.mRealValues,
                   mFFTData.mImaginaryValues);
   }
   else //passthrough
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         float* buffer = GetBuffer()->GetChannel(ch);
         Add(target->GetBuffer()->GetChannel(ch), buffer, GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(buffer, GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void EQModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (auto& filter : mFilters)
   {
      filter.mTypeSelector->SetShowing(filter.mEnabled);
      filter.mFSlider->SetShowing(filter.mEnabled);
      filter.mGSlider->SetShowing(filter.mEnabled && filter.mFilter[0].UsesGain());
      filter.mQSlider->SetShowing(filter.mEnabled && filter.mFilter[0].UsesQ());

      filter.mEnabledCheckbox->Draw();
      filter.mTypeSelector->Draw();
      filter.mFSlider->Draw();
      filter.mQSlider->Draw();
      filter.mGSlider->Draw();
   }

   ofPushStyle();
   ofPushMatrix();

   float w, h;
   GetDimensions(w, h);
   h -= kDrawYOffset;

   //raw
   ofSetColor(255, 255, 255);
   ofSetLineWidth(1);
   int end = kNumFFTBins / 2 + 1;
   ofBeginShape();
   int lastX = -1;
   for (int i = kBinIgnore; i < end; i++)
   {
      float freq = FreqForBin(i);
      float x = PosForFreq(freq) * w;
      float samp = ofClamp(sqrtf(fabsf(mFFTData.mRealValues[i]) / end) * 3 * mDrawGain, 0, 1);
      float y = (1 - samp) * h + kDrawYOffset;
      if (int(x) != lastX)
         ofVertex(x, y);
      lastX = int(x);

      mSmoother[i - kBinIgnore] = ofLerp(mSmoother[i - kBinIgnore], samp, .1f);
   }
   ofEndShape(false);

   //smoothed
   ofSetColor(245, 58, 135);
   ofSetLineWidth(3);
   ofBeginShape();
   for (int i = kBinIgnore; i < end; i++)
   {
      float freq = FreqForBin(i);
      float x = PosForFreq(freq) * w;
      float y = (1 - mSmoother[i - kBinIgnore]) * h + kDrawYOffset;
      if (int(x) != lastX)
         ofVertex(x, y);
      lastX = int(x);
   }
   ofEndShape(false);

   //frequency response
   ofSetColor(52, 204, 235);
   ofSetLineWidth(3);
   ofBeginShape();
   const int kPixelStep = 2;
   bool updateFrequencyResponseGraph = false;
   if (mNeedToUpdateFrequencyResponseGraph)
   {
      updateFrequencyResponseGraph = true;
      mNeedToUpdateFrequencyResponseGraph = false;
   }
   for (int x = 0; x < w + kPixelStep; x += kPixelStep)
   {
      float response = 1;
      float freq = FreqForPos(x / w);
      if (freq < gSampleRate / 2)
      {
         int responseGraphIndex = x / kPixelStep;
         if (updateFrequencyResponseGraph || responseGraphIndex >= mFrequencyResponse.size())
         {
            for (auto& filter : mFilters)
            {
               if (filter.mEnabled)
                  response *= filter.mFilter[0].GetMagnitudeResponseAt(freq);
            }
            if (responseGraphIndex < mFrequencyResponse.size())
               mFrequencyResponse[responseGraphIndex] = response;
         }
         else
         {
            response = mFrequencyResponse[responseGraphIndex];
         }
         ofVertex(x, (.5f - .666f * log10(response)) * h + kDrawYOffset);
      }
   }
   ofEndShape(false);

   ofSetLineWidth(1);
   for (size_t i = 0; i < mFilters.size(); ++i)
   {
      auto& filter = mFilters[i];
      if (filter.mEnabled)
      {
         float x = PosForFreq(filter.mFilter[0].mF) * w;
         float y = PosForGain(filter.mFilter[0].mDbGain) * h + kDrawYOffset;
         ofFill();
         ofSetColor(255, 210, 0);
         ofCircle(x, y, 8);
         if (i == mHoveredFilterHandleIndex)
         {
            ofNoFill();
            ofSetColor(255, 255, 255);
            ofCircle(x, y, 8);
         }
         ofSetColor(0, 0, 0);
         DrawTextBold(ofToString(i), x - 3, y + 5);
      }
   }

   ofPopMatrix();
   ofPopStyle();
}

bool EQModule::Filter::UpdateCoefficientsIfNecessary()
{
   if (mNeedToCalculateCoefficients)
   {
      mFilter[0].UpdateFilterCoeff();
      for (size_t ch = 1; ch < mFilter.size(); ++ch)
         mFilter[ch].CopyCoeffFrom(mFilter[0]);
      mNeedToCalculateCoefficients = false;
      return true;
   }

   return false;
}

void EQModule::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (!right)
      mDragging = true;
}

void EQModule::MouseReleased()
{
   IDrawableModule::MouseReleased();

   mDragging = false;
}

bool EQModule::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   float w, h;
   GetDimensions(w, h);
   h -= kDrawYOffset;

   if (mDragging)
   {
      if (mHoveredFilterHandleIndex != -1)
      {
         auto* fSlider = mFilters[mHoveredFilterHandleIndex].mFSlider;
         auto* gSlider = mFilters[mHoveredFilterHandleIndex].mGSlider;
         fSlider->SetValue(ofClamp(FreqForPos(x / w), fSlider->GetMin(), fSlider->GetMax()), NextBufferTime(false));
         gSlider->SetValue(ofClamp(GainForPos((y - kDrawYOffset) / h), gSlider->GetMin(), gSlider->GetMax()), NextBufferTime(false));
      }
   }
   else
   {
      mHoveredFilterHandleIndex = -1;
      for (int i = 0; i < mFilters.size(); ++i)
      {
         if (mFilters[i].mEnabled &&
             abs(x - PosForFreq(mFilters[i].mFilter[0].mF) * w) < 5 &&
             abs((y - kDrawYOffset) - PosForGain(mFilters[i].mFilter[0].mDbGain) * h) < 5)
         {
            mHoveredFilterHandleIndex = i;
            break;
         }
      }
   }

   return false;
}

bool EQModule::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   if (mHoveredFilterHandleIndex != -1)
   {
      auto* qSlider = mFilters[mHoveredFilterHandleIndex].mQSlider;
      float add = (2 * scrollY) / MAX(qSlider->GetModulatorMax() / qSlider->GetValue(), 0.1);
      if (GetKeyModifiers() & kModifier_Command)
      {
         add *= 4;
      }
      else if (GetKeyModifiers() & kModifier_Shift)
      {
         add /= 10;
      }
      qSlider->SetValue(ofClamp(qSlider->GetValue() + add, qSlider->GetMin(), qSlider->GetMax()), NextBufferTime(false));
   }
   return false;
}

void EQModule::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
   if (mHoveredFilterHandleIndex != -1)
   {
      auto* qSlider = mFilters[mHoveredFilterHandleIndex].mQSlider;
      if (key == '\\')
      {
         qSlider->ResetToOriginal();
      }
      else if (key == '[')
      {
         qSlider->Halve();
      }
      else if (key == ']')
      {
         qSlider->Double();
      }
      else if ((toupper(key) == 'C' || toupper(key) == 'X') && GetKeyModifiers() == kModifier_Command)
      {
         TheSynth->CopyTextToClipboard(ofToString(qSlider->GetValue()));
      }
   }
}

void EQModule::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   for (auto& filter : mFilters)
   {
      if (filter.mEnabled)
      {
         if (slider == filter.mFSlider || slider == filter.mQSlider || slider == filter.mGSlider)
         {
            filter.mNeedToCalculateCoefficients = true;
         }
      }
   }
}

void EQModule::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   for (auto& filter : mFilters)
   {
      if (list == filter.mTypeSelector)
      {
         filter.mFilter[0].SetFilterType(filter.mFilter[0].mType);
         filter.mNeedToCalculateCoefficients = true;
      }
   }
}

void EQModule::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (auto& filter : mFilters)
   {
      if (checkbox == filter.mEnabledCheckbox)
      {
         filter.mFilter[0].Clear();
         filter.mFilter[1].Clear();
         mNeedToUpdateFrequencyResponseGraph = true;
      }
   }
}

void EQModule::GetModuleDimensions(float& w, float& h)
{
   w = MAX(208, mWidth);
   h = MAX(150, mHeight);
}

void EQModule::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
   mModuleSaveData.SetInt("width", w);
   mModuleSaveData.SetInt("height", h);
   mNeedToUpdateFrequencyResponseGraph = true;
}

void EQModule::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("width", moduleInfo, mWidth, 50, 2000, K(isTextField));
   mModuleSaveData.LoadInt("height", moduleInfo, mHeight, 50, 2000, K(isTextField));
   mModuleSaveData.LoadFloat("draw_gain", moduleInfo, 1, .1f, 4, K(isTextField));
   mModuleSaveData.LoadBool("lite_cpu_modulation", moduleInfo, true);

   SetUpFromSaveData();
}

void EQModule::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void EQModule::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mWidth = mModuleSaveData.GetInt("width");
   mHeight = mModuleSaveData.GetInt("height");
   mDrawGain = mModuleSaveData.GetFloat("draw_gain");
   mLiteCpuModulation = mModuleSaveData.GetBool("lite_cpu_modulation");
}