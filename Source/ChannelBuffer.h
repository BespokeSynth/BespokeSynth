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
/*
  ==============================================================================

    ChannelBuffer.h
    Created: 10 Oct 2017 8:53:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "SynthGlobals.h"
#include "FileStream.h"

class ChannelBuffer
{
public:
   ChannelBuffer(int bufferSize);
   ChannelBuffer(float* data, int bufferSize); //intended as a temporary holder for passing raw data to methods that want a ChannelBuffer
   ~ChannelBuffer();

   float* GetChannel(int channel);

   void Clear() const;

   void SetMaxAllowedChannels(int channels);
   void SetNumActiveChannels(int channels) { mActiveChannels = MIN(mNumChannels, channels); }
   int NumActiveChannels() const { return mActiveChannels; }
   int RecentNumActiveChannels() const { return mRecentActiveChannels; }
   int NumTotalChannels() const { return mNumChannels; }
   int BufferSize() const { return mBufferSize; }
   void CopyFrom(ChannelBuffer* src, int length = -1, int startOffset = 0);
   void SetChannelPointer(float* data, int channel, bool deleteOldData);
   void Reset()
   {
      Clear();
      mRecentActiveChannels = mActiveChannels;
      SetNumActiveChannels(1);
   }
   void Resize(int bufferSize);

   enum class LoadMode
   {
      kSetBufferSize,
      kRequireExactBufferSize,
      kAnyBufferSize
   };

   void Save(FileStreamOut& out, int writeLength);
   void Load(FileStreamIn& in, int& readLength, LoadMode loadMode);

   static const int kMaxNumChannels = 2;

private:
   void Setup(int bufferSize);

   int mActiveChannels{ 1 };
   int mNumChannels{ 1 };
   int mBufferSize{ 0 };
   float** mBuffers;
   int mRecentActiveChannels{ 1 };
   bool mOwnsBuffers{ true };
};
