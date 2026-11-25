/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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
//  IOSAUMidiOutput.h
//  Bespoke
//
//  Collects the MIDI that an AudioUnit sends on iOS.
//  JUCE only registers for this on macOS (canProduceMidiOutput() and the output callback are inside #if JUCE_MAC), so on
//  iOS a hosted plugin's MIDI never reaches the buffer that processBlock() returns.
//

#pragma once

#include <functional>

namespace IOSAUMidiOutput
{
   struct Receiver;

   struct Deleter
   {
      void operator()(Receiver* receiver) const;
   };

   //registers for an AudioUnit's MIDI output. call before the AudioUnit is initialized (prepareToPlay())
   //returns null if it has no MIDI outputs. the AudioUnit holds a pointer to this, so delete it only after the AudioUnit is disposed
   Receiver* Create(void* audioUnit, int bufferSize);

   //hands over the channel messages received since the last call. call on the audio thread, after rendering
   void Collect(Receiver* receiver, const std::function<void(const unsigned char* data, int length, int sampleOffset)>& handler);
}
