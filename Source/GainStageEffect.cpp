/*
  ==============================================================================

    GainStage.cpp
    Created: 24 Apr 2021 3:47:25pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "GainStageEffect.h"
#include "SynthGlobals.h"
#include "Profiler.h"

GainStageEffect::GainStageEffect()
: mGain(1)
{
}

void GainStageEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGainSlider = new FloatSlider(this,"gain",5,2,110,15,&mGain,0,4);
}

void GainStageEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(GainStageEffect);

   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);
      for (int ch = 0; ch < buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= mGain;
   }
}

void GainStageEffect::DrawModule()
{
   mGainSlider->Draw();
}

void GainStageEffect::CheckboxUpdated(Checkbox *checkbox)
{
}

void GainStageEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

