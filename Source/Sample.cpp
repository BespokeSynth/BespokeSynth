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
, mOffset(FLT_MAX)
, mRate(1)
, mStopPoint(-1)
, mLooping(false)
, mNumBars(-1)
, mVolume(1)
{
   mName[0] = 0;
}

Sample::~Sample()
{
}

bool Sample::Read(const char* path, bool mono)
{
   StringCopy(mReadPath,path,MAX_SAMPLE_READ_PATH_LENGTH);
   vector<string> tokens = ofSplitString(path,"/");
   StringCopy(mName,tokens[tokens.size()-1].c_str(),32);
   mName[strlen(mName)-4] = 0;
   
   File file(ofToDataPath(path));
   ScopedPointer<AudioFormatReader> reader(TheSynth->GetGlobalManagers()->mAudioFormatManager.createReaderFor(file));
   
   if (reader != nullptr)
   {
      mNumSamples = reader->lengthInSamples;
      mSampleRateRatio = float(reader->sampleRate) / gSampleRate;
      
      mData.Resize(mNumSamples);
      
      AudioSampleBuffer fileBuffer;
      fileBuffer.setSize (reader->numChannels, mNumSamples);
      reader->read(&fileBuffer, 0, mNumSamples, 0, true, true);
      
      if (mono)
         mData.SetNumActiveChannels(1);
      else
         mData.SetNumActiveChannels(reader->numChannels);
      
      for (int i=0; i<reader->lengthInSamples; ++i)
      {
         if (mono)
         {
            mData.GetChannel(0)[i] = fileBuffer.getSample(0, i); //put first channel in
            for (int ch=1; ch<reader->numChannels; ++ch)
               mData.GetChannel(0)[i] += fileBuffer.getSample(ch, i); //add the other channels
            mData.GetChannel(0)[i] /= reader->numChannels;   //normalize volume
         }
         else
         {
            for (int ch=0; ch<reader->numChannels; ++ch)
               mData.GetChannel(ch)[i] = fileBuffer.getSample(ch, i);
         }
      }
      
      Reset();
      return true;
   }
   
   return false;
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
   strcpy(mName, "newsample");
   mReadPath[0] = 0;
}

bool Sample::Write(const char* path /*=nullptr*/)
{
   const char* writeTo = path ? path : mReadPath;
   WriteDataToFile(writeTo, &mData, mNumSamples);
   return true;
}

//static
bool Sample::WriteDataToFile(const char *path, float **data, int numSamples, int channels)
{
   ScopedPointer<WavAudioFormat> wavFormat = new WavAudioFormat();
   File outputFile(ofToDataPath(path).c_str());
   FileOutputStream* outputTo = outputFile.createOutputStream();
    
   ScopedPointer<AudioFormatWriter> writer = wavFormat->createWriterFor(outputTo, gSampleRate, channels, 16, nullptr, 0);
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

void Sample::Play(float rate /*=1*/, int offset /*=0*/, int stopPoint /*=-1*/)
{
   mPlayMutex.lock();
   mOffset = offset;
   mRate = rate;
   if (stopPoint != -1)
      SetStopPoint(stopPoint);
   else
      ClearStopPoint();
   mPlayMutex.unlock();
}

bool Sample::ConsumeData(ChannelBuffer* out, int size, bool replace)
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
   out << string(mName);
   out << string(mReadPath);
}

void Sample::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   
   int numSamples;
   in >> numSamples;
   if (numSamples > 0)
   {
      int readLength;
      mData.Load(in, readLength);
      assert(readLength == mNumSamples);
   }
   in >> mNumBars;
   in >> mLooping;
   in >> mRate;
   in >> mSampleRateRatio;
   in >> mStopPoint;
   string name;
   in >> name;
   StringCopy(mName,name.c_str(),32);
   string readPath;
   in >> readPath;
   StringCopy(mReadPath, readPath.c_str(), MAX_SAMPLE_READ_PATH_LENGTH);
}
