//
//  FreqDomainBoilerplate.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/24/13.
//
//

#include "FreqDomainBoilerplate.h"
#include "Profiler.h"

namespace
{
   const int fftWindowSize = 1024;
   const int fftFreqDomainSize = fftWindowSize/2 + 1;
}

FreqDomainBoilerplate::FreqDomainBoilerplate()
: IAudioProcessor(gBufferSize)
, mFFT(fftWindowSize)
, mRollingInputBuffer(fftWindowSize)
, mRollingOutputBuffer(fftWindowSize)
, mFFTData(fftWindowSize, fftFreqDomainSize)
, mInputPreamp(1)
, mValue1(1)
, mVolume(1)
, mInputSlider(nullptr)
, mValue1Slider(nullptr)
, mVolumeSlider(nullptr)
, mDryWet(1)
, mDryWetSlider(nullptr)
, mValue2(1)
, mValue2Slider(nullptr)
, mValue3(0)
, mValue3Slider(nullptr)
, mPhaseOffset(0)
, mPhaseOffsetSlider(nullptr)
{
   // Generate a window with a single raised cosine from N/4 to 3N/4
   mWindower = new float[fftWindowSize];
   for (int i=0; i<fftWindowSize; ++i)
      mWindower[i] = -.5*cos(FTWO_PI*i/fftWindowSize)+.5;
}

void FreqDomainBoilerplate::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mInputSlider = new FloatSlider(this,"input", 5, 29, 100, 15, &mInputPreamp, 0, 2);
   mValue1Slider = new FloatSlider(this,"value 1", 5, 47, 100, 15, &mValue1, 0, 2);
   mVolumeSlider = new FloatSlider(this,"volume", 5, 65, 100, 15, &mVolume, 0, 2);
   mDryWetSlider = new FloatSlider(this,"dry/wet", 5, 83, 100, 15, &mDryWet, 0, 1);
   mValue2Slider = new FloatSlider(this,"value 2", 5, 101, 100, 15, &mValue2, 0, 1);
   mValue3Slider = new FloatSlider(this,"value 3", 5, 119, 100, 15, &mValue3, 0, 1);
   mPhaseOffsetSlider = new FloatSlider(this,"phase off", 5, 137, 100, 15, &mPhaseOffset, 0, FTWO_PI);
}

FreqDomainBoilerplate::~FreqDomainBoilerplate()
{
   delete[] mWindower;
}

void FreqDomainBoilerplate::Process(double time)
{
   PROFILER(FreqDomainBoilerplate);

   if (GetTarget() == nullptr || !mEnabled)
      return;

   ComputeSliders(0);
   SyncBuffers();

   float inputPreampSq = mInputPreamp * mInputPreamp;
   float volSq = mVolume * mVolume;

   int bufferSize = GetBuffer()->BufferSize();

   mRollingInputBuffer.WriteChunk(GetBuffer()->GetChannel(0), bufferSize, 0);

   //copy rolling input buffer into working buffer and window it
   mRollingInputBuffer.ReadChunk(mFFTData.mTimeDomain, fftWindowSize, 0, 0);
   Mult(mFFTData.mTimeDomain, mWindower, fftWindowSize);
   Mult(mFFTData.mTimeDomain, inputPreampSq, fftWindowSize);

   mFFT.Forward(mFFTData.mTimeDomain,
                mFFTData.mRealValues,
                mFFTData.mImaginaryValues);

   for (int i=0; i<fftFreqDomainSize; ++i)
   {
      float real = mFFTData.mRealValues[i];
      float imag = mFFTData.mImaginaryValues[i];

      //cartesian to polar
      float amp = 2.*sqrtf(real*real + imag*imag);
      float phase = atan2(imag,real);

      phase += mPhaseOffset;
      FloatWrap(phase, FTWO_PI);

      //polar to cartesian
      real = amp*cos(phase);
      imag = amp*sin(phase);

      mFFTData.mRealValues[i] = real;
      mFFTData.mImaginaryValues[i] = imag;
   }

   mFFT.Inverse(mFFTData.mRealValues,
                mFFTData.mImaginaryValues,
                mFFTData.mTimeDomain);

   for (int i=0; i<bufferSize; ++i)
      mRollingOutputBuffer.Write(0, 0);

   //copy rolling input buffer into working buffer and window it
   for (int i=0; i<fftWindowSize; ++i)
      mRollingOutputBuffer.Accum(fftWindowSize-i-1, mFFTData.mTimeDomain[i] * mWindower[i] * .0001f, 0);

   Mult(GetBuffer()->GetChannel(0), (1-mDryWet)*inputPreampSq, GetBuffer()->BufferSize());

   for (int i=0; i<bufferSize; ++i)
      GetBuffer()->GetChannel(0)[i] += mRollingOutputBuffer.GetSample(fftWindowSize-i-1, 0) * volSq * mDryWet;

   Add(GetTarget()->GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(0), bufferSize);

   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0),bufferSize, 0);

   GetBuffer()->Reset();
}

void FreqDomainBoilerplate::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   
   mInputSlider->Draw();
   mValue1Slider->Draw();
   mVolumeSlider->Draw();
   mDryWetSlider->Draw();
   mValue2Slider->Draw();
   mValue3Slider->Draw();
   mPhaseOffsetSlider->Draw();
}

void FreqDomainBoilerplate::CheckboxUpdated(Checkbox* checkbox)
{
}

