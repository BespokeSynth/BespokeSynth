//
//  SignalGenerator.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/26/14.
//
//

#ifndef __Bespoke__SignalGenerator__
#define __Bespoke__SignalGenerator__

#include <iostream>
#include "IAudioSource.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "EnvOscillator.h"
#include "Ramp.h"

class ofxJSONElement;

class SignalGenerator : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener
{
public:
   SignalGenerator();
   ~SignalGenerator();
   static IDrawableModule* Create() { return new SignalGenerator(); }
   
   string GetTitleLabel() override { return "sig gen"; }
   void CreateUIControls() override;
   
   void SetVol(float vol) { mVol = vol; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   enum FreqMode
   {
      kFreqMode_Instant,
      kFreqMode_Root,
      kFreqMode_Ramp,
      kFreqMode_Slider
   };
   
   void SetType(OscillatorType type);
   void SetFreqMode(FreqMode mode);
   
   float mVol;
   float mSmoothedVol;
   FloatSlider* mVolSlider;
   OscillatorType mOscType;
   DropdownList* mOscSelector;
   float mPulseWidth;
   FloatSlider* mPulseWidthSlider;
   float mShuffle;
   FloatSlider* mShuffleSlider;
   int mMult;
   DropdownList* mMultSelector;
   bool mSync;
   Checkbox* mSyncCheckbox;
   float mSyncFreq;
   FloatSlider* mSyncFreqSlider;
   float mDetune;
   FloatSlider* mDetuneSlider;
   EnvOscillator mOsc;
   float mFreq;
   FloatSlider* mFreqSlider;
   float mPhase;
   float mSyncPhase;
   FreqMode mFreqMode;
   DropdownList* mFreqModeSelector;
   float mFreqSliderStart;
   float mFreqSliderEnd;
   float mFreqSliderAmount;
   FloatSlider* mFreqSliderAmountSlider;
   Ramp mFreqRamp;
   float mFreqRampTime;
   FloatSlider* mFreqRampTimeSlider;
   float mSoften;
   FloatSlider* mSoftenSlider;
   float mPhaseOffset;
   FloatSlider* mPhaseOffsetSlider;
   
   float* mWriteBuffer;
};

#endif /* defined(__Bespoke__SignalGenerator__) */

