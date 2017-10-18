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
   mActiveChannels = 1;
   mNumChannels = kMaxNumChannels;
   
   mBuffers = new float*[mNumChannels];
   mBufferSize = bufferSize;
   
   for (int i=0; i<mNumChannels; ++i)
      mBuffers[i] = nullptr;
   
   Clear();
}

ChannelBuffer::ChannelBuffer(float* data, int bufferSize)
{
   //intended as a temporary holder for passing raw data to methods that want a ChannelBuffer
   
   mActiveChannels = 1;
   mNumChannels = 1;
   
   mBuffers = new float*[1];
   mBuffers[0] = data;
   mBufferSize = bufferSize;
}

ChannelBuffer::~ChannelBuffer()
{
   for (int i=0; i<mNumChannels; ++i)
      delete[] mBuffers[i];
   delete[] mBuffers;
}

float* ChannelBuffer::GetChannel(int channel)
{
   assert(channel < mActiveChannels);
   float* ret = mBuffers[MIN(channel, mActiveChannels-1)];
   if (ret == nullptr)
   {
      ret = mBuffers[MIN(channel, mActiveChannels-1)] = new float[BufferSize()];
      ::Clear(ret, BufferSize());
   }
   return ret;
}

void ChannelBuffer::Clear() const
{
   for (int i=0; i<mNumChannels; ++i)
   {
      if (mBuffers[i] != nullptr)
         ::Clear(mBuffers[i], BufferSize());
   }
}

void ChannelBuffer::SetMaxAllowedChannels(int channels)
{
   float** newBuffers = new float*[channels];
   for (int i=0; i<channels; ++i)
   {
      if (i < mNumChannels)
         newBuffers[i] = mBuffers[i];
      else
         newBuffers[i] = nullptr;
   }
   
   for (int i=channels; i<mNumChannels; ++i)
      delete[] mBuffers[i];
   delete[] mBuffers;
   
   mBuffers = newBuffers;
   mNumChannels = channels;
   if (mActiveChannels > channels)
      mActiveChannels = channels;
}

void ChannelBuffer::CopyFrom(ChannelBuffer* src)
{
   assert(mBufferSize == src->mBufferSize);
   for (int i=0; i<mActiveChannels; ++i)
   {
      if (src->mBuffers[i])
      {
         if (mBuffers[i] == nullptr)
            mBuffers[i] = new float[mBufferSize];
         memcpy(mBuffers[i], src->mBuffers[i], mBufferSize*sizeof(float));
      }
      else
      {
         delete mBuffers[i];
         mBuffers[i] = nullptr;
      }
   }
}
