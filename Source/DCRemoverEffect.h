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
//  DCRemoverEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/2/14.
//
//

#pragma once

#include "IAudioEffect.h"
#include "BiquadFilter.h"

class DCRemoverEffect : public IAudioEffect
{
public:
   DCRemoverEffect();
   ~DCRemoverEffect();

   static IAudioEffect* Create() { return new DCRemoverEffect(); }


   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   std::string GetType() override { return "dcremover"; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;

   BiquadFilter mBiquad[ChannelBuffer::kMaxNumChannels]{};
};
