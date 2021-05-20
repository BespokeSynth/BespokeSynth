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

Sample::Sample()
: mData(0)
, mNumSamples(0)
, mStartTime(0)
, mOffset(FLT_MAX)
, mRate(1)
, mStopPoint(-1)
, mLooping(false)
, mNumBars(-1)
, mVolume(1)
, mReader(nullptr)
, mSamplesLeftToRead(0)
{
   mName[0] = 0;
}

Sample::~Sample()
{
}

bool Sample::Read(const char* path, bool mono, ReadType readType)
{
   mReadPath = path;
   ofStringReplace(mReadPath, GetPathSeparator(), "/");
   vector<string> tokens = ofSplitString(mReadPath, "/");
   mName = tokens[tokens.size()-1].c_str();
   
   File file(ofToDataPath(mReadPath));
   delete mReader;
   mReader = TheSynth->GetGlobalManagers()->mAudioFormatManager.createReaderFor(file);
   
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
      mSampleRateRatio = float(mReader->sampleRate) / gSampleRate;
      
      mReadBuffer = make_unique<AudioSampleBuffer>();
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
      BufferCopy(mData.GetChannel(0), mReadBuffer->getReadPointer(0), mReadBuffer->getNumSamples());  //put first channel in
      for (int ch = 1; ch < mReadBuffer->getNumChannels(); ++ch)
         Add(mData.GetChannel(0), mReadBuffer->getReadPointer(ch), mReadBuffer->getNumSamples()); //add the other channels
      Mult(mData.GetChannel(0), 1.0f / mReadBuffer->getNumChannels(), mReadBuffer->getNumSamples());   //normalize volume
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
   for (int ch=0; ch<channels; ++ch)
      BufferCopy(mData.GetChannel(ch), data->GetChannel(ch), length);
   Setup(length);
}

void Sample::Setup(int length)
{
   mNumSamples = length;
   mRate = 1;
   mOffset = length;
   mSampleRateRatio = 1;
   mStopPoint = -1;
   mName = "newsample";
   mReadPath = "";
}

bool Sample::Write(const char* path /*=nullptr*/)
{
   const char* writeTo = path ? path : mReadPath.c_str();
   WriteDataToFile(writeTo, &mData, mNumSamples);
   return true;
}

//static
bool Sample::WriteDataToFile(const char *path, float **data, int numSamples, int channels)
{
   ScopedPointer<WavAudioFormat> wavFormat = new WavAudioFormat();
   File outputFile(ofToDataPath(path).c_str());
   outputFile.create();
   FileOutputStream* outputTo = outputFile.createOutputStream();
   assert(outputTo != nullptr);
   bool b1 {nullptr};
   ScopedPointer<AudioFormatWriter> writer = wavFormat->createWriterFor(outputTo, gSampleRate, channels, 16, b1, 0);
   writer->writeFromFloatArrays(data, channels, numSamples);
   
   return true;
}

//static
bool Sample::WriteDataToFile(const char *path, ChannelBuffer* data, int numSamples)
{
   int numChannels = data->NumActiveChannels();
   float** channelData = new float*[numChannels];
   for (int ch=0; ch<numChannels; ++ch)
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
   
   if (mOffset >= end || mOffset != mOffset)
   {
      mPlayMutex.unlock();
      return false;
   }
   
   LockDataMutex(true);
   for (int i=0; i<size; ++i)
   {
      if (time < mStartTime)
      {
         if (replace)
         {
            for (int ch=0; ch<out->NumActiveChannels(); ++ch)
               out->GetChannel(ch)[i] = 0;
         }
      }
      else
      {
         for (int ch=0; ch<out->NumActiveChannels(); ++ch)
         {
            int dataChannel = MIN(ch, mData.NumActiveChannels()-1);
            
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
   mSampleRateRatio = sample->mSampleRateRatio;
   mStopPoint = sample->mStopPoint;
   mName = sample->mName;
   mReadPath = sample->mReadPath;
}

namespace
{
   const int kSaveStateRev = 0;
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
   out << mSampleRateRatio;
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
      for (int ch=0; ch<mData.NumActiveChannels(); ++ch)
      {
         float* channelBuffer = mData.GetChannel(ch);
         for (int i=0; i<mData.BufferSize(); ++i)
         {
            assert(channelBuffer[i] >= -1 && channelBuffer[i] <= 1);
         }
      }
   }
   in >> mNumBars;
   in >> mLooping;
   in >> mRate;
   in >> mSampleRateRatio;
   in >> mStopPoint;
   in >> mName;
   in >> mReadPath;
}
