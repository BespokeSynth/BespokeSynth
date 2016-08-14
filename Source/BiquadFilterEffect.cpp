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
: mTypeSelector(NULL)
, mFSlider(NULL)
, mQSlider(NULL)
, mGSlider(NULL)
, mMouseControl(false)
, mCoefficientsHaveChanged(true)
{
   SetEnabled(true);
   
   mDryBufferSize = gBufferSize;
   mDryBuffer = new float[mDryBufferSize];
}

void BiquadFilterEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mTypeSelector = new RadioButton(this,"type",4,52,(int*)(&mBiquad.mType),kRadioHorizontal);
   mFSlider = new FloatSlider(this,"F",4,4,80,15,&mBiquad.mF,10,4000);
   mQSlider = new FloatSlider(this,"Q",4,20,80,15,&mBiquad.mQ,1,10);
   mGSlider = new FloatSlider(this,"G",4,36,80,15,&mBiquad.mDbGain,-96,96,1);
   
   mTypeSelector->AddLabel("lp", kFilterType_Lowpass);
   mTypeSelector->AddLabel("hp", kFilterType_Highpass);
   mTypeSelector->AddLabel("bp", kFilterType_Bandpass);
   mTypeSelector->AddLabel("pn", kFilterType_PeakNotch);
   
   mFSlider->SetMaxValueDisplay("inf");
   mFSlider->SetMode(FloatSlider::kSquare);
   mGSlider->SetShowing(mBiquad.mType == kFilterType_PeakNotch);
}

BiquadFilterEffect::~BiquadFilterEffect()
{
   delete[] mDryBuffer;
}

void BiquadFilterEffect::Init()
{
   IDrawableModule::Init();
}

void BiquadFilterEffect::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("BiquadFilterEffect");

   if (!mEnabled)
      return;

   if (bufferSize != mDryBufferSize)
   {
      delete mDryBuffer;
      mDryBufferSize = bufferSize;
      mDryBuffer = new float[mDryBufferSize];
   }
   
   const float fadeOutStart = mFSlider->GetMax() * .75f;
   const float fadeOutEnd = mFSlider->GetMax();
   bool fadeOut = mBiquad.mF > fadeOutStart && mBiquad.mType == kFilterType_Lowpass;
   if (fadeOut)
      memcpy(mDryBuffer, audio, bufferSize*sizeof(float));
   
   for (int i=0; i<bufferSize; ++i)
   {
      ComputeSliders(i);
      if (mCoefficientsHaveChanged)
      {
         mBiquad.UpdateFilterCoeff();
         mCoefficientsHaveChanged = false;
      }
      audio[i] = mBiquad.Filter(audio[i]);
   }
   
   if (fadeOut)
   {
      float dryness = ofMap(mBiquad.mF,fadeOutStart,fadeOutEnd,0,1);
      Mult(audio,1-dryness,bufferSize);
      Mult(mDryBuffer,dryness,bufferSize);
      Add(audio,mDryBuffer,bufferSize);
   }
}

void BiquadFilterEffect::DrawModule()
{
   mTypeSelector->Draw();

   mFSlider->Draw();
   mQSlider->Draw();
   mGSlider->Draw();
}

float BiquadFilterEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   if (mBiquad.mType == kFilterType_Lowpass)
      return ofClamp(1-(mBiquad.mF/(mFSlider->GetMax() * .75f)),0,1);
   if (mBiquad.mType == kFilterType_Highpass)
      return ofClamp(mBiquad.mF/(mFSlider->GetMax() * .75f),0,1);
   if (mBiquad.mType == kFilterType_Bandpass)
      return ofClamp(.3f+(mBiquad.mQ/mQSlider->GetMax()),0,1);
   if (mBiquad.mType == kFilterType_PeakNotch)
      return ofClamp(fabsf(mBiquad.mDbGain/96),0,1);
   return 0;
}

void BiquadFilterEffect::GetModuleDimensions(int& width, int& height)
{
   width = 90;
   height = 69;
}

void BiquadFilterEffect::ResetFilter()
{
   if (mBiquad.mType == kFilterType_Lowpass)
      mBiquad.SetFilterParams(mFSlider->GetMax(), mQSlider->GetMin());
   if (mBiquad.mType == kFilterType_Highpass)
      mBiquad.SetFilterParams(mFSlider->GetMin(), mQSlider->GetMin());
   mBiquad.Clear();
}

void BiquadFilterEffect::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void BiquadFilterEffect::RadioButtonUpdated(RadioButton* list, int oldVal)
{
   if (list == mTypeSelector)
   {
      if (mBiquad.mType == kFilterType_Lowpass)
         mBiquad.SetFilterParams(mFSlider->GetMax(), mQSlider->GetMin());
      if (mBiquad.mType == kFilterType_Highpass)
         mBiquad.SetFilterParams(mFSlider->GetMin(), mQSlider->GetMin());
      mGSlider->SetShowing(mBiquad.mType == kFilterType_PeakNotch);
   }
}

bool BiquadFilterEffect::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);

   if (mMouseControl)
   {
      int thisx,thisy;
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
      ResetFilter();
   }
}

void BiquadFilterEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mFSlider || slider == mQSlider || slider == mGSlider)
      mCoefficientsHaveChanged = true;
}

void BiquadFilterEffect::LoadLayout(const ofxJSONElement& info)
{
   mModuleSaveData.LoadFloat("f_min", info, 10, 1, 40000, K(isTextField));
   mModuleSaveData.LoadFloat("f_max", info, 4000, 1, 40000, K(isTextField));
   mModuleSaveData.LoadFloat("q_min", info, 1, .1f, 50, K(isTextField));
   mModuleSaveData.LoadFloat("q_max", info, 10, .1f, 50, K(isTextField));
}

void BiquadFilterEffect::SetUpFromSaveData()
{
   mFSlider->SetExtents(mModuleSaveData.GetFloat("f_min"), mModuleSaveData.GetFloat("f_max"));
   mQSlider->SetExtents(mModuleSaveData.GetFloat("q_min"), mModuleSaveData.GetFloat("q_max"));
   ResetFilter();
}

void BiquadFilterEffect::SaveLayout(ofxJSONElement& info)
{
   mModuleSaveData.Save(info);
}

