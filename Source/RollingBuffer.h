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

class RollingBuffer
{
public:
   RollingBuffer(int sizeInSamples);
   ~RollingBuffer();
   float GetSample(int samplesAgo);
   void ReadChunk(float* dst, int size, int samplesAgo = 0);
   void WriteChunk(float* samples, int size);
   void Write(float sample);
   void ClearBuffer();
   void Draw(int x, int y, int width, int height, int samples = -1);
   int Size() { return mBufferSize; }
   float* GetRawBuffer() { return mBuffer; }
   int GetRawBufferOffset() { return mOffsetToStart; }
   void Accum(int samplesAgo, float sample);
   
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);
private:
   int mOffsetToStart;
   int mBufferSize;
   float* mBuffer;
};

#endif /* defined(__modularSynth__RollingBuffer__) */
