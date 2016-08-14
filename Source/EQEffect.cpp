//
//  EQEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/26/14.
//
//

#include "EQEffect.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "Profiler.h"

EQEffect::EQEffect()
: mNumFilters(NUM_EQ_FILTERS)
, mMultiSlider(NULL)
, mEvenButton(NULL)
{
   SetEnabled(true);
   
   mDryBufferSize = gBufferSize;
   mDryBuffer = new float[mDryBufferSize];
   
   for (int i=0; i<NUM_EQ_FILTERS; ++i)
   {
      mBiquad[i].SetFilterType(kFilterType_PeakNotch);
      mBiquad[i].SetFilterParams(40 * powf(2.2f,i), .1f);
   }
}

void EQEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mMultiSlider = new Grid(5,25,80,50,NUM_EQ_FILTERS,1);
   AddUIControl(mMultiSlider);
   mEvenButton = new ClickButton(this,"even",5,5);
   
   mMultiSlider->SetGridMode(Grid::kMultislider);
   mMultiSlider->SetClickClearsToZero(false);
   for (int i=0; i<NUM_EQ_FILTERS; ++i)
      mMultiSlider->SetValRefactor(0, i, .5f);
   mMultiSlider->SetListener(this);
}

EQEffect::~EQEffect()
{
}

void EQEffect::Init()
{
   IDrawableModule::Init();
}

void EQEffect::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("EQEffect");
   
   if (!mEnabled)
      return;
   
   if (bufferSize != mDryBufferSize)
   {
      delete mDryBuffer;
      mDryBufferSize = bufferSize;
      mDryBuffer = new float[mDryBufferSize];
   }
   
   ComputeSliders(0);
   
   for (int i=0; i<mNumFilters; ++i)
      mBiquad[i].Filter(audio,bufferSize);
}

void EQEffect::DrawModule()
{
   mMultiSlider->Draw();
   mEvenButton->Draw();
}

float EQEffect::GetEffectAmount()
{
   if (mEnabled)
   {
      float amount = 0;
      for (int i=0; i<mNumFilters; ++i)
      {
         amount += fabsf(mMultiSlider->GetVal(i,0) - .5f);
      }
      return amount;
   }
   return 0;
}

void EQEffect::GetModuleDimensions(int& width, int& height)
{
   width = 90;
   height = 80;
}

void EQEffect::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void EQEffect::RadioButtonUpdated(RadioButton* list, int oldVal)
{
}

void EQEffect::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      for (int i=0; i<NUM_EQ_FILTERS; ++i)
         mBiquad[i].Clear();
   }
}

void EQEffect::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void EQEffect::ButtonClicked(ClickButton* button)
{
   if (button == mEvenButton)
   {
      for (int i=0; i<NUM_EQ_FILTERS; ++i)
      {
         mMultiSlider->SetVal(i, 0, .5f);
         mBiquad[i].mDbGain = 0;
         mBiquad[i].UpdateFilterCoeff();
      }
   }
}

void EQEffect::GridUpdated(Grid* grid, int col, int row, float value, float oldValue)
{
   for (int i=0; i<mNumFilters; ++i)
   {
      mBiquad[i].mDbGain = ofMap(mMultiSlider->GetVal(i,0),0,1,-12,12);
      mBiquad[i].UpdateFilterCoeff();
   }
}
