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
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetType(OscillatorType type) { mVoiceParams.mOscType = type; }
   void SetDetune(float detune) { mVoiceParams.mDetune = detune; }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal, double time) override;

   bool HasDebugDraw() const override { return true; }

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void UpdateOldControlName(std::string& oldName) override;

   float mWidth{ 200 };
   float mHeight{ 20 };
   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   OscillatorVoiceParams mVoiceParams;
   FloatSlider* mVolSlider{ nullptr };
   FloatSlider* mPhaseOffsetSlider{ nullptr };
   DropdownList* mOscSelector{ nullptr };
   FloatSlider* mPulseWidthSlider{ nullptr };
   FloatSlider* mSoftenSlider{ nullptr };
   int mMult{ 1 };
   DropdownList* mMultSelector{ nullptr };
   ADSRDisplay* mADSRDisplay{ nullptr };
   Checkbox* mSyncCheckbox{ nullptr };
   FloatSlider* mSyncFreqSlider{ nullptr };
   FloatSlider* mDetuneSlider{ nullptr };
   IntSlider* mUnisonSlider{ nullptr };
   FloatSlider* mUnisonWidthSlider{ nullptr };
   FloatSlider* mShuffleSlider{ nullptr };
   FloatSlider* mVelToVolumeSlider{ nullptr };
   FloatSlider* mVelToEnvelopeSlider{ nullptr };
   Checkbox* mLiteCPUModeCheckbox{ nullptr };

   FloatSlider* mFilterCutoffMaxSlider{ nullptr };
   FloatSlider* mFilterCutoffMinSlider{ nullptr };
   FloatSlider* mFilterQSlider{ nullptr };
   ADSRDisplay* mFilterADSRDisplay{ nullptr };

   ChannelBuffer mWriteBuffer;

   Oscillator mDrawOsc{ OscillatorType::kOsc_Square };

   struct DebugLine
   {
      std::string text;
      ofColor color;
   };

   std::array<DebugLine, 20> mDebugLines;
   int mDebugLinesPos{ 0 };
};


#endif /* defined(__modularSynth__SingleOscillator__) */
