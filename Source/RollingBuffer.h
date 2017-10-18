//
//  RollingBuffer.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#ifndef __modularSynth__RollingBuffer__
#define __modularSynth__RollingBuffer__

#include <iostream>
#include "FileStream.h"
#include "ChannelBuffer.h"

class RollingBuffer
{
public:
   RollingBuffer(int sizeInSamples);
   ~RollingBuffer();
   float GetSample(int samplesAgo, int channel = 0);
   void ReadChunk(float* dst, int size, int samplesAgo = 0, int channel = 0);
   void WriteChunk(float* samples, int size, int channel = 0);
   void Write(float sample, int channel = 0);
   void ClearBuffer();
   void Draw(int x, int y, int width, int height, int samples = -1, int channel = 0);
   int Size() { return mBuffer.BufferSize(); }
   float* GetRawBuffer(int channel = 0) { return mBuffer.GetChannel(channel); }
   int GetRawBufferOffset(int channel = 0) { return mOffsetToStart[channel]; }
   void Accum(int samplesAgo, float sample, int channel = 0);
   void SetNumChannels(int channels) { mBuffer.SetNumActiveChannels(channels); }
   int NumChannels() const { return mBuffer.NumActiveChannels(); }
   
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);
private:
   int mOffsetToStart[ChannelBuffer::kMaxNumChannels];
   ChannelBuffer mBuffer;
};

#endif /* defined(__modularSynth__RollingBuffer__) */
