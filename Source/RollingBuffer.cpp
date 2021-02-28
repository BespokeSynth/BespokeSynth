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
      mOffsetToNow[i] = 0;
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
   mBuffer.GetChannel(channel)[(Size() + mOffsetToNow[channel] - samplesAgo) % Size()] += sample;
}

void RollingBuffer::WriteChunk(float* samples, int size, int channel)
{
   assert(size < Size());
   
   int wrapSamples = (mOffsetToNow[channel] + size) - Size();
   if (wrapSamples <= 0) //no wraparound
   {
      BufferCopy(mBuffer.GetChannel(channel)+mOffsetToNow[channel], samples, size);
   }
   else  //wrap around loop point
   {
      BufferCopy(mBuffer.GetChannel(channel)+mOffsetToNow[channel], samples, (size-wrapSamples));
      BufferCopy(mBuffer.GetChannel(channel), samples+(size-wrapSamples), wrapSamples);
   }
   
   mOffsetToNow[channel] = (mOffsetToNow[channel] + size) % Size();
}

void RollingBuffer::Write(float sample, int channel)
{
   mBuffer.GetChannel(channel)[mOffsetToNow[channel]] = sample;
   mOffsetToNow[channel] = (mOffsetToNow[channel] + 1) % Size();
}

void RollingBuffer::ClearBuffer()
{
   mBuffer.Clear();
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
      if (startSample < 0)
         startSample += Size();
      int endSample = startSample + length;

      if (endSample >= Size())
      {
         endSample -= Size();
         int firstHalfSamples = Size() - startSample;
         int widthFirstHalf = width * firstHalfSamples/length;
         if (widthFirstHalf > 0)
         {
            if (channel == -1)
               DrawAudioBuffer(widthFirstHalf, height, &mBuffer, startSample, Size() - 1, -1);
            else
               DrawAudioBuffer(widthFirstHalf, height, mBuffer.GetChannel(channel), startSample, Size() - 1, -1);
         }
         ofTranslate(widthFirstHalf,0);
         if (channel == -1)
            DrawAudioBuffer(width-widthFirstHalf, height, &mBuffer, 0, endSample, -1);
         else
            DrawAudioBuffer(width-widthFirstHalf, height, mBuffer.GetChannel(channel), 0, endSample, -1);
      }
      else
      {
         if (channel == -1)
            DrawAudioBuffer(width, height, &mBuffer, startSample, endSample, -1);
         else
            DrawAudioBuffer(width, height, mBuffer.GetChannel(channel), startSample, endSample, -1);
      }
   }
   
   ofPopMatrix();
   ofPopStyle();
}

namespace
{
   const int kSaveStateRev = 3;
}

void RollingBuffer::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << mBuffer.NumActiveChannels();
   out << Size();
   for (int i=0; i<mBuffer.NumActiveChannels(); ++i)
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
   int savedSize = Size();
   if (rev >= 3)
      in >> savedSize;
   mBuffer.SetNumActiveChannels(channels);
   for (int i=0; i<channels; ++i)
   {
      in >> mOffsetToNow[i];
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
}
