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
//  AbletonLink.h
//
//  Created by Ryan Challinor on 12/9/21.
//
//

#pragma once

#include <iostream>
#include <memory>

#include "IDrawableModule.h"
#include "Checkbox.h"
#include "IAudioPoller.h"
#include "Slider.h"

namespace ableton
{
   class Link;
}

class AbletonLink : public IDrawableModule, public IAudioPoller, public IFloatSliderListener
{
public:
   AbletonLink();
   virtual ~AbletonLink();
   static IDrawableModule* Create() { return new AbletonLink(); }

   void Init() override;
   void CreateUIControls() override;
   void Poll() override;

   void OnTransportAdvanced(float amount) override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   bool Enabled() const override { return mEnabled; }

   float mWidth;
   float mHeight;

   float mOffsetMs{ 0 };
   FloatSlider* mOffsetMsSlider;

   std::unique_ptr<ableton::Link> mLink;
   double mTempo = 120.0;
   std::size_t mNumPeers = 0;
   double mLastReceivedBeat{ 0 };
   double mSampleTime{ 0 };
};
