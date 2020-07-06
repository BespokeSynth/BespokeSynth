//
//  SeaOfGrain.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/8/14.
//
//

#ifndef __Bespoke__SeaOfGrain__
#define __Bespoke__SeaOfGrain__

#include <iostream>
#include "IAudioSource.h"
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

class SeaOfGrain : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   SeaOfGrain();
   ~SeaOfGrain();
   static IDrawableModule* Create() { return new SeaOfGrain(); }
   
   string GetTitleLabel() override { return "sea of grain"; }
   void CreateUIControls() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IDrawableModule
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   void Poll() override;
   
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
   void UpdateSample();
   void UpdateDisplaySamples();

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   
   struct GrainMPEVoice
   {
      GrainMPEVoice();
      void Process(float* out, int outLength, float* sample, int sampleLength);
      void Draw(float w, float h);
      
      float mPlay;
      float mPitch;
      ModulationChain* mPitchBend;
      ModulationChain* mPressure;
      ModulationChain* mModWheel;
      
      float mGain;
      
      ::ADSR mADSR;
      Granulator mGranulator;
      SeaOfGrain* mOwner;
   };
   
   struct GrainManualVoice
   {
      GrainManualVoice();
      void Process(float* out, int outLength, float* sample, int sampleLength);
      void Draw(float w, float h);
      
      float mGain;
      float mPosition;
      
      Granulator mGranulator;
      SeaOfGrain* mOwner;
      
      FloatSlider* mGainSlider;
      FloatSlider* mPositionSlider;
      FloatSlider* mOverlapSlider;
      FloatSlider* mSpeedSlider;
      FloatSlider* mLengthMsSlider;
      FloatSlider* mPosRandomizeSlider;
      FloatSlider* mSpeedRandomizeSlider;
   };
   
   static const int kNumMPEVoices = 16;
   GrainMPEVoice mMPEVoices[kNumMPEVoices];
   static const int kNumManualVoices = 4;
   GrainManualVoice mManualVoices[kNumManualVoices];
   
   Sample* mSample;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   float* mWriteBuffer;
   bool mLoading;
   FloatSlider* mDisplayOffsetSlider;
   float mDisplayOffset;
   FloatSlider* mDisplayLengthSlider;
   float mDisplayLength;
   int mDisplayStartSamples;
   int mDisplayEndSamples;
   DropdownList* mKeyboardBasePitchSelector;
   int mKeyboardBasePitch;
   DropdownList* mKeyboardNumPitchesSelector;
   int mKeyboardNumPitches;
};

#endif /* defined(__Bespoke__SeaOfGrain__) */

