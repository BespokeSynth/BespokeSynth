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
//  PadSynth.h
//  modularSynth
//
//  Created by Andrius Merkys on 4/8/25.
//
//

#pragma once

#include "IAudioSource.h"
#include "PolyphonyMgr.h"
#include "PadSynthVoice.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ADSRDisplay.h"
#include "Checkbox.h"
#include "BiquadFilterEffect.h"
#include "ChannelBuffer.h"

class PadSynth : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener
{
public:
   PadSynth();
   ~PadSynth();
   static IDrawableModule* Create() { return new PadSynth(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

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

   float mWidth{ 317 };
   float mHeight{ 87 };

   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   PadSynthVoiceParams mVoiceParams;
   FloatSlider* mFilterSlider{ nullptr };
   FloatSlider* mFeedbackSlider{ nullptr };
   float mVolume{ 1 };
   FloatSlider* mVolSlider{ nullptr };
   DropdownList* mSourceDropdown{ nullptr };
   BiquadFilterEffect mBiquad;
   BiquadFilter mDCRemover[ChannelBuffer::kMaxNumChannels];

   Checkbox* mStretchCheckbox{ nullptr };
   FloatSlider* mPitchToneSlider{ nullptr };
   FloatSlider* mVelToVolumeSlider{ nullptr };
   FloatSlider* mVelToEnvelopeSlider{ nullptr };
   FloatSlider* mBandwidthSlider{ nullptr };
   FloatSlider* mBandwidthScaleSlider{ nullptr };
   FloatSlider* mDetuneSlider{ nullptr };
   IntSlider* mHarmonicsSlider{ nullptr };
   Checkbox* mLiteCPUModeCheckbox{ nullptr };
   ADSRDisplay* mADSRDisplay{ nullptr };
   FloatSlider* mChannelOffsetSlider{ nullptr };

   ChannelBuffer mWriteBuffer;
};
