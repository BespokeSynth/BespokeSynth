/*
  ==============================================================================

    MultitapDelay.h
    Created: 25 Nov 2018 11:16:38am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "INoteReceiver.h"
#include "Granulator.h"
#include "ADSR.h"

class Sample;

class MultitapDelay : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   MultitapDelay();
   ~MultitapDelay();
   static IDrawableModule* Create() { return new MultitapDelay(); }
   
   string GetTitleLabel() override { return "multitap delay"; }
   void CreateUIControls() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   //IFloatSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IDropdownListener
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   
   struct DelayTap
   {
      DelayTap();
      void Process(float* sampleOut, int offset, int ch);
      void Draw(float w, float h);
      
      float mDelayMs;
      float mGain;
      float mFeedback;
      
      MultitapDelay* mOwner;
      
      FloatSlider* mDelayMsSlider;
      FloatSlider* mGainSlider;
      FloatSlider* mFeedbackSlider;
      
      ChannelBuffer mTapBuffer;
   };
   
   struct DelayMPETap
   {
      DelayMPETap();
      void Process(float* sampleOut, int offset, int ch);
      void Draw(float w, float h);
      
      float mPlay;
      float mPitch;
      ModulationChain* mPitchBend;
      ModulationChain* mPressure;
      ModulationChain* mModWheel;
      
      ::ADSR mADSR;
   
      MultitapDelay* mOwner;
   };
   
   int mNumTaps;
   vector<DelayTap> mTaps;
   static const int kNumMPETaps = 16;
   DelayMPETap mMPETaps[kNumMPETaps];
   
   ChannelBuffer mWriteBuffer;
   FloatSlider* mDryAmountSlider;
   float mDryAmount;
   FloatSlider* mDisplayLengthSlider;
   float mDisplayLength;
   RollingBuffer mDelayBuffer;
};
