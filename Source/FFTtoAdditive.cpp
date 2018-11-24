//
//  FFTtoAdditive.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/24/13.
//
//

#include "FFTtoAdditive.h"
#include "ModularSynth.h"
#include "Profiler.h"

namespace
{
   const int fftWindowSize = 1024;
   const int fftFreqDomainSize = fftWindowSize/2 + 1;

   const int numPartials = fftFreqDomainSize-1;

   
   double sineBuffer[514]={0,0.012268,0.024536,0.036804,0.049042,0.06131,0.073547,0.085785,0.097992,0.1102,0.12241,0.13455,0.1467,0.15884,0.17093,0.18301,0.19507,0.20709,0.21909,0.23105,0.24295,0.25485,0.26669,0.2785,0.29025,0.30197,0.31366,0.32529,0.33685,0.34839,0.35986,0.37128,0.38266,0.39395,0.40521,0.41641,0.42752,0.4386,0.44958,0.46051,0.47137,0.48215,0.49286,0.50351,0.51407,0.52457,0.53497,0.54529,0.55554,0.5657,0.57578,0.58575,0.59567,0.60547,0.6152,0.62482,0.63437,0.6438,0.65314,0.66238,0.67151,0.68057,0.68951,0.69833,0.70706,0.7157,0.72421,0.7326,0.74091,0.74908,0.75717,0.76514,0.77298,0.7807,0.7883,0.79581,0.80316,0.81042,0.81754,0.82455,0.83142,0.8382,0.84482,0.85132,0.8577,0.86392,0.87006,0.87604,0.88187,0.8876,0.89319,0.89862,0.90396,0.90912,0.91415,0.91907,0.92383,0.92847,0.93295,0.93729,0.9415,0.94556,0.94949,0.95325,0.95691,0.96039,0.96375,0.96692,0.97,0.9729,0.97565,0.97827,0.98074,0.98306,0.98523,0.98724,0.98914,0.99084,0.99243,0.99387,0.99515,0.99628,0.99725,0.99808,0.99875,0.99927,0.99966,0.99988,0.99997,0.99988,0.99966,0.99927,0.99875,0.99808,0.99725,0.99628,0.99515,0.99387,0.99243,0.99084,0.98914,0.98724,0.98523,0.98306,0.98074,0.97827,0.97565,0.9729,0.97,0.96692,0.96375,0.96039,0.95691,0.95325,0.94949,0.94556,0.9415,0.93729,0.93295,0.92847,0.92383,0.91907,0.91415,0.90912,0.90396,0.89862,0.89319,0.8876,0.88187,0.87604,0.87006,0.86392,0.8577,0.85132,0.84482,0.8382,0.83142,0.82455,0.81754,0.81042,0.80316,0.79581,0.7883,0.7807,0.77298,0.76514,0.75717,0.74908,0.74091,0.7326,0.72421,0.7157,0.70706,0.69833,0.68951,0.68057,0.67151,0.66238,0.65314,0.6438,0.63437,0.62482,0.6152,0.60547,0.59567,0.58575,0.57578,0.5657,0.55554,0.54529,0.53497,0.52457,0.51407,0.50351,0.49286,0.48215,0.47137,0.46051,0.44958,0.4386,0.42752,0.41641,0.40521,0.39395,0.38266,0.37128,0.35986,0.34839,0.33685,0.32529,0.31366,0.30197,0.29025,0.2785,0.26669,0.25485,0.24295,0.23105,0.21909,0.20709,0.19507,0.18301,0.17093,0.15884,0.1467,0.13455,0.12241,0.1102,0.097992,0.085785,0.073547,0.06131,0.049042,0.036804,0.024536,0.012268,0,-0.012268,-0.024536,-0.036804,-0.049042,-0.06131,-0.073547,-0.085785,-0.097992,-0.1102,-0.12241,-0.13455,-0.1467,-0.15884,-0.17093,-0.18301,-0.19507,-0.20709,-0.21909,-0.23105,-0.24295,-0.25485,-0.26669,-0.2785,-0.29025,-0.30197,-0.31366,-0.32529,-0.33685,-0.34839,-0.35986,-0.37128,-0.38266,-0.39395,-0.40521,-0.41641,-0.42752,-0.4386,-0.44958,-0.46051,-0.47137,-0.48215,-0.49286,-0.50351,-0.51407,-0.52457,-0.53497,-0.54529,-0.55554,-0.5657,-0.57578,-0.58575,-0.59567,-0.60547,-0.6152,-0.62482,-0.63437,-0.6438,-0.65314,-0.66238,-0.67151,-0.68057,-0.68951,-0.69833,-0.70706,-0.7157,-0.72421,-0.7326,-0.74091,-0.74908,-0.75717,-0.76514,-0.77298,-0.7807,-0.7883,-0.79581,-0.80316,-0.81042,-0.81754,-0.82455,-0.83142,-0.8382,-0.84482,-0.85132,-0.8577,-0.86392,-0.87006,-0.87604,-0.88187,-0.8876,-0.89319,-0.89862,-0.90396,-0.90912,-0.91415,-0.91907,-0.92383,-0.92847,-0.93295,-0.93729,-0.9415,-0.94556,-0.94949,-0.95325,-0.95691,-0.96039,-0.96375,-0.96692,-0.97,-0.9729,-0.97565,-0.97827,-0.98074,-0.98306,-0.98523,-0.98724,-0.98914,-0.99084,-0.99243,-0.99387,-0.99515,-0.99628,-0.99725,-0.99808,-0.99875,-0.99927,-0.99966,-0.99988,-0.99997,-0.99988,-0.99966,-0.99927,-0.99875,-0.99808,-0.99725,-0.99628,-0.99515,-0.99387,-0.99243,-0.99084,-0.98914,-0.98724,-0.98523,-0.98306,-0.98074,-0.97827,-0.97565,-0.9729,-0.97,-0.96692,-0.96375,-0.96039,-0.95691,-0.95325,-0.94949,-0.94556,-0.9415,-0.93729,-0.93295,-0.92847,-0.92383,-0.91907,-0.91415,-0.90912,-0.90396,-0.89862,-0.89319,-0.8876,-0.88187,-0.87604,-0.87006,-0.86392,-0.8577,-0.85132,-0.84482,-0.8382,-0.83142,-0.82455,-0.81754,-0.81042,-0.80316,-0.79581,-0.7883,-0.7807,-0.77298,-0.76514,-0.75717,-0.74908,-0.74091,-0.7326,-0.72421,-0.7157,-0.70706,-0.69833,-0.68951,-0.68057,-0.67151,-0.66238,-0.65314,-0.6438,-0.63437,-0.62482,-0.6152,-0.60547,-0.59567,-0.58575,-0.57578,-0.5657,-0.55554,-0.54529,-0.53497,-0.52457,-0.51407,-0.50351,-0.49286,-0.48215,-0.47137,-0.46051,-0.44958,-0.4386,-0.42752,-0.41641,-0.40521,-0.39395,-0.38266,-0.37128,-0.35986,-0.34839,-0.33685,-0.32529,-0.31366,-0.30197,-0.29025,-0.2785,-0.26669,-0.25485,-0.24295,-0.23105,-0.21909,-0.20709,-0.19507,-0.18301,-0.17093,-0.15884,-0.1467,-0.13455,-0.12241,-0.1102,-0.097992,-0.085785,-0.073547,-0.06131,-0.049042,-0.036804,-0.024536,-0.012268,0,0.012268
   };
}

FFTtoAdditive::FFTtoAdditive()
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
, mHistoryPtr(0)
{
   // Generate a window with a single raised cosine from N/4 to 3N/4
   mWindower = new float[fftWindowSize];
   for (int i=0; i<fftWindowSize; ++i)
      mWindower[i] = -.5*cos(FTWO_PI*i/fftWindowSize)+.5;

   mPhaseInc = new float[numPartials];
   for (int i=0; i<numPartials; ++i)
   {
      float freq = i/float(fftFreqDomainSize) * (gNyquistLimit/2);
      mPhaseInc[i] = GetPhaseInc(freq);
   }

   for (int i=0; i<fftFreqDomainSize; ++i)
   {
      mFFTData.mRealValues[i] = 0;
      mFFTData.mImaginaryValues[i] = 0;
   }
}

void FFTtoAdditive::CreateUIControls()
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

FFTtoAdditive::~FFTtoAdditive()
{
   delete[] mWindower;
}

void FFTtoAdditive::Process(double time)
{
   PROFILER(FFTtoAdditive);

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

      mFFTData.mRealValues[i] = amp / (fftWindowSize/2);
      mFFTData.mImaginaryValues[i] = phase;
   }

   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   for (int i=0; i<bufferSize; ++i)
   {
      float write = 0;
      for (int j=1; j<numPartials; ++j)
      {
         float phase = ((mFFTData.mImaginaryValues[j+1] + i*mPhaseInc[j]) / FTWO_PI) * 512;
         float sample = SinSample(phase) * mFFTData.mRealValues[j+1] * volSq * .4f;
         write += sample;
      }

      GetVizBuffer()->Write(write, 0);

      out[i] += write;

      time += gInvSampleRateMs;
   }

   GetBuffer()->Reset();
}

float FFTtoAdditive::SinSample(float phase)
{
   int intPhase = int(phase) % 512;
   if (intPhase < 0)
      intPhase += 512;
   float remainder = phase - int(phase);
   float retVal = ((1-remainder) * sineBuffer[intPhase] + remainder * sineBuffer[1+intPhase]);
   return retVal;
}

void FFTtoAdditive::DrawModule()
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

   DrawViz();
}

void FFTtoAdditive::DrawViz()
{
   ofPushStyle();

   int zeroHeight = 240;

   for (int i=1; i<RAZOR_HISTORY-1; ++i)
   {
      float age = 1 - float(i) / RAZOR_HISTORY;
      ofSetColor(0, 200*age, 255*age);
      for (int x=0; x<VIZ_WIDTH; ++x)
      {
         int intHeight = mPeakHistory[(i+mHistoryPtr) % RAZOR_HISTORY][x];
         int intHeightNext = mPeakHistory[(i+mHistoryPtr+1) % RAZOR_HISTORY][x];
         if (intHeight != 0)
         {
            int xpos = 10+x+i;
            int ypos = zeroHeight-intHeight-i;
            int xposNext = xpos;
            int yposNext = ypos;
            if (intHeightNext != 0)
            {
               xposNext = 10+x+i+1;
               yposNext = zeroHeight-intHeightNext-i+1;
            }
            if (xpos < 1020)
               ofLine(xpos, ypos, xposNext, yposNext);
         }
      }
   }

   bzero(mPeakHistory[mHistoryPtr], sizeof(float) * VIZ_WIDTH);
   for (int i=1; i<=numPartials; ++i)
   {
      float height = mFFTData.mRealValues[i-1];
      int intHeight = int(height*100.0f);
      if (intHeight == 0)
      {
         if (height > 0) intHeight = 1;
         if (height < 0) intHeight = -1;
      }
      if (intHeight < 0)
      {
         ofSetColor(255,0,0);
         intHeight *= -1;
      }
      else
      {
         ofSetColor(255,255,255);
      }
      int x = int(ofMap(i,4,numPartials,0,VIZ_WIDTH,true));
      ofLine(10+x,zeroHeight,10+x,zeroHeight-intHeight);
      mPeakHistory[mHistoryPtr][x] = intHeight;
   }

   mHistoryPtr = (mHistoryPtr - 1 + RAZOR_HISTORY) % RAZOR_HISTORY;
   ofPopStyle();
}

void FFTtoAdditive::CheckboxUpdated(Checkbox* checkbox)
{
}

void FFTtoAdditive::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void FFTtoAdditive::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

