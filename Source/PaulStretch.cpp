#include "PaulStretch.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Profiler.h"
#include "Checkbox.h"
#include <cmath>
#include <random>

namespace
{
   constexpr float kTwoPi = 6.28318530717958647692f;
   const int kWindowSizes[] = { 4096, 16384, 32768, 65536, 131072, 262144 };
   const int kNumWindowSizes = sizeof(kWindowSizes) / sizeof(kWindowSizes[0]);
}

PaulStretch::PaulStretch()
: IAudioProcessor(gBufferSize)
{
   mInputBuffer = std::make_unique<RollingBuffer>(kWindowSizes[2] * 4);
   mInputBuffer->SetNumChannels(2);
   UpdateFFTSize(kWindowSizes[2]); // default to 32768
}

PaulStretch::~PaulStretch()
{
   if (mOwnsSample)
      delete mSample;
}

void PaulStretch::UpdateFFTSize(int newSize)
{
   if (newSize == mCurrentFFTSize)
      return;

   mCurrentFFTSize = newSize;
   mFFT = std::make_unique<FFT>(mCurrentFFTSize);

   mWindow.resize(mCurrentFFTSize);
   mFftInput.resize(mCurrentFFTSize);
   mFftReal.resize(mCurrentFFTSize / 2 + 1);
   mFftImag.resize(mCurrentFFTSize / 2 + 1);
   mFftOut.resize(mCurrentFFTSize);

   for (int i = 0; i < mCurrentFFTSize; ++i)
   {
      mWindow[i] = 0.5f * (1.0f - std::cos(kTwoPi * i / (mCurrentFFTSize - 1)));
   }

   if (!mInputBuffer || mInputBuffer->Size() != mCurrentFFTSize * 4)
   {
      mInputBuffer = std::make_unique<RollingBuffer>(mCurrentFFTSize * 4);
      mInputBuffer->SetNumChannels(2);
   }

   mOutputBuffer[0].Init(mCurrentFFTSize * 2);
   mOutputBuffer[1].Init(mCurrentFFTSize * 2);
}

void PaulStretch::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mStretchSlider = new FloatSlider(this, "stretch", 5, 20, 110, 15, &mStretch, 1.0f, 1000.0f);
   mPhaseRandSlider = new FloatSlider(this, "phase rand", 5, 40, 110, 15, &mPhaseRand, 0.0f, 1.0f);
   mMixSlider = new FloatSlider(this, "mix", 5, 60, 110, 15, &mMix, 0.0f, 1.0f);
   mVolumeSlider = new FloatSlider(this, "volume", 5, 80, 110, 15, &mVolume, 0.0f, 2.0f);
   mPlayStartSlider = new FloatSlider(this, "start", 5, 100, 110, 15, &mPlayStart, 0.0f, 1.0f);
   mPlayEndSlider = new FloatSlider(this, "end", 5, 120, 110, 15, &mPlayEnd, 0.0f, 1.0f);
   mCurrentSlider = new FloatSlider(this, "current", 5, 140, 110, 15, &mCurrentPct, 0.0f, 1.0f);

   mWindowSizeSlider = new FloatSlider(this, "window size", 120, 20, 110, 15, &mWindowSizeIdx, 0.0f, (float)(kNumWindowSizes - 1));
   mWindowSizeSlider->SetMode(FloatSlider::kSquare); // Integer steps

   mPitchShiftSlider = new FloatSlider(this, "pitch shift", 120, 40, 110, 15, &mPitchShift, -24.0f, 24.0f);
   mFineTuneSlider = new FloatSlider(this, "fine tune", 120, 60, 110, 15, &mFineTune, -100.0f, 100.0f);
   mFreqShiftSlider = new FloatSlider(this, "freq shift", 120, 80, 110, 15, &mFreqShift, -1000.0f, 1000.0f);

   mUnisonSlider = new FloatSlider(this, "unison", 120, 100, 110, 15, &mUnison, 1.0f, 8.0f);
   mUnisonSlider->SetMode(FloatSlider::kSquare); // Integer steps
   mDetuneSlider = new FloatSlider(this, "detune", 120, 120, 110, 15, &mDetune, 0.0f, 100.0f);

   mLoopCheckbox = new Checkbox(this, "loop", 120, 140, &mLoop);

   mWidth = 240;
   mHeight = 295; // Room for waveform
}

void PaulStretch::FilesDropped(std::vector<std::string> files, int x, int y)
{
   if (files.empty())
      return;

   Sample* sample = new Sample();
   sample->Read(files[0].c_str());
   UpdateSample(sample, true);
}

void PaulStretch::SampleDropped(int x, int y, Sample* sample)
{
   //NOTE: this used to require TheSynth->MouseMovedSignificantlySincePressed() before accepting the
   //drop. SampleDropped is only ever called from ModularSynth::MouseReleased's mHeldSample branch,
   //which already means a sample was legitimately grabbed and released here - no extra movement
   //gate is needed, and it was the reason dropping a second/different sample onto an already-loaded
   //PaulStretch silently did nothing (compare to SampleUniverse::SampleDropped, which has no such gate).
   if (sample == nullptr || sample->LengthInSamples() == 0)
      return;

   Sample* copy = new Sample();
   copy->CopyFrom(sample);
   UpdateSample(copy, true);
}

void PaulStretch::UpdateSample(Sample* sample, bool ownsSample)
{
   Sample* oldSamplePtr = mSample;
   bool ownedOldSample = mOwnsSample;

   mSample = sample;
   mOwnsSample = ownsSample;
   mSamplePlayPosition = 0;

   if (ownedOldSample)
      delete oldSamplePtr;

   mIsLoadingSample = true;
}

void PaulStretch::ButtonClicked(ClickButton* button, double time)
{
}

void PaulStretch::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   if (mSample && mSample->LengthInSamples() > 0)
   {
      // Waveform is at x=5, y=185, w=230, h=100
      if (x >= 5 && x <= 235 && y >= 185 && y <= 285)
      {
         float pct = (x - 5) / 230.0f;
         mSamplePlayPosition = pct * mSample->LengthInSamples();
         mCurrentPct = mSamplePlayPosition / mSample->LengthInSamples();
         mLastCurrentPct = mCurrentPct;
      }
   }
}

void PaulStretch::DrawModule()
{
   ofPushStyle();
   ofSetColor(255, 255, 255, 255);
   ofRect(0, 0, mWidth, mHeight);
   ofPopStyle();

   if (mSample && mSample->LengthInSamples() > 0 && mSample->Data())
   {
      if (mSample->LengthInSamples() != mDrawBuffer.BufferSize())
      {
         mDrawBuffer.Resize(mSample->LengthInSamples());
         mDrawBuffer.CopyFrom(mSample->Data());
      }

      float sampleWidth = 230;
      float sampleHeight = 100;
      ofPushMatrix();
      ofTranslate(5, 185); // Draw below the sliders

      ofPushStyle();
      ofSetColor(20, 20, 20, 255);
      ofRect(0, 0, sampleWidth, sampleHeight);
      ofPopStyle();

      DrawAudioBuffer(sampleWidth, sampleHeight, &mDrawBuffer, 0, mSample->LengthInSamples(), mSamplePlayPosition);

      ofPushStyle();
      // Draw loop start and end lines
      ofSetColor(0, 255, 0, 150);
      float startX = mPlayStart * sampleWidth;
      ofLine(startX, 0, startX, sampleHeight);

      ofSetColor(255, 0, 0, 150);
      float endX = mPlayEnd * sampleWidth;
      ofLine(endX, 0, endX, sampleHeight);

      ofSetColor(255, 255, 255, 200);
      DrawTextNormal(mSample->Name(), 5, 12);
      ofPopStyle();

      ofPopMatrix();
   }
   else
   {
      // Draw live audio output
      float sampleWidth = 230;
      float sampleHeight = 100;
      ofPushMatrix();
      ofTranslate(5, 165); // Draw below the sliders

      ofPushStyle();
      ofSetColor(20, 20, 20, 255);
      ofRect(0, 0, sampleWidth, sampleHeight);
      ofPopStyle();

      GetVizBuffer()->Draw(0, 0, sampleWidth, sampleHeight, GetVizBuffer()->Size());

      ofPushStyle();
      ofSetColor(255, 255, 255, 150);
      DrawTextNormal("live input", 5, 12);
      ofPopStyle();

      ofPopMatrix();
   }

   mStretchSlider->Draw();
   mPhaseRandSlider->Draw();
   mMixSlider->Draw();
   mVolumeSlider->Draw();
   mPlayStartSlider->Draw();
   mPlayEndSlider->Draw();
   mCurrentSlider->Draw();
   mWindowSizeSlider->Draw();
   mPitchShiftSlider->Draw();
   mFineTuneSlider->Draw();
   mFreqShiftSlider->Draw();
   mUnisonSlider->Draw();
   mDetuneSlider->Draw();
   mLoopCheckbox->Draw();
}

void PaulStretch::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mCurrentSlider)
   {
      if (mSample && mSample->LengthInSamples() > 0)
      {
         mSamplePlayPosition = mCurrentPct * mSample->LengthInSamples();
         mLastCurrentPct = mCurrentPct;
      }
   }
   if (slider == mWindowSizeSlider)
   {
      int idx = (int)(mWindowSizeIdx + 0.5f);
      if (idx >= 0 && idx < kNumWindowSizes)
      {
         UpdateFFTSize(kWindowSizes[idx]);
      }
   }
}

void PaulStretch::Process(double time)
{
   PROFILER(PaulStretch);

   ComputeSliders(0);

   IAudioReceiver* target = GetTarget();
   if (target == nullptr)
      return;

   SyncBuffers();
   int bufferSize = GetBuffer()->BufferSize();
   int numChannels = GetBuffer()->NumActiveChannels();

   ChannelBuffer* out = target->GetBuffer();

   // Step 1: Write input audio into the input rolling buffer (if no sample is loaded)
   if (!mSample)
   {
      for (int ch = 0; ch < numChannels && ch < 2; ++ch)
      {
         mInputBuffer->WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
      }
   }

   // Step 2: Overlap-add processing loop
   const int hopSize = mCurrentFFTSize / 4; // 75% overlap
   mHopAccumulator += bufferSize;

   static std::mt19937 rng(1337);
   static std::uniform_real_distribution<float> dist(0.0f, kTwoPi);

   while (mHopAccumulator >= hopSize)
   {
      mHopAccumulator -= hopSize;

      float readStep = hopSize / std::max(1.0f, mStretch);
      int readOffset = (int)readStep;

      for (int ch = 0; ch < numChannels && ch < 2; ++ch)
      {
         if (mSample && mSample->LengthInSamples() > 0)
         {
            // Read from sample with loop start/end logic
            float loopStart = mSample->LengthInSamples() * std::min(mPlayStart, mPlayEnd);
            float loopEnd = mSample->LengthInSamples() * std::max(mPlayStart, mPlayEnd);
            if (loopEnd <= loopStart)
               loopEnd = loopStart + 1;

            // Ensure playhead is within bounds
            if (mSamplePlayPosition < loopStart || mSamplePlayPosition >= loopEnd)
            {
               mSamplePlayPosition = loopStart;
            }

            int intPlayPos = (int)mSamplePlayPosition;
            for (int i = 0; i < mCurrentFFTSize; ++i)
            {
               int sampleIdx = intPlayPos + i;
               if (sampleIdx >= loopEnd)
               {
                  if (mLoop)
                     sampleIdx = (int)loopStart + (sampleIdx - (int)loopEnd) % (int)(loopEnd - loopStart);
                  else
                     sampleIdx = (int)loopEnd - 1;
               }
               // Get channel 0 or 1 depending on sample channel count
               int sampleCh = (ch < mSample->NumChannels()) ? ch : 0;
               mFftInput[i] = mSample->Data()->GetChannel(sampleCh)[sampleIdx];
            }
         }
         else
         {
            // Read audio from rolling buffer with hop offset scaled by stretch factor
            if (readOffset + mCurrentFFTSize > mInputBuffer->Size())
            {
               readOffset = mInputBuffer->Size() - mCurrentFFTSize;
            }
            if (readOffset < 0)
               readOffset = 0;

            mInputBuffer->ReadChunk(mFftInput.data(), mCurrentFFTSize, readOffset, ch);
         }

         // Apply window
         for (int i = 0; i < mCurrentFFTSize; ++i)
            mFftInput[i] *= mWindow[i];

         // Forward FFT
         mFFT->Forward(mFftInput.data(), mFftReal.data(), mFftImag.data());

         // Spectral Processing (Phase Randomization, Filters, Pitch Shift)
         int numFreqs = mCurrentFFTSize / 2 + 1;
         std::vector<float> magnitudes(numFreqs, 0.0f);

         // 1. Calculate Magnitudes
         for (int k = 0; k < numFreqs; ++k)
         {
            magnitudes[k] = std::sqrt(mFftReal[k] * mFftReal[k] + mFftImag[k] * mFftImag[k]);
         }

         // 2. Pitch Shift, Fine Tune, Frequency Shift, Unison & Phase Randomization
         float unisonVoices = std::max(1.0f, std::round(mUnison));
         if (std::abs(mPitchShift) > 0.01f || std::abs(mFineTune) > 0.01f || std::abs(mFreqShift) > 0.01f || unisonVoices > 1.1f)
         {
            std::vector<float> complexReal(numFreqs, 0.0f);
            std::vector<float> complexImag(numFreqs, 0.0f);
            float freqShiftBins = mFreqShift * mCurrentFFTSize / gSampleRate;

            for (int v = 0; v < unisonVoices; ++v)
            {
               float voiceDetuneCents = (unisonVoices <= 1.0f) ? 0.0f : (-mDetune + (2.0f * mDetune * v / (unisonVoices - 1.0f)));
               float totalPitchShift = mPitchShift + (mFineTune + voiceDetuneCents) / 100.0f;
               float pitchRatio = std::pow(2.0f, -totalPitchShift / 12.0f);

               for (int k = 0; k < numFreqs; ++k)
               {
                  float sourceK = k * pitchRatio - freqShiftBins;
                  if (sourceK >= 0.0f && sourceK < numFreqs - 1)
                  {
                     int k1 = (int)sourceK;
                     int k2 = k1 + 1;
                     float frac = sourceK - k1;
                     float interpolatedMag = magnitudes[k1] * (1.0f - frac) + magnitudes[k2] * frac;

                     if (interpolatedMag > 1e-6f)
                     {
                        // True unison requires independent phases for each voice!
                        float sourcePhase = std::atan2(mFftImag[k1], mFftReal[k1]);
                        float phase = sourcePhase * (1.0f - mPhaseRand) + dist(rng) * mPhaseRand;

                        complexReal[k] += interpolatedMag * std::cos(phase);
                        complexImag[k] += interpolatedMag * std::sin(phase);
                     }
                  }
               }
            }

            float normalize = 1.0f / std::sqrt(unisonVoices);
            for (int k = 0; k < numFreqs; ++k)
            {
               mFftReal[k] = complexReal[k] * normalize;
               mFftImag[k] = complexImag[k] * normalize;
            }
         }
         else
         {
            // 3. Normal Phase Randomization (no pitch shift)
            for (int k = 0; k < numFreqs; ++k)
            {
               float mag = magnitudes[k];
               if (mag > 1e-6f)
               {
                  float phase = std::atan2(mFftImag[k], mFftReal[k]);
                  float randomPhase = dist(rng);
                  phase = phase * (1.0f - mPhaseRand) + randomPhase * mPhaseRand;

                  mFftReal[k] = mag * std::cos(phase);
                  mFftImag[k] = mag * std::sin(phase);
               }
               else
               {
                  mFftReal[k] = 0.0f;
                  mFftImag[k] = 0.0f;
               }
            }
         }

         // Inverse FFT
         mFFT->Inverse(mFftReal.data(), mFftImag.data(), mFftOut.data());

         // Apply window to output and overlap-add to output buffer
         // mayer_ifft scales by N/2, and 75% overlap Hann^2 scales by 1.5.
         // Total scale = 0.75 * N. To normalize, multiply by 1 / (0.75 * N).
         float scale = 1.33333333f / mCurrentFFTSize;
         for (int i = 0; i < mCurrentFFTSize; ++i)
            mFftOut[i] *= mWindow[i] * scale;

         int writeOffset = mOutputBuffer[ch].readPos;
         mOutputBuffer[ch].Add(mFftOut.data(), mCurrentFFTSize, writeOffset);
      }

      // Advance playhead if sample is loaded
      if (mSample && mSample->LengthInSamples() > 0)
      {
         float loopStart = mSample->LengthInSamples() * std::min(mPlayStart, mPlayEnd);
         float loopEnd = mSample->LengthInSamples() * std::max(mPlayStart, mPlayEnd);
         if (loopEnd <= loopStart)
            loopEnd = loopStart + 1;

         mSamplePlayPosition += readStep;
         if (mSamplePlayPosition >= loopEnd)
         {
            if (mLoop)
               mSamplePlayPosition = loopStart + fmod(mSamplePlayPosition - loopEnd, loopEnd - loopStart);
            else
               mSamplePlayPosition = loopEnd - 1.0f;
         }

         mCurrentPct = mSamplePlayPosition / mSample->LengthInSamples();
         mLastCurrentPct = mCurrentPct;
      }
   }

   // Step 3: Stream from output buffer to final output channel buffer
   for (int ch = 0; ch < numChannels; ++ch)
   {
      int chIdx = (ch < 2) ? ch : 0;
      const float* inData = GetBuffer()->GetChannel(ch);
      float* outData = out->GetChannel(ch);

      for (int i = 0; i < bufferSize; ++i)
      {
         float wetSample = 0.0f;
         if (mEnabled)
            wetSample = mOutputBuffer[chIdx].ReadAndClear() * mVolume;
         else
            wetSample = inData[i];

         float drySample = inData[i];
         gWorkBuffer[i] = drySample * (1.0f - mMix) + wetSample * mMix;
      }
      Add(outData, gWorkBuffer, bufferSize);
      GetVizBuffer()->WriteChunk(gWorkBuffer, bufferSize, ch);
   }
}

void PaulStretch::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
   IDrawableModule::SaveState(out);

   //rev 1 never wrote the dropped-in sample at all, which is why reopening a saved patch always
   //came back with an empty PaulStretch - the sample data just wasn't in the file to begin with
   bool hasSample = (mSample != nullptr && mSample->LengthInSamples() > 0);
   out << hasSample;
   if (hasSample)
      mSample->SaveState(out);
}

void PaulStretch::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (rev >= 2)
   {
      bool hasSample;
      in >> hasSample;
      if (hasSample)
      {
         Sample* loaded = new Sample();
         loaded->LoadState(in);
         UpdateSample(loaded, true);
      }
   }
}
