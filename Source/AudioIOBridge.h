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

#pragma once

#include <cstdint>
#include <memory>

class ModularSynth;

// Adapts JUCE device clocks to Bespoke's fixed engine rate and block size.
// Construct and destroy only while the device callback is detached.
class AudioIOBridge
{
public:
   AudioIOBridge(ModularSynth& synth, int engineRate, int engineBlockSize, int deviceRate, int deviceBlockSize,
                 int inputChannels, int outputChannels, int separateInputRate = 0, int separateInputBlockSize = 0);
   ~AudioIOBridge();

   void Process(const float* const* input, int inputChannels, float* const* output, int outputChannels, int frames);
   void PushSeparateInput(const float* const* input, int inputChannels, int frames);
   uint64_t GetInputUnderrunFrames() const;

private:
   class Impl;
   std::unique_ptr<Impl> mImpl;
};
