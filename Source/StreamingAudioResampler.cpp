/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
**/

#include "StreamingAudioResampler.h"

#include <algorithm>
#include <cmath>

StreamingAudioResampler::StreamingAudioResampler(int channels, ReadFrames readFrames, void* context)
: mChannels(channels)
, mReadFrames(readFrames)
, mContext(context)
, mInput(kInputChunk * channels)
, mOutput(kOutputChunk * channels)
{
   if (channels > 0 && readFrames != nullptr)
   {
      int error = 0;
      mState = src_new(SRC_SINC_MEDIUM_QUALITY, channels, &error);
   }
}

StreamingAudioResampler::~StreamingAudioResampler()
{
   if (mState != nullptr)
      src_delete(mState);
}

bool StreamingAudioResampler::Process(float* const* output, int frames, double ratio)
{
   if (mState == nullptr || output == nullptr || frames < 0 || !std::isfinite(ratio) || !src_is_valid_ratio(ratio))
      return false;

   for (int generated = 0; generated < frames;)
   {
      if (mInputPosition == mInputFrames)
      {
         mReadFrames(mContext, mInput.data(), kInputChunk);
         mInputPosition = 0;
         mInputFrames = kInputChunk;
      }

      SRC_DATA data{};
      data.data_in = mInput.data() + mInputPosition * mChannels;
      data.input_frames = mInputFrames - mInputPosition;
      data.data_out = mOutput.data();
      data.output_frames = std::min(kOutputChunk, frames - generated);
      data.src_ratio = ratio;
      if (src_process(mState, &data) != 0)
         return false;

      mInputPosition += (int)data.input_frames_used;
      for (int frame = 0; frame < data.output_frames_gen; ++frame)
         for (int ch = 0; ch < mChannels; ++ch)
            output[ch][generated + frame] = mOutput[frame * mChannels + ch];
      generated += (int)data.output_frames_gen;

      // An unchanged full input buffer cannot make progress on a subsequent
      // call with the same output capacity. Return silence through the caller.
      if (data.input_frames_used == 0 && data.output_frames_gen == 0)
         return false;
   }
   return true;
}
