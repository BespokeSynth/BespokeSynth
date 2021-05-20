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
: mTypeSelector(nullptr)
, mFSlider(nullptr)
, mQSlider(nullptr)
, mGSlider(nullptr)
, mMouseControl(false)
, mCoefficientsHaveChanged(true)
, mDryBuffer(gBufferSize)
{
   SetEnabled(true);
}

void BiquadFilterEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTypeSelector = new RadioButton(this,"type",4,52,(int*)(&mBiquad[0].mType),kRadioHorizontal);
   mFSlider = new FloatSlider(this,"F",4,4,80,15,&mBiquad[0].mF,10,4000);
   mQSlider = new FloatSlider(this,"Q",4,20,80,15,&mBiquad[0].mQ,.1f,18,3);
   mGSlider = new FloatSlider(this,"G",4,36,80,15,&mBiquad[0].mDbGain,-96,96,1);
   
   mTypeSelector->AddLabel("lp", kFilterType_Lowpass);
   mTypeSelector->AddLabel("hp", kFilterType_Highpass);
   mTypeSelector->AddLabel("bp", kFilterType_Bandpass);
   mTypeSelector->AddLabel("pk", kFilterType_Peak);
   mTypeSelector->AddLabel("ap", kFilterType_Allpass);
   
   mFSlider->SetMaxValueDisplay("inf");
   mFSlider->SetMode(FloatSlider::kSquare);
   mQSlider->SetMode(FloatSlider::kSquare);
   mQSlider->SetShowing(mBiquad[0].UsesQ());
   mGSlider->SetShowing(mBiquad[0].UsesGain());
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
   
   for (int i=0; i<bufferSize; ++i)
   {
      ComputeSliders(i);
      if (mCoefficientsHaveChanged)
      {
         mBiquad[0].UpdateFilterCoeff();
         for (int ch=1;ch<buffer->NumActiveChannels(); ++ch)
            mBiquad[ch].CopyCoeffFrom(mBiquad[0]);
         mCoefficientsHaveChanged = false;
      }
      for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] = mBiquad[ch].Filter(buffer->GetChannel(ch)[i]);
   }
   
   if (fadeOut)
   {
      for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
      {
         float dryness = ofMap(mBiquad[0].mF,fadeOutStart,fadeOutEnd,0,1);
         Mult(buffer->GetChannel(ch),1-dryness,bufferSize);
         Mult(mDryBuffer.GetChannel(ch),dryness,bufferSize);
         Add(buffer->GetChannel(ch),mDryBuffer.GetChannel(ch),bufferSize);
      }
   }
}

void BiquadFilterEffect::DrawModule()
{
   mTypeSelector->Draw();

   mFSlider->Draw();
   mQSlider->Draw();
   mGSlider->Draw();

   auto FreqForPos = [](float pos) { return 20.0 * std::pow(2.0, pos * 10); };

   float w, h;
   GetModuleDimensions(w, h);
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

float BiquadFilterEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   if (mBiquad[0].mType == kFilterType_Lowpass)
      return ofClamp(1-(mBiquad[0].mF/(mFSlider->GetMax() * .75f)),0,1);
   if (mBiquad[0].mType == kFilterType_Highpass)
      return ofClamp(mBiquad[0].mF/(mFSlider->GetMax() * .75f),0,1);
   if (mBiquad[0].mType == kFilterType_Bandpass)
      return ofClamp(.3f+(mBiquad[0].mQ/mQSlider->GetMax()),0,1);
   if (mBiquad[0].mType == kFilterType_Peak)
      return ofClamp(fabsf(mBiquad[0].mDbGain/96),0,1);
   return 0;
}

void BiquadFilterEffect::GetModuleDimensions(float& width, float& height)
{
   width = 120;
   height = 69;
}

void BiquadFilterEffect::ResetFilter()
{
   if (mBiquad[0].mType == kFilterType_Lowpass)
      mBiquad[0].SetFilterParams(mFSlider->GetMax(), sqrt(2) / 2);
   if (mBiquad[0].mType == kFilterType_Highpass)
      mBiquad[0].SetFilterParams(mFSlider->GetMin(), sqrt(2) / 2);
   
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
      mBiquad[i].Clear();
}

void BiquadFilterEffect::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void BiquadFilterEffect::RadioButtonUpdated(RadioButton* list, int oldVal)
{
   if (list == mTypeSelector)
   {
      if (mBiquad[0].mType == kFilterType_Lowpass)
         mBiquad[0].SetFilterParams(mFSlider->GetMax(), sqrt(2) / 2);
      if (mBiquad[0].mType == kFilterType_Highpass)
         mBiquad[0].SetFilterParams(mFSlider->GetMin(), sqrt(2) / 2);
      mQSlider->SetShowing(mBiquad[0].UsesQ());
      mGSlider->SetShowing(mBiquad[0].UsesGain());
      mCoefficientsHaveChanged = true;
   }
}

bool BiquadFilterEffect::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);

   if (mMouseControl)
   {
      float thisx,thisy;
      GetPosition(thisx, thisy);
      x += thisx;
      y += thisy;
      mFSlider->SetValue(x*2+150);
      mQSlider->SetValue(y/100.0f);
   }

   return false;
}

void BiquadFilterEffect::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
         mBiquad[i].Clear();
   }
}

void BiquadFilterEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
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

