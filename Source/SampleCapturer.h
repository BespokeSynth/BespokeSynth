/*
  ==============================================================================

    SampleCapturer.h
    Created: 12 Nov 2020 6:36:00pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include <array>

class SampleCapturer : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IButtonListener
{
public:
   SampleCapturer();
   virtual ~SampleCapturer();
   static IDrawableModule* Create() { return new SampleCapturer(); }

   string GetTitleLabel() override { return "sample capturer"; }
   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void ButtonClicked(ClickButton* button) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   static const int kMaxSampleLengthSeconds = 3;
   struct SampleElement
   {
      SampleElement()
      : mBuffer(gSampleRate * kMaxSampleLengthSeconds)
      , mRecordingLength(0)
      , mPlaybackPos(-1)
      {
      }

      ChannelBuffer mBuffer;
      int mRecordingLength;
      int mPlaybackPos;
   };
   std::array<SampleElement, 10> mSamples;
   int mCurrentSampleIndex;
   bool mWantRecord;
   Checkbox* mWantRecordCheckbox;
   bool mIsRecording;
   ClickButton* mDeleteButton;
   ClickButton* mSaveButton;
   ClickButton* mPlayButton;
   bool mIsDragging;
};
