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
//  StereoWidthEffect.h
//  Bespoke
//
//  Mid/Side stereo width utility (inspired by Kilohearts' "Stereo"). Splits the signal into Mid
//  (L+R) and Side (L-R), scales each, then recombines:
//    - width : 0 = fully mono (Side muted), 1 = unchanged, up to 2 = extra-wide.
//    - mid   : level of the centre (mono) component.
//    - pan   : left/right balance of the output.
//  Great for collapsing a stereo source to mono (width down) or widening it (width up). Widening a
//  truly mono source has no effect, since there is no Side information to expand.
//

#pragma once

#include "IAudioEffect.h"
#include "Slider.h"

class StereoWidthEffect : public IAudioEffect, public IFloatSliderListener
{
public:
   StereoWidthEffect() {}
   ~StereoWidthEffect() {}

   static IAudioEffect* Create() { return new StereoWidthEffect(); }

   void CreateUIControls() override;

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "stereo"; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 100;
      height = 56;
   }

   float mWidthAmt{ 1.0f }; //0 = mono, 1 = normal, 2 = wide
   float mMid{ 1.0f };
   float mPan{ 0.0f }; //-1 left .. +1 right
   FloatSlider* mWidthSlider{ nullptr };
   FloatSlider* mMidSlider{ nullptr };
   FloatSlider* mPanSlider{ nullptr };
};
