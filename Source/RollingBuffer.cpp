//
//  RollingBuffer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#include "RollingBuffer.h"
#include "SynthGlobals.h"

RollingBuffer::RollingBuffer(int sizeInSamples)
: mBuffer(sizeInSamples)
{
   for (int i=0; i<ChannelBuffer::kMaxNumChannels; ++i)
      mOffsetToStart[i] = 0;
}

RollingBuffer::~RollingBuffer()
{
}

float RollingBuffer::GetSample(int samplesAgo, int channel)
{
   assert(samplesAgo >= 0);
   assert(samplesAgo < Size());
   return mBuffer.GetChannel(channel)[(Size() + mOffsetToStart[channel] - samplesAgo) % Size()];
}

void RollingBuffer::ReadChunk(float* dst, int size, int samplesAgo, int channel)
{
   assert(size <= Size());
   
   int offset = mOffsetToStart[channel] - samplesAgo;
   if (offset < 0)
      offset += Size();
   
   int wrapSamples = size - offset;
   if (wrapSamples <= 0) //no wraparound
   {
      BufferCopy(dst, mBuffer.GetChannel(channel)+(offset-size), size);
   }
   else  //wrap around loop point
   {
      BufferCopy(dst, mBuffer.GetChannel(channel)+(Size()-wrapSamples), wrapSamples);
      BufferCopy(dst+wrapSamples, mBuffer.GetChannel(channel), (size-wrapSamples));
   }
}

void RollingBuffer::Accum(int samplesAgo, float sample, int channel)
{
   assert(samplesAgo < Size());
   mBuffer.GetChannel(channel)[(Size() + mOffsetToStart[channel] - samplesAgo) % Size()] += sample;
}

void RollingBuffer::WriteChunk(float* samples, int size, int channel)
{
   assert(size < Size());
   
   int wrapSamples = (mOffsetToStart[channel] + size) - Size();
   if (wrapSamples <= 0) //no wraparound
   {
      BufferCopy(mBuffer.GetChannel(channel)+mOffsetToStart[channel], samples, size);
   }
   else  //wrap around loop point
   {
      BufferCopy(mBuffer.GetChannel(channel)+mOffsetToStart[channel], samples, (size-wrapSamples));
      BufferCopy(mBuffer.GetChannel(channel), samples+(size-wrapSamples), wrapSamples);
   }
   
   mOffsetToStart[channel] = (mOffsetToStart[channel] + size) % Size();
}

void RollingBuffer::Write(float sample, int channel)
{
   mBuffer.GetChannel(channel)[mOffsetToStart[channel]] = sample;
   mOffsetToStart[channel] = (mOffsetToStart[channel] + 1) % Size();
}

void RollingBuffer::ClearBuffer()
{
   mBuffer.Clear();
}

void RollingBuffer::Draw(int x, int y, int width, int height, int samples /*= -1*/, int channel)
{
   ofPushStyle();
   ofPushMatrix();

   ofTranslate(x, y);

   if (samples == -1)
   {
      DrawAudioBuffer(width, height, mBuffer.GetChannel(channel), 0, Size(), mOffsetToStart[channel]);
   }
   if (samples != -1)
   {
      int start = mOffsetToStart[channel] - samples;
      if (start < 0)
      {
         int endSamples = -start;
         int w1 = width * endSamples/samples;
         if (w1>0)
            DrawAudioBuffer(w1, height, mBuffer.GetChannel(channel), Size()-endSamples, Size()-1, -1);
         ofTranslate(w1,0);
         DrawAudioBuffer(width-w1, height, mBuffer.GetChannel(channel), 0, mOffsetToStart[channel], mOffsetToStart[channel]);
      }
      else
      {
         DrawAudioBuffer(width, height, mBuffer.GetChannel(channel), start, start+samples, mOffsetToStart[channel]);
      }
   }
   
   ofPopMatrix();
   ofPopStyle();
}

namespace
{
   const int kSaveStateRev = 2;
}

void RollingBuffer::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << mBuffer.NumActiveChannels();
   for (int i=0; i<mBuffer.NumActiveChannels(); ++i)
   {
      out << mOffsetToStart[i];
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
   for (int i=0; i<channels; ++i)
   {
      in >> mOffsetToStart[i];
      in.Read(mBuffer.GetChannel(i), Size());
   }
}
