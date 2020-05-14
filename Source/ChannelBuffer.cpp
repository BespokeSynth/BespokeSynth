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
   mRecentActiveChannels = 1;
   mOwnsBuffers = true;
   
   Setup(bufferSize);
}

ChannelBuffer::ChannelBuffer(float* data, int bufferSize)
{
   //intended as a temporary holder for passing raw data to methods that want a ChannelBuffer
   
   mActiveChannels = 1;
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
      for (int i=0; i<mNumChannels; ++i)
         delete[] mBuffers[i];
   }
   delete[] mBuffers;
}

void ChannelBuffer::Setup(int bufferSize)
{
   mBuffers = new float*[mNumChannels];
   mBufferSize = bufferSize;
   
   for (int i=0; i<mNumChannels; ++i)
      mBuffers[i] = nullptr;
   
   Clear();
}

float* ChannelBuffer::GetChannel(int channel)
{
   if (channel >= mActiveChannels)
      ofLog() << "error: requesting a higher channel index than we have active";
   float* ret = mBuffers[MIN(channel, mActiveChannels-1)];
   if (ret == nullptr)
   {
      assert(mOwnsBuffers);
      ret = new float[BufferSize()];
      ::Clear(ret, BufferSize());
      mBuffers[MIN(channel, mActiveChannels-1)] = ret;
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

void ChannelBuffer::CopyFrom(ChannelBuffer* src, int length /*= -1*/)
{
   if (length == -1)
      length = mBufferSize;
   assert(length <= mBufferSize);
   assert(length <= src->mBufferSize);
   mActiveChannels = src->mActiveChannels;
   for (int i=0; i<mActiveChannels; ++i)
   {
      if (src->mBuffers[i])
      {
         if (mBuffers[i] == nullptr)
         {
            assert(mOwnsBuffers);
            mBuffers[i] = new float[mBufferSize];
            ::Clear(mBuffers[i], mBufferSize);
         }
         BufferCopy(mBuffers[i], src->mBuffers[i], length);
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
   for (int i=0; i<mNumChannels; ++i)
      delete[] mBuffers[i];
   delete[] mBuffers;
   
   Setup(bufferSize);
}

namespace
{
   const int kSaveStateRev = 0;
}

void ChannelBuffer::Save(FileStreamOut& out, int writeLength)
{
   out << kSaveStateRev;
   
   out << writeLength;
   out << mActiveChannels;
   for (int i=0; i<mActiveChannels; ++i)
      out.Write(mBuffers[i], writeLength);
}

void ChannelBuffer::Load(FileStreamIn& in, int& readLength, bool setBufferSize)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> readLength;
   if (setBufferSize)
      Setup(readLength);
   else
      assert(readLength == mBufferSize);
   in >> mActiveChannels;
   for (int i=0; i<mActiveChannels; ++i)
      in.Read(GetChannel(i), readLength);
}
