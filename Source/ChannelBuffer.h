/*
  ==============================================================================

    ChannelBuffer.h
    Created: 10 Oct 2017 8:53:51pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "SynthGlobals.h"

class ChannelBuffer
{
public:
   ChannelBuffer(int bufferSize);
   ChannelBuffer(float* data, int bufferSize);  //intended as a temporary holder for passing raw data to methods that want a ChannelBuffer
   ~ChannelBuffer();
   
   float* GetChannel(int channel);
   
   void Clear() const;
   
   void SetMaxAllowedChannels(int channels);
   void SetNumActiveChannels(int channels) { mActiveChannels = MIN(mNumChannels, channels); }
   int NumActiveChannels() const { return mActiveChannels; }
   int NumTotalChannels() const { return mNumChannels; }
   int BufferSize() const { return mBufferSize; }
   void CopyFrom(ChannelBuffer* src);
   
   static const int kMaxNumChannels = 2;
   
private:
   int mActiveChannels;
   int mNumChannels;
   int mBufferSize;
   float** mBuffers;
};
