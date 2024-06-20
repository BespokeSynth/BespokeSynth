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
//  IAudioEffect.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/21/12.
//
//

#pragma once

#include "IDrawableModule.h"
#include "ChannelBuffer.h"

class IAudioEffect : public IDrawableModule
{
public:
   virtual ~IAudioEffect() {}
   virtual void ProcessAudio(double time, ChannelBuffer* buffer) = 0;
   void SetEnabled(bool enabled) override = 0;
   virtual float GetEffectAmount() { return 0; }
   virtual std::string GetType() = 0;
   bool CanMinimize() override { return false; }
   bool IsSaveable() override { return false; }
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override {}
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override {}
};
