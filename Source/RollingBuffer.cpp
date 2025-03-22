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
//  RollingBuffer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#include "RollingBuffer.h"
#include "SynthGlobals.h"
#include "UserPrefs.h"

RollingBuffer::RollingBuffer(int sizeInSamples)
: mBuffer(sizeInSamples)
{
}

RollingBuffer::~RollingBuffer()
{
}

float RollingBuffer::GetSample(int samplesAgo, int channel)
{
   assert(samplesAgo >= 0);
   assert(samplesAgo < Size());
   return mBuffer.GetChannel(channel)[(Size() + mOffsetToNow[channel] - samplesAgo) % Size()];
}

void RollingBuffer::ReadChunk(float* dst, int size, int samplesAgo, int channel)
{
   assert(size <= Size());

   int offset = mOffsetToNow[channel] - samplesAgo;
   if (offset < 0)
      offset += Size();

   int wrapSamples = size - offset;
   if (wrapSamples <= 0) //no wraparound
   {
      BufferCopy(dst, mBuffer.GetChannel(channel) + (offset - size), size);
   }
   else //wrap around loop point
   {
      BufferCopy(dst, mBuffer.GetChannel(channel) + (Size() - wrapSamples), wrapSamples);
      BufferCopy(dst + wrapSamples, mBuffer.GetChannel(channel), (size - wrapSamples));
   }
}

void RollingBuffer::Accum(int samplesAgo, float sample, int channel)
{
   assert(samplesAgo < Size());
   mBuffer.GetChannel(channel)[(Size() + mOffsetToNow[channel] - samplesAgo) % Size()] += sample;
}

void RollingBuffer::WriteChunk(float* samples, int size, int channel)
{
   assert(size < Size());

   int wrapSamples = (mOffsetToNow[channel] + size) - Size();
   if (wrapSamples <= 0) //no wraparound
   {
      BufferCopy(mBuffer.GetChannel(channel) + mOffsetToNow[channel], samples, size);
   }
   else //wrap around loop point
   {
      BufferCopy(mBuffer.GetChannel(channel) + mOffsetToNow[channel], samples, (size - wrapSamples));
      BufferCopy(mBuffer.GetChannel(channel), samples + (size - wrapSamples), wrapSamples);
   }

   mOffsetToNow[channel] = (mOffsetToNow[channel] + size) % Size();
   if (channel != 0 && mOffsetToNow[channel] < mOffsetToNow[0] - gBufferSize * 2) //channels out of sync, probably was only writing to channel 0 for a while
      mOffsetToNow[channel] = mOffsetToNow[0];
}

void RollingBuffer::Write(float sample, int channel)
{
   mBuffer.GetChannel(channel)[mOffsetToNow[channel]] = sample;
   mOffsetToNow[channel] = (mOffsetToNow[channel] + 1) % Size();
   if (channel != 0 && mOffsetToNow[channel] < mOffsetToNow[0] - gBufferSize * 2) //channels out of sync, probably was only writing to channel 0 for a while
      mOffsetToNow[channel] = mOffsetToNow[0];
}

void RollingBuffer::ClearBuffer()
{
   mBuffer.Clear();
   for (int i = 0; i < ChannelBuffer::kMaxNumChannels; ++i)
      mOffsetToNow[i] = 0;
}

void RollingBuffer::Draw(int x, int y, int width, int height, int length /*= -1*/, int channel /*= -1*/, int delayOffset /*= 0*/)
{
   ofPushStyle();
   ofPushMatrix();

   ofTranslate(x, y);

   if (length == -1) //draw full rolling buffer
   {
      if (channel == -1)
         DrawAudioBuffer(width, height, &mBuffer, 0, Size(), -1);
      else
         DrawAudioBuffer(width, height, mBuffer.GetChannel(channel), 0, Size(), -1);
   }
   else //draw segment
   {
      int startSample;
      if (channel == -1)
         startSample = mOffsetToNow[0] - length - delayOffset;
      else
         startSample = mOffsetToNow[channel] - length - delayOffset;
      while (startSample < 0)
         startSample += Size();
      int endSample = startSample + length;

      while (endSample >= Size())
         endSample -= Size(); //draw wraparound

      if (channel == -1)
         DrawAudioBuffer(width, height, &mBuffer, startSample, endSample, -1, 1, ofColor::black, Size());
      else
         DrawAudioBuffer(width, height, mBuffer.GetChannel(channel), startSample, endSample, -1, 1, ofColor::black, Size());
   }

   ofPopMatrix();
   ofPopStyle();
}

namespace
{
   const int kSaveStateRev = 4;
}

void RollingBuffer::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << mBuffer.NumActiveChannels();
   out << Size();
   out << gSampleRate;
   for (int i = 0; i < mBuffer.NumActiveChannels(); ++i)
   {
      out << mOffsetToNow[i];
      out.Write(mBuffer.GetChannel(i), Size());
   }
}

void RollingBuffer::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;

   int channels = ChannelBuffer::kMaxNumChannels;
   if (rev >= 2)
      in >> channels;
   mBuffer.SetNumActiveChannels(channels);

   int savedSize = Size();
   if (rev >= 3)
      in >> savedSize;
   if (rev < 3 && UserPrefs.oversampling.Get() > 1)
      savedSize = savedSize / UserPrefs.oversampling.Get();

   int savedSampleRate = gSampleRate;
   if (rev >= 4)
      in >> savedSampleRate;

   for (int i = 0; i < channels; ++i)
   {
      in >> mOffsetToNow[i];
      mOffsetToNow[i] %= Size();
      if (savedSampleRate == gSampleRate)
      {
         if (savedSize <= Size())
         {
            in.Read(mBuffer.GetChannel(i), savedSize);
         }
         else
         {
            //saved with a longer buffer than we have... not sure what the right solution here is, but lets just fill the buffer over and over again until we consume all of the samples
            int sizeLeft = savedSize;
            while (sizeLeft > 0)
            {
               in.Read(mBuffer.GetChannel(i), MIN(sizeLeft, Size()));
               sizeLeft -= Size();
            }
         }
      }
      else
      {
         float sampleRateRatio = (float)savedSampleRate / gSampleRate;
         mOffsetToNow[i] = int(mOffsetToNow[i] / sampleRateRatio);
         float* sampleLoader = new float[savedSize];
         in.Read(sampleLoader, savedSize);
         float* destinationBuffer = mBuffer.GetChannel(i);
         for (int j = 0; j < Size(); ++j)
         {
            float pos = j * sampleRateRatio;
            int posA = MIN(int(pos), savedSize - 1);
            int posB = MIN(posA + 1, savedSize - 1);
            float alpha = pos - posA;
            destinationBuffer[j] = sampleLoader[posA] * (1 - alpha) + sampleLoader[posB] * alpha;
         }
         delete[] sampleLoader;
      }
   }
}
