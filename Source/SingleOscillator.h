/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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
#include "Oscillator.h"

class ofxJSONElement;

class SingleOscillator : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IRadioButtonListener
{
public:
   SingleOscillator();
   ~SingleOscillator();
   static IDrawableModule* Create() { return new SingleOscillator(); }


   void CreateUIControls() override;

   void SetType(OscillatorType type) { mVoiceParams.mOscType = type; }
   void SetDetune(float detune) { mVoiceParams.mDetune = detune; }

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
   void RadioButtonUpdated(RadioButton* list, int oldVal) override;

   bool HasDebugDraw() const override { return true; }

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   bool Enabled() const override { return mEnabled; }
   void UpdateOldControlName(std::string& oldName) override;

   float mWidth;
   float mHeight;
   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   OscillatorVoiceParams mVoiceParams;
   FloatSlider* mVolSlider;
   FloatSlider* mPhaseOffsetSlider;
   DropdownList* mOscSelector;
   FloatSlider* mPulseWidthSlider;
   FloatSlider* mSoftenSlider;
   int mMult;
   DropdownList* mMultSelector;
   ADSRDisplay* mADSRDisplay;
   Checkbox* mSyncCheckbox;
   FloatSlider* mSyncFreqSlider;
   FloatSlider* mDetuneSlider;
   IntSlider* mUnisonSlider;
   FloatSlider* mUnisonWidthSlider;
   FloatSlider* mShuffleSlider;
   float mLengthMultiplier;
   FloatSlider* mLengthMultiplierSlider;
   FloatSlider* mVelToVolumeSlider;
   FloatSlider* mVelToEnvelopeSlider;
   Checkbox* mLiteCPUModeCheckbox;

   FloatSlider* mFilterCutoffMaxSlider;
   FloatSlider* mFilterCutoffMinSlider;
   FloatSlider* mFilterQSlider;
   ADSRDisplay* mFilterADSRDisplay;

   ChannelBuffer mWriteBuffer;

   Oscillator mDrawOsc;

   std::string mDebugLines;
};


#endif /* defined(__modularSynth__SingleOscillator__) */
