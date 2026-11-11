/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2022 Ryan Challinor (contact: awwbees@gmail.com)

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
//  FixedBlockAudioAdapter.h
//  Bespoke
//
//  Bespoke processes audio in fixed-size blocks at a fixed sample rate, but some audio devices (notably on iOS)
//  don't guarantee either:
//  - the callback size can differ from what was requested, and change at any time (e.g. iOS uses large buffers
//    while the screen is locked)
//  - the device's sample rate can change mid-session (e.g. iOS switches to 44.1k when routing to a 44.1k-only
//    USB interface)
//  This adapter renders fixed-size blocks at bespoke's rate on demand, and resamples between bespoke's rate and the
//  device's rate when they differ.
//
//  Output is rendered when the device asks for it, so it adds at most a block of latency. Input is buffered with a
//  block of silence up front, so input always adds a block of latency.
//

#pragma once

#include "juce_audio_basics/juce_audio_basics.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

class FixedBlockAudioAdapter
{
public:
   //not realtime-safe, call before audio starts
   void Prepare(int numInputChannels, int numOutputChannels, int blockSize, int maxChunkSize)
   {
      mBlockSize = blockSize;
      mMaxChunkSize = maxChunkSize;

      //always keep at least one channel of each, so there's something to drive the resampling clock
      const int inChannels = std::max(numInputChannels, 1);
      const int outChannels = std::max(numOutputChannels, 1);
      const int fifoSize = (maxChunkSize + blockSize) * kMaxRatio + 16;

      mInFifo.assign(inChannels, std::vector<float>(fifoSize, 0));
      mOutFifo.assign(outChannels, std::vector<float>(fifoSize, 0));
      mBlockInput.assign(inChannels, std::vector<float>(blockSize, 0));
      mScratch.assign(maxChunkSize, 0);
      mInPtrs.assign(inChannels, nullptr);
      mOutPtrs.assign(outChannels, nullptr);
      //(interpolators aren't copyable, so they're held by pointer)
      mInInterpolators.clear();
      for (int i = 0; i < inChannels; ++i)
         mInInterpolators.push_back(std::make_unique<juce::LagrangeInterpolator>());
      mOutInterpolators.clear();
      for (int i = 0; i < outChannels; ++i)
         mOutInterpolators.push_back(std::make_unique<juce::LagrangeInterpolator>());

      SetSampleRates(1, 1);
      Reset();
   }

   bool IsPrepared() const { return mBlockSize > 0; }

   //not safe to call concurrently with Process(). call Reset() afterwards
   void SetSampleRates(double bespokeSampleRate, double deviceSampleRate)
   {
      if (bespokeSampleRate <= 0 || deviceSampleRate <= 0)
         mRatio = 1;
      else
         mRatio = std::clamp(bespokeSampleRate / deviceSampleRate, 1.0 / kMaxRatio, (double)kMaxRatio);
   }

   bool IsResampling() const { return mRatio != 1; }

   //realtime-safe. discards buffered audio and restores the initial state
   void Reset()
   {
      for (auto& interpolator : mInInterpolators)
         interpolator->reset();
      for (auto& interpolator : mOutInterpolators)
         interpolator->reset();

      mOutCount = 0;

      //prefill input with silence, so rendering a block never has to wait for device input.
      //when resampling, output can be rendered a few samples ahead and the interpolator needs some margin, so add headroom for that
      mInCount = GetDeviceSamplesNeededForBlock();
      if (IsResampling())
         mInCount += (int)std::ceil(8 / mRatio) + 4;
      for (auto& fifo : mInFifo)
         std::fill(fifo.begin(), fifo.begin() + mInCount, 0.0f);
   }

   //processBlock(const float* const* input, float* const* output, int blockSize) is called once per block, at bespoke's sample rate
   template <typename ProcessBlockFn>
   void Process(const float* const* input, int numInputChannels, float* const* output, int numOutputChannels, int numSamples, ProcessBlockFn&& processBlock)
   {
      for (int offset = 0; offset < numSamples; offset += mMaxChunkSize)
         ProcessChunk(input, numInputChannels, output, numOutputChannels, offset, std::min(mMaxChunkSize, numSamples - offset), processBlock);
   }

private:
   static constexpr int kMaxRatio = 8;

   //device-rate input samples needed to produce one block at bespoke's rate (with margin for the interpolator)
   int GetDeviceSamplesNeededForBlock() const
   {
      if (!IsResampling())
         return mBlockSize;
      return (int)std::ceil(mBlockSize / mRatio) + 2;
   }

   template <typename ProcessBlockFn>
   void ProcessChunk(const float* const* input, int numInputChannels, float* const* output, int numOutputChannels, int offset, int numSamples, ProcessBlockFn& processBlock)
   {
      //buffer the device's input
      for (size_t ch = 0; ch < mInFifo.size(); ++ch)
      {
         float* dest = mInFifo[ch].data() + mInCount;
         if ((int)ch < numInputChannels && input[ch] != nullptr)
            std::memcpy(dest, input[ch] + offset, sizeof(float) * numSamples);
         else
            std::memset(dest, 0, sizeof(float) * numSamples);
      }
      mInCount += numSamples;

      //render blocks until there's enough output to fill the device's buffer
      const int outputNeeded = IsResampling() ? (int)std::ceil(numSamples * mRatio) + 2 : numSamples;
      while (mOutCount < outputNeeded)
         RenderBlock(processBlock);

      //hand the output to the device
      int consumed = numSamples;
      for (size_t ch = 0; ch < mOutFifo.size(); ++ch)
      {
         const float* source = mOutFifo[ch].data();
         if (IsResampling())
         {
            consumed = mOutInterpolators[ch]->process(mRatio, source, mScratch.data(), numSamples);
            source = mScratch.data();
         }

         if ((int)ch < numOutputChannels && output[ch] != nullptr)
            std::memcpy(output[ch] + offset, source, sizeof(float) * numSamples);
      }
      for (int ch = (int)mOutFifo.size(); ch < numOutputChannels; ++ch)
      {
         if (output[ch] != nullptr)
            std::memset(output[ch] + offset, 0, sizeof(float) * numSamples);
      }

      mOutCount -= consumed;
      for (auto& fifo : mOutFifo)
         std::memmove(fifo.data(), fifo.data() + consumed, sizeof(float) * mOutCount);
   }

   template <typename ProcessBlockFn>
   void RenderBlock(ProcessBlockFn& processBlock)
   {
      //get one block of input at bespoke's rate
      const int inputNeeded = GetDeviceSamplesNeededForBlock();
      if (mInCount >= inputNeeded)
      {
         int consumed = mBlockSize;
         for (size_t ch = 0; ch < mInFifo.size(); ++ch)
         {
            if (IsResampling())
               consumed = mInInterpolators[ch]->process(1.0 / mRatio, mInFifo[ch].data(), mBlockInput[ch].data(), mBlockSize);
            else
               std::memcpy(mBlockInput[ch].data(), mInFifo[ch].data(), sizeof(float) * mBlockSize);
         }

         mInCount -= consumed;
         for (auto& fifo : mInFifo)
            std::memmove(fifo.data(), fifo.data() + consumed, sizeof(float) * mInCount);
      }
      else //shouldn't happen thanks to the prefill, but if input runs short, use silence rather than stalling output
      {
         for (auto& block : mBlockInput)
            std::fill(block.begin(), block.end(), 0.0f);
      }

      for (size_t ch = 0; ch < mBlockInput.size(); ++ch)
         mInPtrs[ch] = mBlockInput[ch].data();
      for (size_t ch = 0; ch < mOutFifo.size(); ++ch)
         mOutPtrs[ch] = mOutFifo[ch].data() + mOutCount;

      processBlock(mInPtrs.data(), mOutPtrs.data(), mBlockSize);
      mOutCount += mBlockSize;
   }

   int mBlockSize{ 0 };
   int mMaxChunkSize{ 0 };
   double mRatio{ 1 }; //bespoke sample rate / device sample rate
   int mInCount{ 0 }; //device-rate samples waiting in mInFifo
   int mOutCount{ 0 }; //bespoke-rate samples waiting in mOutFifo
   std::vector<std::vector<float>> mInFifo;
   std::vector<std::vector<float>> mOutFifo;
   std::vector<std::vector<float>> mBlockInput;
   std::vector<float> mScratch;
   std::vector<const float*> mInPtrs;
   std::vector<float*> mOutPtrs;
   std::vector<std::unique_ptr<juce::LagrangeInterpolator>> mInInterpolators;
   std::vector<std::unique_ptr<juce::LagrangeInterpolator>> mOutInterpolators;
};
