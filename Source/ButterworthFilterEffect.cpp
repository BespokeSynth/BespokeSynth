//
//  ButterworthFilterEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#include "ButterworthFilterEffect.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "Profiler.h"

ButterworthFilterEffect::ButterworthFilterEffect()
: mF(2000)
, mFSlider(NULL)
, mQ(0)
, mQSlider(NULL)
, mCoefficientsHaveChanged(true)
{
   SetEnabled(true);
   
   mDryBufferSize = gBufferSize;
   mDryBuffer = new float[mDryBufferSize];
}

void ButterworthFilterEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mFSlider = new FloatSlider(this,"F",4,4,80,15,&mF,10,4000);
   mQSlider = new FloatSlider(this,"Q",4,20,80,15,&mQ,0,1);
   
   mFSlider->SetMaxValueDisplay("inf");
   mFSlider->SetMode(FloatSlider::kSquare);
}

ButterworthFilterEffect::~ButterworthFilterEffect()
{
   delete[] mDryBuffer;
}

void ButterworthFilterEffect::Init()
{
   IDrawableModule::Init();
}

void ButterworthFilterEffect::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("ButterworthFilterEffect");
   
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
   bool fadeOut = mF > fadeOutStart;
   if (fadeOut)
      memcpy(mDryBuffer, audio, bufferSize*sizeof(float));
   
   for (int i=0; i<bufferSize; ++i)
   {
      ComputeSliders(i);
      if (mCoefficientsHaveChanged)
      {
         mButterworth.Set(mF,mQ);
         mCoefficientsHaveChanged = false;
      }
      audio[i] = mButterworth.Run(audio[i]);
   }
   
   if (fadeOut)
   {
      float dryness = ofMap(mF,fadeOutStart,fadeOutEnd,0,1);
      Mult(audio,1-dryness,bufferSize);
      Mult(mDryBuffer,dryness,bufferSize);
      Add(audio,mDryBuffer,bufferSize);
   }
}

void ButterworthFilterEffect::DrawModule()
{
   mFSlider->Draw();
   mQSlider->Draw();
}

float ButterworthFilterEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp(1-(mF/(mFSlider->GetMax() * .75f)),0,1);
}

void ButterworthFilterEffect::GetModuleDimensions(int& width, int& height)
{
   width = 90;
   height = 69;
}

void ButterworthFilterEffect::ResetFilter()
{
   mButterworth.Clear();
}

void ButterworthFilterEffect::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void ButterworthFilterEffect::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      ResetFilter();
   }
}

void ButterworthFilterEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mFSlider || slider == mQSlider)
      mCoefficientsHaveChanged = true;
}

void ButterworthFilterEffect::LoadLayout(const ofxJSONElement& info)
{
   mModuleSaveData.LoadFloat("f_min", info, 10, 1, 40000, K(isTextField));
   mModuleSaveData.LoadFloat("f_max", info, 10000, 1, 40000, K(isTextField));
}

void ButterworthFilterEffect::SetUpFromSaveData()
{
   mFSlider->SetExtents(mModuleSaveData.GetFloat("f_min"), mModuleSaveData.GetFloat("f_max"));
   ResetFilter();
}

void ButterworthFilterEffect::SaveLayout(ofxJSONElement& info)
{
   mModuleSaveData.Save(info);
}
