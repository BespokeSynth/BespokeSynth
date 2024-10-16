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
//  RollingBuffer.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#pragma once

#include "FileStream.h"
#include "ChannelBuffer.h"

class RollingBuffer
{
public:
   RollingBuffer(int sizeInSamples);
   ~RollingBuffer();
   float GetSample(int samplesAgo, int channel);
   void ReadChunk(float* dst, int size, int samplesAgo, int channel);
   void WriteChunk(float* samples, int size, int channel);
   void Write(float sample, int channel);
   void ClearBuffer();
   void Draw(int x, int y, int width, int height, int length = -1, int channel = -1, int delayOffset = 0);
   int Size() { return mBuffer.BufferSize(); }
   ChannelBuffer* GetRawBuffer() { return &mBuffer; }
   int GetRawBufferOffset(int channel) { return mOffsetToNow[channel]; }
   void Accum(int samplesAgo, float sample, int channel);
   void SetNumChannels(int channels) { mBuffer.SetNumActiveChannels(channels); }
   int NumChannels() const { return mBuffer.NumActiveChannels(); }

   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

private:
   int mOffsetToNow[ChannelBuffer::kMaxNumChannels]{};
   ChannelBuffer mBuffer;
};
