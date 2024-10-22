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
//  BandVocoder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/1/14.
//
//

#include "BandVocoder.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"

BandVocoder::BandVocoder()
: IAudioProcessor(gBufferSize)
{
   mCarrierInputBuffer = new float[GetBuffer()->BufferSize()];
   Clear(mCarrierInputBuffer, GetBuffer()->BufferSize());

   mWorkBuffer = new float[GetBuffer()->BufferSize()];
   Clear(mWorkBuffer, GetBuffer()->BufferSize());

   mOutBuffer = new float[GetBuffer()->BufferSize()];
   Clear(mOutBuffer, GetBuffer()->BufferSize());

   for (int i = 0; i < VOCODER_MAX_BANDS; ++i)
   {
      mPeaks[i].SetDecayTime(mRingTime);
      mOutputPeaks[i].SetDecayTime(mRingTime);
      mPeaks[i].SetLimit(mMaxBand);
      mOutputPeaks[i].SetLimit(mMaxBand);
   }

   CalcFilters();
}

void BandVocoder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   FLOATSLIDER(mInputSlider, "input", &mInputPreamp, 0.1f, 2);
   FLOATSLIDER(mCarrierSlider, "carrier", &mCarrierPreamp, .1f, 2);
   FLOATSLIDER(mVolumeSlider, "volume", &mVolume, .1f, 2);
   FLOATSLIDER(mDryWetSlider, "mix", &mDryWet, 0, 1);
   FLOATSLIDER(mMaxBandSlider, "max band", &mMaxBand, 0.001f, 1);
   FLOATSLIDER(mSpacingStyleSlider, "spacing", &mSpacingStyle, -1, 1);
   UIBLOCK_NEWCOLUMN();
   INTSLIDER(mNumBandsSlider, "bands", &mNumBands, 2, VOCODER_MAX_BANDS);
   FLOATSLIDER(mFBaseSlider, "f base", &mFreqBase, 20, 300);
   FLOATSLIDER(mFRangeSlider, "f range", &mFreqRange, 0, gSampleRate / 2 - 1000);
   FLOATSLIDER_DIGITS(mQSlider, "q", &mQ, 20, 80, 3);
   FLOATSLIDER_DIGITS(mRingTimeSlider, "ring", &mRingTime, .0001f, .1f, 4);
   ENDUIBLOCK0();

   mFRangeSlider->SetMode(FloatSlider::kSquare);
}

BandVocoder::~BandVocoder()
{
   delete[] mCarrierInputBuffer;
   delete[] mWorkBuffer;
}

void BandVocoder::SetCarrierBuffer(float* carrier, int bufferSize)
{
   assert(bufferSize == GetBuffer()->BufferSize());
   BufferCopy(mCarrierInputBuffer, carrier, bufferSize);
   mCarrierDataSet = true;
}

void BandVocoder::Process(double time)
{
   PROFILER(BandVocoder);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();

   if (!mEnabled)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();
      return;
   }

   ComputeSliders(0);

   float inputPreampSq = mInputPreamp * mInputPreamp;
   float carrierPreampSq = mCarrierPreamp * mCarrierPreamp;
   float volSq = mVolume * mVolume;

   int bufferSize = GetBuffer()->BufferSize();

   Clear(mOutBuffer, bufferSize);

   Mult(GetBuffer()->GetChannel(0), inputPreampSq * 5, bufferSize);
   Mult(mCarrierInputBuffer, carrierPreampSq * 5, bufferSize);

   for (int i = 0; i < mNumBands; ++i)
   {
      //get modulator band
      BufferCopy(mWorkBuffer, GetBuffer()->GetChannel(0), bufferSize);
      mBiquadCarrier[i].Filter(mWorkBuffer, bufferSize);

      float oldPeak = mPeaks[i].GetPeak();

      //calculate modulator band level
      mPeaks[i].Process(mWorkBuffer, bufferSize);

      //get carrier band
      BufferCopy(mWorkBuffer, mCarrierInputBuffer, bufferSize);
      mBiquadOut[i].Filter(mWorkBuffer, bufferSize);

      //multiply carrier band by modulator band level
      //Mult(mWorkBuffer, mPeaks[i].GetPeak(), bufferSize);
      for (int j = 0; j < bufferSize; ++j)
         mWorkBuffer[j] *= ofMap(j, 0, bufferSize, oldPeak, mPeaks[i].GetPeak());

      //don't allow a band to go crazy
      /*mOutputPeaks[i].Process(mWorkBuffer, bufferSize);
      if (mOutputPeaks[i].GetPeak() > mMaxBand)
         Mult(mWorkBuffer, 1/mOutputPeaks[i].GetPeak(), bufferSize);*/

      //accumulate output band into total output
      Add(mOutBuffer, mWorkBuffer, bufferSize);
   }

   Mult(mOutBuffer, mDryWet * volSq, bufferSize);
   Mult(GetBuffer()->GetChannel(0), (1 - mDryWet), bufferSize);
   Add(mOutBuffer, GetBuffer()->GetChannel(0), bufferSize);

   Add(target->GetBuffer()->GetChannel(0), mOutBuffer, bufferSize);

   GetVizBuffer()->WriteChunk(mOutBuffer, bufferSize, 0);

   GetBuffer()->Reset();
}

void BandVocoder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mInputSlider->Draw();
   mCarrierSlider->Draw();
   mVolumeSlider->Draw();
   mDryWetSlider->Draw();
   mFBaseSlider->Draw();
   mFRangeSlider->Draw();
   mQSlider->Draw();
   mNumBandsSlider->Draw();
   mRingTimeSlider->Draw();
   mMaxBandSlider->Draw();
   mSpacingStyleSlider->Draw();

   float w, h;
   GetModuleDimensions(w, h);

   auto PosForFreq = [](float freq)
   {
      return log2(freq / 20) / 10;
   };
   ofSetColor(0, 255, 0);
   for (int i = 0; i < mNumBands; ++i)
   {
      float x = PosForFreq(mBiquadCarrier[i].mF) * w;
      ofLine(x, h, x, h - mPeaks[i].GetPeak() * 200);
   }

   auto FreqForPos = [](float pos)
   {
      return 20.0 * std::pow(2.0, pos * 10);
   };
   ofSetColor(52, 204, 235, 70);
   ofSetLineWidth(1);
   for (int i = 0; i < mNumBands; ++i)
   {
      ofBeginShape();
      const int kPixelStep = 1;
      for (int x = 0; x < w + kPixelStep; x += kPixelStep)
      {
         float freq = FreqForPos(x / w);
         if (freq < gSampleRate / 2)
         {
            float response = mBiquadCarrier[i].GetMagnitudeResponseAt(freq);
            ofVertex(x, (.5f - .666f * log10(response)) * h);
         }
      }
      ofEndShape(false);
   }

   if (!mCarrierDataSet)
   {
      ofPushStyle();
      ofSetColor(255, 0, 0);
      DrawTextNormal("no vocodercarrier!", 5, h - 5);
      ofPopStyle();
   }
}

void BandVocoder::CalcFilters()
{
   for (int i = 0; i < mNumBands; ++i)
   {
      float a = float(i) / (mNumBands - 1);
      float freqMax = ofClamp(mFreqBase + mFreqRange, 0, gSampleRate / 2);
      float fExp = mFreqBase * powf(freqMax / mFreqBase, a);
      float fLin = ofLerp(mFreqBase, freqMax, a);
      float fBass = ofLerp(mFreqBase, freqMax, a * a * a * a);
      float f;
      if (mSpacingStyle >= 0)
         f = ofLerp(fExp, fLin, mSpacingStyle);
      else
         f = ofLerp(fExp, fBass, -mSpacingStyle);

      mBiquadCarrier[i].SetFilterType(kFilterType_Bandpass);
      mBiquadOut[i].SetFilterType(kFilterType_Bandpass);
      mBiquadCarrier[i].SetFilterParams(f, mQ);
      mBiquadOut[i].CopyCoeffFrom(mBiquadCarrier[i]);
   }
}

void BandVocoder::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < VOCODER_MAX_BANDS; ++i)
      {
         mBiquadCarrier[i].Clear();
         mBiquadOut[i].Clear();
      }
   }
}

void BandVocoder::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumBandsSlider)
   {
      CalcFilters();
   }
}

void BandVocoder::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mFBaseSlider || slider == mFRangeSlider || slider == mQSlider || slider == mSpacingStyleSlider)
   {
      CalcFilters();
   }
   if (slider == mRingTimeSlider)
   {
      for (int i = 0; i < VOCODER_MAX_BANDS; ++i)
      {
         mPeaks[i].SetDecayTime(mRingTime);
         mOutputPeaks[i].SetDecayTime(mRingTime);
      }
   }
   if (slider == mMaxBandSlider)
   {
      for (int i = 0; i < VOCODER_MAX_BANDS; ++i)
      {
         mPeaks[i].SetLimit(mMaxBand);
         mOutputPeaks[i].SetLimit(mMaxBand);
      }
   }
}

void BandVocoder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void BandVocoder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
