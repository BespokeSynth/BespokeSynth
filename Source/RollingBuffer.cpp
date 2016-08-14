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
   : mOffsetToStart(0)
   , mBufferSize(sizeInSamples)
{
   mBuffer = new float[sizeInSamples];
   Clear(mBuffer,sizeInSamples);
}

RollingBuffer::~RollingBuffer()
{
   delete[] mBuffer;
}

float RollingBuffer::GetSample(int samplesAgo)
{
   assert(samplesAgo >= 0);
   assert(samplesAgo < mBufferSize);
   return mBuffer[(mBufferSize + mOffsetToStart - samplesAgo) % mBufferSize];
}

void RollingBuffer::ReadChunk(float* dst, int size, int samplesAgo)
{
   assert(size <= mBufferSize);
   
   int offset = mOffsetToStart - samplesAgo;
   if (offset < 0)
      offset += mBufferSize;
   
   int wrapSamples = size - offset;
   if (wrapSamples <= 0) //no wraparound
   {
      memcpy(dst, mBuffer+(offset-size), size*sizeof(float));
   }
   else  //wrap around loop point
   {
      memcpy(dst, mBuffer+(mBufferSize-wrapSamples), wrapSamples*sizeof(float));
      memcpy(dst+wrapSamples, mBuffer, (size-wrapSamples)*sizeof(float));
   }
}

void RollingBuffer::Accum(int samplesAgo, float sample)
{
   assert(samplesAgo < mBufferSize);
   mBuffer[(mBufferSize + mOffsetToStart - samplesAgo) % mBufferSize] += sample;
}

void RollingBuffer::WriteChunk(float* samples, int size)
{
   assert(size < mBufferSize);
   
   int wrapSamples = (mOffsetToStart + size) - mBufferSize;
   if (wrapSamples <= 0) //no wraparound
   {
      memcpy(mBuffer+mOffsetToStart, samples, size*sizeof(float));
   }
   else  //wrap around loop point
   {
      memcpy(mBuffer+mOffsetToStart, samples, (size-wrapSamples)*sizeof(float));
      memcpy(mBuffer, samples+(size-wrapSamples), wrapSamples*sizeof(float));
   }
   
   mOffsetToStart = (mOffsetToStart + size) % mBufferSize;
}

void RollingBuffer::Write(float sample)
{
   mBuffer[mOffsetToStart] = sample;
   mOffsetToStart = (mOffsetToStart + 1) % mBufferSize;
}

void RollingBuffer::ClearBuffer()
{
   Clear(mBuffer, mBufferSize);
}

void RollingBuffer::Draw(int x, int y, int width, int height, int samples /*= -1*/)
{
   ofPushStyle();
   ofPushMatrix();

   ofTranslate(x, y);

   if (samples == -1)
   {
      DrawAudioBuffer(width, height, mBuffer, 0, mBufferSize, mOffsetToStart);
   }
   if (samples != -1)
   {
      int start = mOffsetToStart - samples;
      if (start < 0)
      {
         int endSamples = -start;
         int w1 = width * endSamples/samples;
         if (w1>0)
            DrawAudioBuffer(w1, height, mBuffer, mBufferSize-endSamples, mBufferSize-1, -1);
         ofTranslate(w1,0);
         DrawAudioBuffer(width-w1, height, mBuffer, 0, mOffsetToStart, mOffsetToStart);
      }
      else
      {
         DrawAudioBuffer(width, height, mBuffer, start, start+samples, mOffsetToStart);
      }
   }
   
   ofPopMatrix();
   ofPopStyle();
}

namespace
{
   const int kSaveStateRev = 0;
}

void RollingBuffer::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << mOffsetToStart;
   out << mBufferSize;
   out.Write(mBuffer, mBufferSize);
}

void RollingBuffer::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> mOffsetToStart;
   in >> mBufferSize;
   in.Read(mBuffer, mBufferSize);
}