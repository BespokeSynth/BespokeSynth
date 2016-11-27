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

class FileStreamOut;
class FileStreamIn;

#define MAX_SAMPLE_READ_PATH_LENGTH 1024

class Sample
{
public:
   Sample();
   ~Sample();
   bool Read(const char* path);
   bool Write(const char* path = NULL);   //no path = use read filename
   bool ConsumeData(float* data, int size, bool replace);
   void Play(float rate = 1, int offset=0, int stopPoint=-1);
   void SetRate(float rate) { mRate = rate; }
   const char* Name() { return mName; }
   int LengthInSamples() { return mNumSamples; }
   float* Data() const {return mData; }
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
   bool IsPlaying() { return mOffset < mNumSamples; }
   void LockDataMutex(bool lock) { lock ? mDataMutex.lock() : mDataMutex.unlock(); }
   void Create(int length);
   void Create(float* data, int length);
   void SetLooping(bool looping) { mLooping = looping; }
   void SetNumBars(int numBars) { mNumBars = numBars; }
   int GetNumBars() const { return mNumBars; }
   void SetVolume(float vol) { mVolume = vol; }
   
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);
private:
   
   float* mData;
   int mNumSamples;
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
