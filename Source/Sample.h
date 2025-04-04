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
//
//  Sample.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#pragma once

#include "OpenFrameworksPort.h"
#include "ChannelBuffer.h"
#include <limits>

#include "juce_events/juce_events.h"

class FileStreamOut;
class FileStreamIn;

namespace juce
{
   class AudioFormatReader;
   template <typename T>
   class AudioBuffer;
   using AudioSampleBuffer = AudioBuffer<float>;
}

class Sample : public juce::Timer
{
public:
   enum class ReadType
   {
      Sync,
      Async
   };

   Sample();
   ~Sample();
   bool Read(const char* path, bool mono = false, ReadType readType = ReadType::Sync);
   bool Write(const char* path = nullptr); //no path = use read filename
   bool ConsumeData(double time, ChannelBuffer* out, int size, bool replace);
   void Play(double time, float rate, int offset, int stopPoint = -1);
   void SetRate(float rate) { mRate = rate; }
   std::string Name() const { return mName; }
   void SetName(std::string name) { mName = name; }
   int LengthInSamples() const { return mNumSamples; }
   int NumChannels() const { return mData.NumActiveChannels(); }
   ChannelBuffer* Data() { return &mData; }
   double GetPlayPosition() const { return mOffset; }
   void SetPlayPosition(double sample) { mOffset = sample; }
   float GetSampleRateRatio() const { return mSampleRateRatio; }
   void Reset() { mOffset = mNumSamples; }
   void SetStopPoint(int stopPoint) { mStopPoint = stopPoint; }
   void ClearStopPoint() { mStopPoint = -1; }
   void PadBack(int amount);
   void ClipTo(int start, int end);
   void ShiftWrap(int numSamples);
   std::string GetReadPath() const { return mReadPath; }
   static bool WriteDataToFile(const std::string& path, float** data, int numSamples, int channels = 1);
   static bool WriteDataToFile(const std::string& path, ChannelBuffer* data, int numSamples);
   bool IsPlaying() const { return mOffset < mNumSamples; }
   void LockDataMutex(bool lock) { lock ? mDataMutex.lock() : mDataMutex.unlock(); }
   void Create(int length);
   void Create(ChannelBuffer* data);
   void SetLooping(bool looping) { mLooping = looping; }
   void SetNumBars(int numBars) { mNumBars = numBars; }
   int GetNumBars() const { return mNumBars; }
   void SetVolume(float vol) { mVolume = vol; }
   void CopyFrom(Sample* sample);
   bool IsSampleLoading() { return mSamplesLeftToRead > 0; }
   float GetSampleLoadProgress() { return (mNumSamples > 0) ? (1 - (float(mSamplesLeftToRead) / mNumSamples)) : 1; }

   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

private:
   void Setup(int length);
   void FinishRead();
   //juce::Timer
   void timerCallback();

   ChannelBuffer mData{ 0 };
   int mNumSamples{ 0 };
   double mStartTime{ 0 };
   double mOffset{ std::numeric_limits<double>::max() };
   float mRate{ 1 };
   int mOriginalSampleRate{ gSampleRate };
   float mSampleRateRatio{ 1 };
   int mStopPoint{ -1 };
   std::string mName{ "" };
   std::string mReadPath{ "" };
   ofMutex mDataMutex;
   ofMutex mPlayMutex;
   bool mLooping{ false };
   int mNumBars{ -1 };
   float mVolume{ 1 };

   juce::AudioFormatReader* mReader{};
   std::unique_ptr<juce::AudioSampleBuffer> mReadBuffer;
   int mSamplesLeftToRead{ 0 };
};
