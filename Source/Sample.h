//
//  Sample.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#ifndef __modularSynth__Sample__
#define __modularSynth__Sample__

#include "OpenFrameworksPort.h"
#include "ChannelBuffer.h"

class FileStreamOut;
class FileStreamIn;

#define MAX_SAMPLE_READ_PATH_LENGTH 1024

class Sample
{
public:
   Sample();
   ~Sample();
   bool Read(const char* path, bool mono = false);
   bool Write(const char* path = nullptr);   //no path = use read filename
   bool ConsumeData(double time, ChannelBuffer* out, int size, bool replace);
   void Play(double time, float rate, int offset, int stopPoint=-1);
   void SetRate(float rate) { mRate = rate; }
   const char* Name() { return mName; }
   int LengthInSamples() const { return mNumSamples; }
   int NumChannels() const { return mData.NumActiveChannels(); }
   ChannelBuffer* Data() { return &mData; }
   int GetPlayPosition() const { return mOffset; }
   void SetPlayPosition(int sample) { mOffset = sample; }
   float GetSampleRateRatio() const { return mSampleRateRatio; }
   void Reset() { mOffset = mNumSamples; }
   void SetStopPoint(int stopPoint) { mStopPoint = stopPoint; }
   void ClearStopPoint() { mStopPoint = -1; }
   void PadBack(int amount);
   void ClipTo(int start, int end);
   void ShiftWrap(int numSamples);
   const char* GetReadPath() const { return mReadPath; }
   static bool WriteDataToFile(const char* path, float** data, int numSamples, int channels = 1);
   static bool WriteDataToFile(const char* path, ChannelBuffer* data, int numSamples);
   bool IsPlaying() { return mOffset < mNumSamples; }
   void LockDataMutex(bool lock) { lock ? mDataMutex.lock() : mDataMutex.unlock(); }
   void Create(int length);
   void Create(ChannelBuffer* data);
   void SetLooping(bool looping) { mLooping = looping; }
   void SetNumBars(int numBars) { mNumBars = numBars; }
   int GetNumBars() const { return mNumBars; }
   void SetVolume(float vol) { mVolume = vol; }
   void CopyFrom(Sample* sample);
   
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);
private:
   void Setup(int length);
   
   ChannelBuffer mData;
   int mNumSamples;
   double mStartTime;
   double mOffset;
   float mRate;
   float mSampleRateRatio;
   int mStopPoint;
   char mName[32];
   char mReadPath[MAX_SAMPLE_READ_PATH_LENGTH];
   ofMutex mDataMutex;
   ofMutex mPlayMutex;
   bool mLooping;
   int mNumBars;
   float mVolume;
};

#endif /* defined(__modularSynth__Sample__) */
