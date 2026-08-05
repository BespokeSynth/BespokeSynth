#pragma once

#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "Sample.h"
#include "FFT.h"
#include <vector>

class PaulStretch : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IButtonListener
{
public:
   PaulStretch();
   virtual ~PaulStretch();
   static IDrawableModule* Create() { return new PaulStretch(); }
   static bool AcceptsAudio() { return false; } //sampler-only - it reads from a dropped-in Sample, not a live input
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   // IAudioSource
   void Process(double time) override;

   // IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; } //rev 2 adds saving/loading the dropped-in sample itself

   // Drag and drop sample support
   void FilesDropped(std::vector<std::string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }

private:
   void DrawModule() override;
   void OnClicked(float x, float y, bool right) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void UpdateFFTSize(int newSize);
   void UpdateSample(Sample* sample, bool ownsSample);

   std::unique_ptr<FFT> mFFT;
   int mCurrentFFTSize{ 0 };

   // Windowing & FFT buffers
   std::vector<float> mWindow;
   std::vector<float> mFftInput;
   std::vector<float> mFftReal;
   std::vector<float> mFftImag;
   std::vector<float> mFftOut;

   // Overlap-add output ring buffer for L/R channels
   struct OverlapAddBuffer
   {
      std::vector<float> data;
      int readPos{ 0 };

      void Init(int size)
      {
         data.assign(size, 0.0f);
         readPos = 0;
      }

      void Add(const float* samples, int count, int offset)
      {
         int bufSize = (int)data.size();
         for (int i = 0; i < count; ++i)
         {
            int idx = (offset + i) % bufSize;
            data[idx] += samples[i];
         }
      }

      float ReadAndClear()
      {
         int bufSize = (int)data.size();
         float val = data[readPos];
         data[readPos] = 0.0f;
         readPos = (readPos + 1) % bufSize;
         return val;
      }
   };

   OverlapAddBuffer mOutputBuffer[2];

   float mStretch{ 8.0f };
   float mPhaseRand{ 1.0f };
   float mVolume{ 1.0f };
   float mWindowSizeIdx{ 2.0f }; // default 2 -> index 2 in our size array (65536)

   FloatSlider* mStretchSlider{ nullptr };
   FloatSlider* mPhaseRandSlider{ nullptr };
   FloatSlider* mVolumeSlider{ nullptr };
   FloatSlider* mWindowSizeSlider{ nullptr };

   float mHopAccumulator{ 0.0f };

   float mPitchShift{ 0.0f }; // semitones
   float mFineTune{ 0.0f }; // cents
   float mFreqShift{ 0.0f }; // Hz
   float mUnison{ 1.0f }; // voices
   float mDetune{ 10.0f }; // cents
   float mPlayStart{ 0.0f }; // 0 to 1
   float mPlayEnd{ 1.0f }; // 0 to 1
   float mCurrentPct{ 0.0f }; // 0 to 1
   float mLastCurrentPct{ 0.0f };
   bool mLoop{ true };

   FloatSlider* mPitchShiftSlider{ nullptr };
   FloatSlider* mFineTuneSlider{ nullptr };
   FloatSlider* mFreqShiftSlider{ nullptr };
   FloatSlider* mUnisonSlider{ nullptr };
   FloatSlider* mDetuneSlider{ nullptr };
   FloatSlider* mPlayStartSlider{ nullptr };
   FloatSlider* mPlayEndSlider{ nullptr };
   FloatSlider* mCurrentSlider{ nullptr };
   Checkbox* mLoopCheckbox{ nullptr };

   Sample* mSample{ nullptr };
   bool mOwnsSample{ true };
   bool mIsLoadingSample{ false };
   ChannelBuffer mDrawBuffer{ 0 };
   double mSamplePlayPosition{ 0.0 };
   ClickButton* mDeleteSampleButton{ nullptr };
};
