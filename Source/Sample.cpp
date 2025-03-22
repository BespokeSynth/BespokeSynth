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
//  Sample.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#include "Sample.h"
#include "SynthGlobals.h"
#include "FileStream.h"
#include "ModularSynth.h"
#include "ChannelBuffer.h"
#include <memory>

#include "juce_audio_formats/juce_audio_formats.h"

Sample::Sample()
{
}

Sample::~Sample()
{
}

bool Sample::Read(const char* path, bool mono, ReadType readType)
{
   mReadPath = path;
   ofStringReplace(mReadPath, GetPathSeparator(), "/");
   std::vector<std::string> tokens = ofSplitString(mReadPath, "/");
   mName = tokens[tokens.size() - 1];

   juce::File file(ofToSamplePath(mReadPath));
   delete mReader;
   mReader = TheSynth->GetAudioFormatManager().createReaderFor(file);

   if (mReader != nullptr)
   {
      mData.Resize((int)mReader->lengthInSamples);
      if (mono)
         mData.SetNumActiveChannels(1);
      else
         mData.SetNumActiveChannels(mReader->numChannels);
      mData.Clear();

      mNumSamples = (int)mReader->lengthInSamples;
      mOffset = mNumSamples;
      mOriginalSampleRate = mReader->sampleRate;
      mSampleRateRatio = float(mOriginalSampleRate) / gSampleRate;

      mReadBuffer = std::make_unique<juce::AudioSampleBuffer>();
      mReadBuffer->setSize(mReader->numChannels, mNumSamples);

      if (readType == ReadType::Sync)
      {
         mReader->read(mReadBuffer.get(), 0, mNumSamples, 0, true, true);
         FinishRead();
      }
      else if (readType == ReadType::Async)
      {
         mSamplesLeftToRead = mNumSamples;
         startTimer(100);
      }

      return true;
   }
   else
   {
      TheSynth->LogEvent("failed to load sample " + file.getFullPathName().toStdString(), kLogEventType_Error);
   }

   return false;
}

void Sample::FinishRead()
{
   if (mData.NumActiveChannels() == 1 && mReadBuffer->getNumChannels() > 1)
   {
      BufferCopy(mData.GetChannel(0), mReadBuffer->getReadPointer(0), mReadBuffer->getNumSamples()); //put first channel in
      for (int ch = 1; ch < mReadBuffer->getNumChannels(); ++ch)
         Add(mData.GetChannel(0), mReadBuffer->getReadPointer(ch), mReadBuffer->getNumSamples()); //add the other channels
      Mult(mData.GetChannel(0), 1.0f / mReadBuffer->getNumChannels(), mReadBuffer->getNumSamples()); //normalize volume
   }
   else
   {
      for (int ch = 0; ch < mReadBuffer->getNumChannels(); ++ch)
         BufferCopy(mData.GetChannel(ch), mReadBuffer->getReadPointer(ch), mReadBuffer->getNumSamples());
   }
}

//juce::Timer
void Sample::timerCallback()
{
   int samplesToRead = 44100 * 10;
   if (samplesToRead > mSamplesLeftToRead)
      samplesToRead = mSamplesLeftToRead;
   int startSample = mNumSamples - mSamplesLeftToRead;
   mReader->read(mReadBuffer.get(), startSample, samplesToRead, startSample, true, true);
   mSamplesLeftToRead -= samplesToRead;

   if (mSamplesLeftToRead <= 0)
   {
      FinishRead();
      stopTimer();
   }
}

void Sample::Create(int length)
{
   mData.Resize(length);
   mData.SetNumActiveChannels(1);
   Setup(length);
}

void Sample::Create(ChannelBuffer* data)
{
   int channels = data->NumActiveChannels();
   int length = data->BufferSize();
   mData.Resize(length);
   mData.SetNumActiveChannels(channels);
   for (int ch = 0; ch < channels; ++ch)
      BufferCopy(mData.GetChannel(ch), data->GetChannel(ch), length);
   Setup(length);
}

void Sample::Setup(int length)
{
   mNumSamples = length;
   mRate = 1;
   mOffset = length;
   mOriginalSampleRate = gSampleRate;
   mSampleRateRatio = 1;
   mStopPoint = -1;
   mName = "newsample";
   mReadPath = "";
}

bool Sample::Write(const char* path /*=nullptr*/)
{
   const std::string writeTo = path ? path : mReadPath;
   WriteDataToFile(writeTo, &mData, mNumSamples);
   return true;
}

//static
bool Sample::WriteDataToFile(const std::string& path, float** data, int numSamples, int channels)
{
   auto wavFormat = std::make_unique<juce::WavAudioFormat>();
   juce::File outputFile(ofToSamplePath(path));
   outputFile.create();
   auto outputTo = outputFile.createOutputStream();
   assert(outputTo != nullptr);
   bool b1{ false };
   auto writer = std::unique_ptr<juce::AudioFormatWriter>(
   wavFormat->createWriterFor(outputTo.release(), gSampleRate, channels, 16, b1, 0));
   writer->writeFromFloatArrays(data, channels, numSamples);

   return true;
}

//static
bool Sample::WriteDataToFile(const std::string& path, ChannelBuffer* data, int numSamples)
{
   int numChannels = data->NumActiveChannels();
   float** channelData = new float*[numChannels];
   for (int ch = 0; ch < numChannels; ++ch)
      channelData[ch] = data->GetChannel(ch);
   bool ret = WriteDataToFile(path, channelData, numSamples, numChannels);
   delete[] channelData;
   return ret;
}

void Sample::Play(double startTime, float rate /*=1*/, int offset /*=0*/, int stopPoint /*=-1*/)
{
   mPlayMutex.lock();
   mStartTime = startTime;
   mOffset = offset;
   mRate = rate;
   if (stopPoint != -1)
      SetStopPoint(stopPoint);
   else
      ClearStopPoint();
   mPlayMutex.unlock();
}

bool Sample::ConsumeData(double time, ChannelBuffer* out, int size, bool replace)
{
   assert(size <= out->BufferSize());

   mPlayMutex.lock();
   float end = mNumSamples;
   if (mStopPoint != -1)
      end = mStopPoint;

   if (mLooping && mOffset >= mNumSamples)
      mOffset -= mNumSamples;

   if (mOffset >= end || std::isnan(mOffset))
   {
      mPlayMutex.unlock();
      return false;
   }

   LockDataMutex(true);
   for (int i = 0; i < size; ++i)
   {
      if (time < mStartTime)
      {
         if (replace)
         {
            for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
               out->GetChannel(ch)[i] = 0;
         }
      }
      else
      {
         for (int ch = 0; ch < out->NumActiveChannels(); ++ch)
         {
            int dataChannel = MIN(ch, mData.NumActiveChannels() - 1);

            float sample = 0;
            if (mOffset < end || mLooping)
               sample = GetInterpolatedSample(mOffset, mData.GetChannel(dataChannel), mNumSamples) * mVolume;

            if (replace)
               out->GetChannel(ch)[i] = sample;
            else
               out->GetChannel(ch)[i] += sample;
         }

         mOffset += mRate * mSampleRateRatio;
      }
      time += gInvSampleRateMs;
   }
   LockDataMutex(false);
   mPlayMutex.unlock();

   return true;
}

void Sample::PadBack(int amount)
{
   //TODO(Ryan)
   /*int newSamples = mNumSamples + amount;
   float* newData = new float[newSamples];
   BufferCopy(newData, mData, mNumSamples);
   Clear(newData+mNumSamples, amount);
   LockDataMutex(true);
   delete mData;
   mData = newData;
   LockDataMutex(false);
   mNumSamples = newSamples;*/
}

void Sample::ClipTo(int start, int end)
{
   //TODO(Ryan)
   /*assert(start < end);
   assert(end <= mNumSamples);
   int newSamples = end-start;
   float* newData = new float[newSamples];
   BufferCopy(newData, mData+start, newSamples);
   LockDataMutex(true);
   delete mData;
   mData = newData;
   LockDataMutex(false);
   mNumSamples = newSamples;*/
}

void Sample::ShiftWrap(int numSamplesToShift)
{
   //TODO(Ryan)
   /*assert(numSamplesToShift <= mNumSamples);
   float* newData = new float[mNumSamples];
   int chunk = mNumSamples - numSamplesToShift;
   BufferCopy(newData, mData+numSamplesToShift, chunk);
   BufferCopy(newData+chunk, mData, numSamplesToShift);
   LockDataMutex(true);
   delete mData;
   mData = newData;
   LockDataMutex(false);*/
}

void Sample::CopyFrom(Sample* sample)
{
   mNumSamples = sample->mNumSamples;
   if (mData.BufferSize() != sample->mData.BufferSize())
      mData.Resize(sample->mNumSamples);
   mData.CopyFrom(&sample->mData);
   mNumBars = sample->mNumBars;
   mLooping = sample->mLooping;
   mRate = sample->mRate;
   mOriginalSampleRate = sample->mOriginalSampleRate;
   mSampleRateRatio = sample->mSampleRateRatio;
   mStopPoint = sample->mStopPoint;
   mName = sample->mName;
   mReadPath = sample->mReadPath;
}

namespace
{
   const int kSaveStateRev = 1;
}

void Sample::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << mNumSamples;
   if (mNumSamples > 0)
      mData.Save(out, mNumSamples);
   out << mNumBars;
   out << mLooping;
   out << mRate;
   out << mOriginalSampleRate;
   out << mStopPoint;
   out << mName;
   out << mReadPath;
}

void Sample::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;

   in >> mNumSamples;
   if (mNumSamples > 0)
   {
      int readLength;
      mData.Load(in, readLength, ChannelBuffer::LoadMode::kSetBufferSize);
      assert(readLength == mNumSamples);
      /*for (int ch=0; ch<mData.NumActiveChannels(); ++ch)
      {
         float* channelBuffer = mData.GetChannel(ch);
         for (int i=0; i<mData.BufferSize(); ++i)
         {
            assert(channelBuffer[i] >= -1 && channelBuffer[i] <= 1);
         }
      }*/
   }
   in >> mNumBars;
   in >> mLooping;
   in >> mRate;
   if (rev == 0)
   {
      in >> mSampleRateRatio;
      mOriginalSampleRate = gSampleRate * mSampleRateRatio;
   }
   else
   {
      in >> mOriginalSampleRate;
      mSampleRateRatio = float(mOriginalSampleRate) / gSampleRate;
   }
   in >> mStopPoint;
   in >> mName;
   in >> mReadPath;
}
