/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
**/

#pragma once

#include "samplerate.h"

#include <vector>

// A pull adapter for libsamplerate. All buffers and converter state are made
// before audio starts; Process only consumes the preallocated buffers.
class StreamingAudioResampler
{
public:
   using ReadFrames = void (*)(void* context, float* interleaved, int frames);

   StreamingAudioResampler(int channels, ReadFrames readFrames, void* context);
   ~StreamingAudioResampler();

   StreamingAudioResampler(const StreamingAudioResampler&) = delete;
   StreamingAudioResampler& operator=(const StreamingAudioResampler&) = delete;

   bool IsReady() const { return mState != nullptr; }
   bool Process(float* const* output, int frames, double outputFramesPerInputFrame);

   static constexpr int kInputChunk = 256;
   static constexpr int kOutputChunk = 512;

private:
   SRC_STATE* mState{ nullptr };
   const int mChannels;
   const ReadFrames mReadFrames;
   void* const mContext;
   std::vector<float> mInput;
   std::vector<float> mOutput;
   int mInputPosition{ 0 };
   int mInputFrames{ 0 };
};
