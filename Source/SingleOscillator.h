//
//  SingleOscillator.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/13.
//
//

#ifndef __modularSynth__SingleOscillator__
#define __modularSynth__SingleOscillator__

#include <iostream>
#include "IAudioSource.h"
#include "PolyphonyMgr.h"
#include "SingleOscillatorVoice.h"
#include "ADSR.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ADSRDisplay.h"
#include "Checkbox.h"
#include "RadioButton.h"

class ofxJSONElement;

class SingleOscillator : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IRadioButtonListener
{
public:
   SingleOscillator();
   ~SingleOscillator();
   static IDrawableModule* Create() { return new SingleOscillator(); }
   
   string GetTitleLabel() override { return "osc"; }
   void CreateUIControls() override;
   
   void SetVol(float vol) { mVoiceParams.mVol = vol; mADSRDisplay->SetVol(vol); }
   void SetType(OscillatorType type) { mVoiceParams.mOscType = type; }
   void SetDetune(float detune) { mVoiceParams.mDetune = detune; }
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   bool Enabled() const override { return mEnabled; }

   void UpdateADSRDisplays();
   
   PolyphonyMgr mPolyMgr;
   OscillatorVoiceParams mVoiceParams;
   FloatSlider* mVolSlider;
   DropdownList* mOscSelector;
   FloatSlider* mPulseWidthSlider;
   int mMult;
   DropdownList* mMultSelector;
   ADSRDisplay* mADSRDisplay;
   Checkbox* mSyncCheckbox;
   FloatSlider* mSyncFreqSlider;
   FloatSlider* mDetuneSlider;
   FloatSlider* mShuffleSlider;
   
   FloatSlider* mFilterCutoffSlider;
   ADSRDisplay* mFilterADSRDisplay;
   
   RadioButton* mADSRModeSelector;
   int mADSRMode;

   float* mWriteBuffer;
};



#endif /* defined(__modularSynth__SingleOscillator__) */

