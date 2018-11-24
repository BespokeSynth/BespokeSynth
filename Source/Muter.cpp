//
//  Muter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/26/13.
//
//

#include "Muter.h"
#include "SynthGlobals.h"
#include "Profiler.h"

Muter::Muter()
: mPass(false)
, mPassCheckbox(nullptr)
, mRampTimeMs(3)
{
   mRamp.SetValue(0);
}

void Muter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mPassCheckbox = new Checkbox(this,"pass",5,2,&mPass);
   mRampTimeSlider = new FloatSlider(this,"ms",5,20,70,15,&mRampTimeMs,3,1000);
   mRampTimeSlider->SetMode(FloatSlider::kSquare);
}

Muter::~Muter()
{
}

void Muter::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(Muter);

   float bufferSize = buffer->BufferSize();
   
   for (int i=0; i<bufferSize; ++i)
   {
      for (int ch=0; ch<buffer->NumActiveChannels(); ++ch)
         buffer->GetChannel(ch)[i] *= mRamp.Value(time);
      time += gInvSampleRateMs;
   }
}

void Muter::DrawModule()
{
   mPassCheckbox->Draw();
   mRampTimeSlider->Draw();
}

void Muter::CheckboxUpdated(Checkbox* checkbox)
{
   mRamp.Start(mPass ? 1 : 0, mRampTimeMs);
}

