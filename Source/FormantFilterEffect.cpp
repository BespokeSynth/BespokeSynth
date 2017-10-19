//
//  FormantFilterEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/21/16.
//
//

#include "FormantFilterEffect.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "Profiler.h"

FormantFilterEffect::FormantFilterEffect()
: mEE(1)
, mOO(0)
, mI(0)
, mE(0)
, mU(0)
, mA(0)
, mRescaling(false)
{
   SetEnabled(true);
   
   mOutputBuffer = new float[gBufferSize];
   
   for (int i=0; i<NUM_FORMANT_BANDS; ++i)
      mBiquads[i].SetFilterType(kFilterType_Bandpass);
   
   mFormants.push_back(Formants(400,1,  1700,.35f,   2300,.4f));  //EE
   mFormants.push_back(Formants(360,1,   750,.25f,   2400,.035f));//OO
   mFormants.push_back(Formants(238,1,  1741,.1f,    2450,.15f)); //I
   mFormants.push_back(Formants(300,1,  1600,.2f,    2150,.25f)); //E
   mFormants.push_back(Formants(415,1,  1400,.25f,   2200,.15f)); //U
   mFormants.push_back(Formants(609,1,  1000,.5f,    2450,.25f)); //A
}

void FormantFilterEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mEESlider = new FloatSlider(this,"ee",4,4,80,15,&mEE,0,1);
   mOOSlider = new FloatSlider(this,"oo",4,4,80,15,&mOO,0,1);
   mISlider = new FloatSlider(this,"i",4,4,80,15,&mI,0,1);
   mESlider = new FloatSlider(this,"e",4,4,80,15,&mE,0,1);
   mUSlider = new FloatSlider(this,"u",4,4,80,15,&mU,0,1);
   mASlider = new FloatSlider(this,"a",4,4,80,15,&mA,0,1);
   mOOSlider->PositionTo(mEESlider, kAnchorDirection_Below);
   mISlider->PositionTo(mOOSlider, kAnchorDirection_Below);
   mESlider->PositionTo(mISlider, kAnchorDirection_Below);
   mUSlider->PositionTo(mESlider, kAnchorDirection_Below);
   mASlider->PositionTo(mUSlider, kAnchorDirection_Below);
   mSliders.push_back(mEESlider);
   mSliders.push_back(mOOSlider);
   mSliders.push_back(mISlider);
   mSliders.push_back(mESlider);
   mSliders.push_back(mUSlider);
   mSliders.push_back(mASlider);
}

FormantFilterEffect::~FormantFilterEffect()
{
}

void FormantFilterEffect::Init()
{
   IDrawableModule::Init();
}

void FormantFilterEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   Profiler profiler("FormantFilterEffect");
   
   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();
   
   ComputeSliders(0);
   
   assert(gBufferSize == bufferSize);
   Clear(mOutputBuffer, bufferSize);
   
   //TODO(Ryan)
   /*for (int i=0; i<NUM_FORMANT_BANDS; ++i)
   {
      BufferCopy(gWorkBuffer, audio, bufferSize);
      mBiquads[i].Filter(gWorkBuffer, bufferSize);
      Add(mOutputBuffer, gWorkBuffer, bufferSize);
   }
   
   BufferCopy(audio, gWorkBuffer, bufferSize);*/
}

void FormantFilterEffect::DrawModule()
{
   mEESlider->Draw();
   mOOSlider->Draw();
   mISlider->Draw();
   mESlider->Draw();
   mUSlider->Draw();
   mASlider->Draw();
}

float FormantFilterEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return 1;
}

void FormantFilterEffect::GetModuleDimensions(int& width, int& height)
{
   width = 90;
   height = 100;
}

void FormantFilterEffect::ResetFilters()
{
   for (int i=0; i<NUM_FORMANT_BANDS; ++i)
      mBiquads[i].Clear();
}

void FormantFilterEffect::UpdateFilters()
{
   assert(NUM_FORMANT_BANDS == 3);
   
   float total = 0;
   for (int i=0; i<mSliders.size(); ++i)
      total += mSliders[i]->GetValue();
   
   if (total == 0)
      return;
   
   vector<float> formant;
   formant.resize(NUM_FORMANT_BANDS);
   formant.assign(NUM_FORMANT_BANDS, 0);
   
   assert(mSliders.size() == mFormants.size());
   for (int i=0; i<mSliders.size(); ++i)
   {
      float weight = mSliders[i]->GetValue() / total;
      for (int j=0; j<NUM_FORMANT_BANDS; ++j)
         formant[j] += mFormants[i].mFreqs[j] * mFormants[i].mGains[j] * weight;
   }
   
   const float bandwidth = 100;
   for (int i=0; i<NUM_FORMANT_BANDS; ++i)
      mBiquads[i].SetFilterParams(formant[i], formant[i]/(bandwidth/2));
}

void FormantFilterEffect::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void FormantFilterEffect::RadioButtonUpdated(RadioButton* list, int oldVal)
{
}

void FormantFilterEffect::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      ResetFilters();
   }
}

void FormantFilterEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (!mRescaling)
   {
      mRescaling = true;
      for (int i=0; i<mSliders.size(); ++i)
      {
         if (mSliders[i] != slider)
         {
            mSliders[i]->SetValue(mSliders[i]->GetValue() * (1 - (slider->GetValue() - oldVal)));
         }
      }
      UpdateFilters();
      mRescaling = false;
   }
}

void FormantFilterEffect::LoadLayout(const ofxJSONElement& info)
{
}

void FormantFilterEffect::SetUpFromSaveData()
{
   ResetFilters();
}

void FormantFilterEffect::SaveLayout(ofxJSONElement& info)
{
   mModuleSaveData.Save(info);
}

