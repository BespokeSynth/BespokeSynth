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

#include "AudioIOBridge.h"
#include "ModularSynth.h"
#include "StreamingAudioResampler.h"
#include "juce_audio_basics/juce_audio_basics.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace
{
   // The pinned medium quality sinc filter has a roughly 46 frame half width at
   // ratio 1. Round up and include one source read chunk for callback boundaries.
   constexpr int kSincHalfWidth = 48;
   constexpr double kMaximumClockDrift = 0.002;

   int InputFramesPerEngineBlock(int inputRate, int engineRate, int engineBlockSize)
   {
      return (int)std::ceil((double)engineBlockSize * inputRate / engineRate);
   }

   int InputResamplerLookahead(int inputRate, int engineRate)
   {
      return StreamingAudioResampler::kInputChunk +
             (int)std::ceil(kSincHalfWidth * std::max(1.0, (double)inputRate / engineRate));
   }

   int SeparateInputTarget(int inputRate, int engineRate, int engineBlockSize, int outputRate, int outputBlockSize,
                           int inputBlockSize)
   {
      const int lookahead = InputResamplerLookahead(inputRate, engineRate);
      const int callbackInputFrames = (int)std::ceil((double)outputBlockSize * inputRate / outputRate);
      return std::max({ inputBlockSize * 3, InputFramesPerEngineBlock(inputRate, engineRate, engineBlockSize) + lookahead,
                        callbackInputFrames + lookahead });
   }
}

class AudioIOBridge::Impl
{
public:
   Impl(ModularSynth& synth, int engineRate, int engineBlockSize, int deviceRate, int deviceBlockSize,
        int inputChannels, int outputChannels, int separateInputRate, int separateInputBlockSize)
   : mSynth(synth)
   , mEngineRate(engineRate)
   , mDeviceRate(deviceRate)
   , mInputRate(separateInputRate > 0 ? separateInputRate : deviceRate)
   , mSeparateInput(separateInputRate > 0)
   , mEngineBlockSize(engineBlockSize)
   , mInputChannels(inputChannels)
   , mOutputChannels(outputChannels)
   , mMaxChunk(std::max(1, std::min(8192, std::max(2048, deviceBlockSize * 4))))
   , mInputRing(std::max(1, inputChannels), std::max(mMaxChunk * 8 + engineBlockSize * 8,
                                                     mMaxChunk * 2 + 2 * (InputFramesPerEngineBlock(mInputRate, mEngineRate, engineBlockSize) +
                                                                          InputResamplerLookahead(mInputRate, mEngineRate))))
   , mSeparateInputRing(std::max(1, inputChannels),
                        mSeparateInput ? std::max({ 8192, separateInputBlockSize * 16,
                                                    2 * (SeparateInputTarget(mInputRate, mEngineRate, engineBlockSize,
                                                                             mDeviceRate, deviceBlockSize, separateInputBlockSize) +
                                                         separateInputBlockSize * 3) })
                                       : 1)
   , mSeparateInputTarget(mSeparateInput ? SeparateInputTarget(mInputRate, mEngineRate, engineBlockSize,
                                                               mDeviceRate, deviceBlockSize, separateInputBlockSize)
                                         : 0)
   , mEngineInput(std::max(1, inputChannels), engineBlockSize)
   , mEngineOutput(std::max(1, outputChannels), engineBlockSize)
   , mDeviceOutput(std::max(1, outputChannels), mMaxChunk)
   , mInputScratch(std::max(1, inputChannels), StreamingAudioResampler::kInputChunk)
   , mOutputScratch(std::max(1, outputChannels), StreamingAudioResampler::kInputChunk)
   {
      mInputRing.clear();
      mSeparateInputRing.clear();
      // A device callback can be much shorter than an engine block. Prime the
      // synchronous input FIFO for the first full engine render, including the
      // resampler's input lookahead.
      if (mInputChannels > 0 && !mSeparateInput)
      {
         const int engineInputFrames = InputFramesPerEngineBlock(mInputRate, mEngineRate, mEngineBlockSize);
         const int lookahead = mInputRate == mEngineRate ? 0 : InputResamplerLookahead(mInputRate, mEngineRate);
         // When both converters are active, output conversion can request the
         // following engine block before another hardware input callback runs.
         const int renderAheadBlocks = mInputRate == mEngineRate ? 1 : 2;
         mInputAvailable = std::min(mInputRing.getNumSamples(),
                                    std::max(deviceBlockSize, renderAheadBlocks * engineInputFrames + lookahead));
         mInputWrite = mInputAvailable % mInputRing.getNumSamples();
      }
      if (mInputChannels > 0 && (mSeparateInput || mInputRate != mEngineRate))
         mInputResampler = std::make_unique<StreamingAudioResampler>(mInputChannels, &Impl::ReadInputFrames, this);
      if (mOutputChannels > 0 && mEngineRate != mDeviceRate)
         mOutputResampler = std::make_unique<StreamingAudioResampler>(mOutputChannels, &Impl::ReadOutputFrames, this);
   }

   void Process(const float* const* input, int inputChannels, float* const* output, int outputChannels, int frames)
   {
      for (int offset = 0; offset < frames;)
      {
         int chunk = std::min(mMaxChunk, frames - offset);
         if (mSeparateInput)
         {
            // Keep the asynchronous input FIFO near its target occupancy. Two
            // physical devices have independent clocks even at equal nominal rates.
            const auto fill = mSeparateInputWrite.load(std::memory_order_acquire) - mSeparateInputRead.load(std::memory_order_acquire);
            const double targetCorrection = std::clamp(((double)fill - mSeparateInputTarget) / (mInputRate * 2.0),
                                                       -kMaximumClockDrift, kMaximumClockDrift);
            mDriftCorrection += (targetCorrection - mDriftCorrection) * 0.02;
         }
         else
            PushInput(input, inputChannels, offset, chunk);

         if (mOutputChannels > 0)
         {
            if (mOutputResampler)
            {
               if (!mOutputResampler->Process(mDeviceOutput.getArrayOfWritePointers(), chunk,
                                              (double)mDeviceRate / mEngineRate))
                  mDeviceOutput.clear(0, chunk);
            }
            else
               PopEngineOutput(juce::AudioSourceChannelInfo(&mDeviceOutput, 0, chunk));
            for (int ch = 0; ch < outputChannels; ++ch)
            {
               if (ch < mOutputChannels && output != nullptr && output[ch] != nullptr)
                  juce::FloatVectorOperations::copy(output[ch] + offset, mDeviceOutput.getReadPointer(ch), chunk);
               else if (output != nullptr && output[ch] != nullptr)
                  juce::FloatVectorOperations::clear(output[ch] + offset, chunk);
            }
         }
         else
         {
            for (int ch = 0; ch < outputChannels; ++ch)
               if (output != nullptr && output[ch] != nullptr)
                  juce::FloatVectorOperations::clear(output[ch] + offset, chunk);

            // Input-only devices still advance the synth's transport and input modules.
            mInputOnlyFrames += (double)chunk * mEngineRate / mDeviceRate;
            while (mInputOnlyFrames >= mEngineBlockSize)
            {
               RenderEngineBlock();
               mInputOnlyFrames -= mEngineBlockSize;
            }
         }
         offset += chunk;
      }
   }

   void PushSeparateInput(const float* const* input, int channels, int frames)
   {
      if (!mSeparateInput)
         return;
      const auto capacity = (uint64_t)mSeparateInputRing.getNumSamples();
      auto written = mSeparateInputWrite.load(std::memory_order_relaxed);
      const auto read = mSeparateInputRead.load(std::memory_order_acquire);
      const int amount = (int)std::min<uint64_t>(frames, capacity - std::min(capacity, written - read));
      for (int frame = 0; frame < amount; ++frame)
      {
         int position = (int)((written + frame) % capacity);
         for (int ch = 0; ch < mInputChannels; ++ch)
            mSeparateInputRing.setSample(ch, position, input != nullptr && ch < channels && input[ch] != nullptr ? input[ch][frame] : 0.0f);
      }
      mSeparateInputWrite.store(written + amount, std::memory_order_release);
   }

   uint64_t GetInputUnderrunFrames() const { return mInputUnderrunFrames.load(std::memory_order_relaxed); }

private:
   static void ReadInputFrames(void* context, float* interleaved, int frames)
   {
      auto& owner = *static_cast<Impl*>(context);
      owner.PopInput(juce::AudioSourceChannelInfo(&owner.mInputScratch, 0, frames));
      for (int frame = 0; frame < frames; ++frame)
         for (int ch = 0; ch < owner.mInputChannels; ++ch)
            interleaved[frame * owner.mInputChannels + ch] = owner.mInputScratch.getSample(ch, frame);
   }

   static void ReadOutputFrames(void* context, float* interleaved, int frames)
   {
      auto& owner = *static_cast<Impl*>(context);
      owner.PopEngineOutput(juce::AudioSourceChannelInfo(&owner.mOutputScratch, 0, frames));
      for (int frame = 0; frame < frames; ++frame)
         for (int ch = 0; ch < owner.mOutputChannels; ++ch)
            interleaved[frame * owner.mOutputChannels + ch] = owner.mOutputScratch.getSample(ch, frame);
   }

   void PushInput(const float* const* input, int channels, int offset, int frames)
   {
      if (mInputChannels == 0)
         return;
      const int capacity = mInputRing.getNumSamples();
      if (frames > capacity - mInputAvailable)
      {
         const int dropped = frames - (capacity - mInputAvailable);
         mInputRead = (mInputRead + dropped) % capacity;
         mInputAvailable -= dropped;
      }
      for (int frame = 0; frame < frames; ++frame)
      {
         int position = (mInputWrite + frame) % capacity;
         for (int ch = 0; ch < mInputChannels; ++ch)
            mInputRing.setSample(ch, position, input != nullptr && ch < channels && input[ch] != nullptr ? input[ch][offset + frame] : 0.0f);
      }
      mInputWrite = (mInputWrite + frames) % capacity;
      mInputAvailable += frames;
   }

   void PopInput(const juce::AudioSourceChannelInfo& info)
   {
      if (mSeparateInput)
      {
         PopSeparateInput(info);
         return;
      }
      const int capacity = mInputRing.getNumSamples();
      int underrunFrames = 0;
      for (int frame = 0; frame < info.numSamples; ++frame)
      {
         bool available = mInputAvailable > 0;
         underrunFrames += !available;
         for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
            info.buffer->setSample(ch, info.startSample + frame,
                                   available && ch < mInputChannels ? mInputRing.getSample(ch, mInputRead) : 0.0f);
         if (available)
         {
            mInputRead = (mInputRead + 1) % capacity;
            --mInputAvailable;
         }
      }
      mInputUnderrunFrames.fetch_add(underrunFrames, std::memory_order_relaxed);
   }

   void PopSeparateInput(const juce::AudioSourceChannelInfo& info)
   {
      const auto written = mSeparateInputWrite.load(std::memory_order_acquire);
      auto read = mSeparateInputRead.load(std::memory_order_relaxed);
      if (!mSeparateInputReady && written - read >= (uint64_t)mSeparateInputTarget)
         mSeparateInputReady = true;
      const auto capacity = (uint64_t)mSeparateInputRing.getNumSamples();
      int underrunFrames = 0;
      for (int frame = 0; frame < info.numSamples; ++frame)
      {
         bool available = mSeparateInputReady && read < written;
         underrunFrames += mSeparateInputReady && !available;
         for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
            info.buffer->setSample(ch, info.startSample + frame,
                                   available && ch < mInputChannels ? mSeparateInputRing.getSample(ch, (int)(read % capacity)) : 0.0f);
         if (available)
            ++read;
      }
      mSeparateInputRead.store(read, std::memory_order_release);
      mInputUnderrunFrames.fetch_add(underrunFrames, std::memory_order_relaxed);
   }

   void RenderEngineBlock()
   {
      if (mInputResampler)
      {
         const double ratio = (double)mEngineRate / (mInputRate * (1.0 + mDriftCorrection));
         if (!mInputResampler->Process(mEngineInput.getArrayOfWritePointers(), mEngineBlockSize, ratio))
            mEngineInput.clear();
         mSynth.AudioIn(mEngineInput.getArrayOfReadPointers(), mEngineBlockSize, mInputChannels);
      }
      else if (mInputChannels > 0)
      {
         PopInput(juce::AudioSourceChannelInfo(&mEngineInput, 0, mEngineBlockSize));
         mSynth.AudioIn(mEngineInput.getArrayOfReadPointers(), mEngineBlockSize, mInputChannels);
      }
      else
         mSynth.AudioIn(nullptr, mEngineBlockSize, 0);

      mSynth.AudioOut(mEngineOutput.getArrayOfWritePointers(), mEngineBlockSize, mOutputChannels);
      mEngineOutputPosition = 0;
   }

   void PopEngineOutput(const juce::AudioSourceChannelInfo& info)
   {
      int copied = 0;
      while (copied < info.numSamples)
      {
         if (mEngineOutputPosition == mEngineBlockSize)
            RenderEngineBlock();
         const int amount = std::min(info.numSamples - copied, mEngineBlockSize - mEngineOutputPosition);
         for (int ch = 0; ch < mOutputChannels; ++ch)
            juce::FloatVectorOperations::copy(info.buffer->getWritePointer(ch, info.startSample + copied),
                                              mEngineOutput.getReadPointer(ch, mEngineOutputPosition), amount);
         copied += amount;
         mEngineOutputPosition += amount;
      }
   }

   ModularSynth& mSynth;
   const int mEngineRate;
   const int mDeviceRate;
   const int mInputRate;
   const bool mSeparateInput;
   const int mEngineBlockSize;
   const int mInputChannels;
   const int mOutputChannels;
   const int mMaxChunk;
   juce::AudioBuffer<float> mInputRing;
   juce::AudioBuffer<float> mSeparateInputRing;
   const int mSeparateInputTarget;
   juce::AudioBuffer<float> mEngineInput;
   juce::AudioBuffer<float> mEngineOutput;
   juce::AudioBuffer<float> mDeviceOutput;
   juce::AudioBuffer<float> mInputScratch;
   juce::AudioBuffer<float> mOutputScratch;
   int mInputRead{ 0 };
   int mInputWrite{ 0 };
   int mInputAvailable{ 0 };
   std::atomic<uint64_t> mSeparateInputRead{ 0 };
   std::atomic<uint64_t> mSeparateInputWrite{ 0 };
   std::atomic<uint64_t> mInputUnderrunFrames{ 0 };
   bool mSeparateInputReady{ false };
   double mDriftCorrection{ 0 };
   int mEngineOutputPosition{ mEngineBlockSize };
   double mInputOnlyFrames{ 0 };
   std::unique_ptr<StreamingAudioResampler> mInputResampler;
   std::unique_ptr<StreamingAudioResampler> mOutputResampler;
};

AudioIOBridge::AudioIOBridge(ModularSynth& synth, int engineRate, int engineBlockSize, int deviceRate, int deviceBlockSize,
                             int inputChannels, int outputChannels, int separateInputRate, int separateInputBlockSize)
: mImpl(std::make_unique<Impl>(synth, engineRate, engineBlockSize, deviceRate, deviceBlockSize,
                               inputChannels, outputChannels, separateInputRate, separateInputBlockSize))
{
}

AudioIOBridge::~AudioIOBridge() = default;

void AudioIOBridge::Process(const float* const* input, int inputChannels, float* const* output, int outputChannels, int frames)
{
   mImpl->Process(input, inputChannels, output, outputChannels, frames);
}

void AudioIOBridge::PushSeparateInput(const float* const* input, int inputChannels, int frames)
{
   mImpl->PushSeparateInput(input, inputChannels, frames);
}

uint64_t AudioIOBridge::GetInputUnderrunFrames() const
{
   return mImpl->GetInputUnderrunFrames();
}
