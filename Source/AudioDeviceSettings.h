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

#include <string>

namespace juce
{
   class Component;
}

struct AudioDeviceSelection
{
   std::string type;
   std::string input;
   std::string output;

   bool operator==(const AudioDeviceSelection& other) const
   {
      return type == other.type && input == other.input && output == other.output;
   }

   bool operator!=(const AudioDeviceSelection& other) const { return !(*this == other); }
};

struct AudioDeviceApplyResult
{
   bool success{ false };
   std::string error;
};

struct AudioEngineSettings
{
   int sampleRate{ 0 };
   int bufferSize{ 0 };
   int oversampling{ 0 };
   int maxInputChannels{ 0 };
   int maxOutputChannels{ 0 };

   bool operator==(const AudioEngineSettings& other) const
   {
      return sampleRate == other.sampleRate && bufferSize == other.bufferSize && oversampling == other.oversampling &&
             maxInputChannels == other.maxInputChannels && maxOutputChannels == other.maxOutputChannels;
   }

   bool operator!=(const AudioEngineSettings& other) const { return !(*this == other); }
};

struct AudioHardwareSettings
{
   int outputSampleRate{ 0 };
   int outputBufferSize{ 0 };
   int inputSampleRate{ 0 };
   int inputBufferSize{ 0 };
};

AudioDeviceSelection GetActiveAudioDeviceSelection(juce::Component* component);
AudioEngineSettings GetActiveAudioEngineSettings(juce::Component* component);
AudioHardwareSettings GetActiveAudioHardwareSettings(juce::Component* component);
AudioDeviceApplyResult ApplyAudioDeviceSelection(juce::Component* component, const AudioDeviceSelection& selection);
