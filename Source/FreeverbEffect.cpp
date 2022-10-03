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
{
   //mFreeverb.setmode(GetParameter(KMode));
   mFreeverb.setroomsize(mRoomSize);
   mFreeverb.setdamp(mDamp);
   mFreeverb.setwet(mWet);
   mFreeverb.setdry(mDry);
   mFreeverb.setwidth(mVerbWidth);
   mFreeverb.update();
}

FreeverbEffect::~FreeverbEffect()
{
}

void FreeverbEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRoomSizeSlider = new FloatSlider(this, "room size", 5, 4, 85, 15, &mRoomSize, .1f, .99f, 2);
   mDampSlider = new FloatSlider(this, "damp", 5, 20, 85, 15, &mDamp, 0, 100);
   mWetSlider = new FloatSlider(this, "wet", 5, 36, 85, 15, &mWet, 0, 1);
   mDrySlider = new FloatSlider(this, "dry", 5, 52, 85, 15, &mDry, 0, 1);
   mWidthSlider = new FloatSlider(this, "width", 5, 68, 85, 15, &mVerbWidth, 0, 100);
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

void FreeverbEffect::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void FreeverbEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
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
