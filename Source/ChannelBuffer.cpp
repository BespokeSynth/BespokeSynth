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

    ChannelBuffer.cpp
    Created: 10 Oct 2017 8:53:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ChannelBuffer.h"

ChannelBuffer::ChannelBuffer(int bufferSize)
{
   mNumChannels = kMaxNumChannels;
   mOwnsBuffers = true;

   Setup(bufferSize);
}

ChannelBuffer::ChannelBuffer(float* data, int bufferSize)
{
   //intended as a temporary holder for passing raw data to methods that want a ChannelBuffer

   mNumChannels = 1;
   mOwnsBuffers = false;

   mBuffers = new float*[1];
   mBuffers[0] = data;
   mBufferSize = bufferSize;
}

ChannelBuffer::~ChannelBuffer()
{
   if (mOwnsBuffers)
   {
      for (int i = 0; i < mNumChannels; ++i)
         delete[] mBuffers[i];
   }
   delete[] mBuffers;
}

void ChannelBuffer::Setup(int bufferSize)
{
   mBuffers = new float*[mNumChannels];
   mBufferSize = bufferSize;

   for (int i = 0; i < mNumChannels; ++i)
      mBuffers[i] = nullptr;

   Clear();
}

float* ChannelBuffer::GetChannel(int channel)
{
   if (channel >= mActiveChannels)
      ofLog() << "error: requesting a higher channel index than we have active";
   float* ret = mBuffers[MIN(channel, mActiveChannels - 1)];
   if (ret == nullptr)
   {
      assert(mOwnsBuffers);
      ret = new float[BufferSize()];
      ::Clear(ret, BufferSize());
      mBuffers[MIN(channel, mActiveChannels - 1)] = ret;
   }
   return ret;
}

void ChannelBuffer::Clear() const
{
   for (int i = 0; i < mNumChannels; ++i)
   {
      if (mBuffers[i] != nullptr)
         ::Clear(mBuffers[i], BufferSize());
   }
}

void ChannelBuffer::SetMaxAllowedChannels(int channels)
{
   float** newBuffers = new float*[channels];
   for (int i = 0; i < channels; ++i)
   {
      if (i < mNumChannels)
         newBuffers[i] = mBuffers[i];
      else
         newBuffers[i] = nullptr;
   }

   for (int i = channels; i < mNumChannels; ++i)
      delete[] mBuffers[i];
   delete[] mBuffers;

   mBuffers = newBuffers;
   mNumChannels = channels;
   if (mActiveChannels > channels)
      mActiveChannels = channels;
}

void ChannelBuffer::CopyFrom(ChannelBuffer* src, int length /*= -1*/, int startOffset /*= 0*/)
{
   if (length == -1)
      length = mBufferSize;
   assert(length <= mBufferSize);
   assert(length + startOffset <= src->mBufferSize);
   mActiveChannels = src->mActiveChannels;
   for (int i = 0; i < mActiveChannels; ++i)
   {
      if (src->mBuffers[i])
      {
         if (mBuffers[i] == nullptr)
         {
            assert(mOwnsBuffers);
            mBuffers[i] = new float[mBufferSize];
            ::Clear(mBuffers[i], mBufferSize);
         }
         BufferCopy(mBuffers[i], src->mBuffers[i] + startOffset, length);
      }
      else
      {
         delete mBuffers[i];
         mBuffers[i] = nullptr;
      }
   }
}

void ChannelBuffer::SetChannelPointer(float* data, int channel, bool deleteOldData)
{
   if (deleteOldData)
      delete[] mBuffers[channel];
   mBuffers[channel] = data;
}

void ChannelBuffer::Resize(int bufferSize)
{
   assert(mOwnsBuffers);
   for (int i = 0; i < mNumChannels; ++i)
      delete[] mBuffers[i];
   delete[] mBuffers;

   Setup(bufferSize);
}

namespace
{
   const int kSaveStateRev = 1;
}

void ChannelBuffer::Save(FileStreamOut& out, int writeLength)
{
   out << kSaveStateRev;

   out << writeLength;
   out << mActiveChannels;
   for (int i = 0; i < mActiveChannels; ++i)
   {
      bool hasBuffer = mBuffers[i] != nullptr;
      out << hasBuffer;
      if (hasBuffer)
         out.Write(mBuffers[i], writeLength);
   }
}

void ChannelBuffer::Load(FileStreamIn& in, int& readLength, LoadMode loadMode)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   in >> readLength;
   if (loadMode == LoadMode::kSetBufferSize)
      Setup(readLength);
   else if (loadMode == LoadMode::kRequireExactBufferSize)
      assert(readLength == mBufferSize);
   else
      assert(readLength <= mBufferSize);
   in >> mActiveChannels;
   for (int i = 0; i < mActiveChannels; ++i)
   {
      bool hasBuffer = true;
      if (rev >= 1)
         in >> hasBuffer;

      if (hasBuffer)
         in.Read(GetChannel(i), readLength);
   }
}
