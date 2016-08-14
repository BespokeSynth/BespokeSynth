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

Sample::Sample()
: mData(NULL)
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
   delete[] mData;
}

bool Sample::Read(const char* path)
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
      
      delete[] mData;
      mData = new float[mNumSamples];
      
      AudioSampleBuffer fileBuffer;
      fileBuffer.setSize (reader->numChannels, mNumSamples);
      reader->read(&fileBuffer, 0, mNumSamples, 0, true, true);
      
      for (int i=0; i<reader->lengthInSamples; ++i)
      {
         mData[i] = fileBuffer.getSample(0, i); //put first channel in
         for (int ch=1; ch<reader->numChannels; ++ch)
            mData[i] += fileBuffer.getSample(ch, i); //add the other channels
         mData[i] /= reader->numChannels;   //normalize volume
      }
      
      Reset();
      return true;
   }
   
   return false;
}

void Sample::Create(int length)
{
   Create(NULL, length);
}

void Sample::Create(float* data, int length)
{
   delete[] mData;
   mData = new float[length];
   if (data)
      memcpy(mData, data, sizeof(float) * length);
   else
      Clear(mData, length);
   mNumSamples = length;
   mRate = 1;
   mOffset = length;
   mSampleRateRatio = 1;
   mStopPoint = -1;
   strcpy(mName, "newsample");
   mReadPath[0] = 0;
}

bool Sample::Write(const char* path /*=NULL*/)
{
   const char* writeTo = path ? path : mReadPath;
   WriteDataToFile(writeTo, &mData[0], mNumSamples);
   return true;
}

//static
bool Sample::WriteDataToFile(const char *path, float *data, int numSamples, int channels)
{
   /*for (int i=0; i<numSamples; ++i)
      data[i] = ofClamp(data[i],-1.0f,1.0f);
   
   const int format=SF_FORMAT_WAV | SF_FORMAT_PCM_16;
   //const int format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;
   const int sampleRate=gSampleRate;
   
   SndfileHandle outfile(ofToDataPath(path).c_str(), SFM_WRITE, format, channels, gSampleRate);
   if (!outfile)
      return false;
   
   outfile.write(data, numSamples);*/
   return true;
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

/*bool Sample::ConsumeData(float* data, int size)
 {
 if (mOffset >= mNumSamples)
 return false;
 
 int samplesLeft = mNumSamples - mOffset;
 memcpy(data, mData+mOffset, MIN(samplesLeft,size)*sizeof(float));
 bzero(data+mOffset+samplesLeft, size-samplesLeft*sizeof(float)); //fill the rest with zero
 mOffset += size;
 return true;
 }*/

bool Sample::ConsumeData(float* data, int size, bool replace)
{
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
      float sample = 0;
      if (mOffset < end || mLooping)
         sample = GetInterpolatedSample(mOffset, mData, mNumSamples) * mVolume;
      
      if (replace)
         data[i] = sample;
      else
         data[i] += sample;
      
      mOffset += mRate * mSampleRateRatio;
   }
   LockDataMutex(false);
   mPlayMutex.unlock();
   
   return true;
}

void Sample::PadBack(int amount)
{
   int newSamples = mNumSamples + amount;
   float* newData = new float[newSamples];
   memcpy(newData, mData, sizeof(float) * mNumSamples);
   bzero(newData+mNumSamples, sizeof(float)*amount);
   LockDataMutex(true);
   delete[] mData;
   mData = newData;
   LockDataMutex(false);
   mNumSamples = newSamples;
}

void Sample::ClipTo(int start, int end)
{
   assert(start < end);
   assert(end <= mNumSamples);
   int newSamples = end-start;
   float* newData = new float[newSamples];
   memcpy(newData, mData+start, sizeof(float) * newSamples);
   LockDataMutex(true);
   delete[] mData;
   mData = newData;
   LockDataMutex(false);
   mNumSamples = newSamples;
}

void Sample::ShiftWrap(int numSamplesToShift)
{
   assert(numSamplesToShift <= mNumSamples);
   float* newData = new float[mNumSamples];
   int chunk = mNumSamples - numSamplesToShift;
   memcpy(newData, mData+numSamplesToShift, sizeof(float) * chunk);
   memcpy(newData+chunk, mData, sizeof(float) * numSamplesToShift);
   LockDataMutex(true);
   delete[] mData;
   mData = newData;
   LockDataMutex(false);
}

namespace
{
   const int kSaveStateRev = 0;
}

void Sample::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << mNumSamples;
   out.Write(mData, mNumSamples);
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
   LoadStateValidate(rev == kSaveStateRev);
   
   int numSamples;
   in >> numSamples;
   float* sample = new float[numSamples];
   in.Read(sample, numSamples);
   Create(sample, numSamples);
   delete[] sample;
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
