//
//  PitchShiftEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/21/15.
//
//

#include "PitchShiftEffect.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Profiler.h"

PitchShiftEffect::PitchShiftEffect()
: mRatio(1)
, mRatioSlider(nullptr)
, mRatioSelector(nullptr)
, mRatioSelection(10)
{
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
      mPitchShifter[i] = new PitchShifter(1024);
}

PitchShiftEffect::~PitchShiftEffect()
{
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
      delete mPitchShifter[i];
}

void PitchShiftEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRatioSlider = new FloatSlider(this,"ratio",5,4,85,15,&mRatio,.5f,2.0f);
   mRatioSelector = new RadioButton(this,"ratioselector",5,20,&mRatioSelection,kRadioHorizontal);
   
   mRatioSelector->AddLabel(".5", 5);
   mRatioSelector->AddLabel("1", 10);
   mRatioSelector->AddLabel("1.5", 15);
   mRatioSelector->AddLabel("2", 20);
}

void PitchShiftEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(PitchShiftEffect);
   
   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();
   
   ComputeSliders(0);
   
   for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
   {
      mPitchShifter[ch]->SetRatio(mRatio);
      mPitchShifter[ch]->Process(buffer->GetChannel(ch), bufferSize);
   }
}

void PitchShiftEffect::DrawModule()
{
   if (!mEnabled)
      return;
   
   mRatioSlider->Draw();
   mRatioSelector->Draw();
}

void PitchShiftEffect::GetModuleDimensions(float& width, float& height)
{
   if (mEnabled)
   {
      width = 105;
      height = 39;
   }
   else
   {
      width = 105;
      height = 0;
   }
}

float PitchShiftEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return ofClamp(fabsf((mRatio-1)*10),0,1);
}

void PitchShiftEffect::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void PitchShiftEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mRatioSlider)
      mRatioSelection = -1;
}

void PitchShiftEffect::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
   if (radio == mRatioSelector)
      mRatio = mRatioSelection / 10.0f;
}
