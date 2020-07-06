//
//  FreeverbEffect.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/19/14.
//
//

#include "FreeverbEffect.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Profiler.h"

FreeverbEffect::FreeverbEffect()
: mRoomSizeSlider(nullptr)
, mDampSlider(nullptr)
, mWetSlider(nullptr)
, mDrySlider(nullptr)
, mWidthSlider(nullptr)
, mNeedUpdate(false)
{
   //mFreeverb.setmode(GetParameter(KMode));
   //mFreeverb.setroomsize(GetParameter(KRoomSize));
   mFreeverb.setdamp(50);
   mFreeverb.setwet(.5f);
   mFreeverb.setdry(1);
   //mFreeverb.setwidth(GetParameter(KWidth));
   mFreeverb.update();
   
   mFreeze = false;
   mRoomSize = mFreeverb.getroomsize();
   mDamp = mFreeverb.getdamp();
   mWet = mFreeverb.getwet();
   mDry = mFreeverb.getdry();
   mVerbWidth = mFreeverb.getwidth();
}

FreeverbEffect::~FreeverbEffect()
{
}

void FreeverbEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRoomSizeSlider = new FloatSlider(this,"room size",5,4,85,15,&mRoomSize,.1f,.99f,2);
   mDampSlider = new FloatSlider(this,"damp",5,20,85,15,&mDamp,0,100);
   mWetSlider = new FloatSlider(this,"wet",5,36,85,15,&mWet,0,1);
   mDrySlider = new FloatSlider(this,"dry",5,52,85,15,&mDry,0,1);
   mWidthSlider = new FloatSlider(this,"width",5,68,85,15,&mVerbWidth,0,1);
}

void FreeverbEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(FreeverbEffect);
   
   if (!mEnabled)
      return;
   
   float bufferSize = buffer->BufferSize();
   
   ComputeSliders(0);
   
   if (mNeedUpdate)
   {
      mFreeverb.update();
      mNeedUpdate = false;
   }
   
   int secondChannel = 1;
   if (buffer->NumActiveChannels() <= 1)
      secondChannel = 0;
   
   mFreeverb.processreplace(buffer->GetChannel(0), buffer->GetChannel(secondChannel), buffer->GetChannel(0), buffer->GetChannel(secondChannel), bufferSize, 1);
}

void FreeverbEffect::DrawModule()
{
   if (!mEnabled)
      return;
   
   mRoomSizeSlider->Draw();
   mDampSlider->Draw();
   mWetSlider->Draw();
   mDrySlider->Draw();
   mWidthSlider->Draw();
}

void FreeverbEffect::GetModuleDimensions(float& width, float& height)
{
   if (mEnabled)
   {
      width = 95;
      height = 84;
   }
   else
   {
      width = 95;
      height = 0;
   }
}

float FreeverbEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return mWet;
}

void FreeverbEffect::CheckboxUpdated(Checkbox* checkbox)
{
}

void FreeverbEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mRoomSizeSlider)
   {
      mFreeverb.setroomsize(mRoomSize);
      mNeedUpdate = true;
   }
   if (slider == mDampSlider)
   {
      mFreeverb.setdamp(mDamp);
      mNeedUpdate = true;
   }
   if (slider == mWetSlider)
   {
      mFreeverb.setwet(mWet);
      mNeedUpdate = true;
   }
   if (slider == mDrySlider)
   {
      mFreeverb.setdry(mDry);
      mNeedUpdate = true;
   }
   if (slider == mWidthSlider)
   {
      mFreeverb.setwidth(mVerbWidth);
      mNeedUpdate = true;
   }
}

