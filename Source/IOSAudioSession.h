/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2022 Ryan Challinor (contact: awwbees@gmail.com)

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
//  IOSAudioSession.h
//  Bespoke
//
//  JUCE requests a buffer duration of (bufferSize + 1) / sampleRate, which modern iOS devices honour literally
//  (e.g. 257-sample callbacks for a 256 request). This requests the exact duration instead. iOS may still choose
//  a different size, which FixedBlockAudioAdapter handles.
//

#pragma once

namespace IOSAudioSession
{
   void RequestExactBufferSize(double sampleRate, int bufferSize);
}
