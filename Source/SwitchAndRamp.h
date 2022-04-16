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
//  SwitchAndRamp.h
//  Bespoke
//
//  Created by Ryan Challinor on 03/27/22.
//
//

#pragma once

#include "ChannelBuffer.h"

class SwitchAndRamp
{
public:
   void StartSwitch();
   float Process(int channel, float input, float rampSpeed = .005f);

private:
   std::array<bool, ChannelBuffer::kMaxNumChannels> mSwitching{ false };
   std::array<float, ChannelBuffer::kMaxNumChannels> mLastOutput;
   std::array<float, ChannelBuffer::kMaxNumChannels> mOffset;
};
