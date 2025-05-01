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
//  FMSynth.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/6/13.
//
//

#pragma once

#include "IAudioSource.h"
#include "PolyphonyMgr.h"
#include "FMVoice.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ADSRDisplay.h"

class FMSynth : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener
{
public:
   FMSynth();
   ~FMSynth();
   static IDrawableModule* Create() { return new FMSynth(); }
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

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool HasDebugDraw() const override { return true; }

   bool IsEnabled() const override { return mEnabled; }

private:
   void UpdateHarmonicRatio();

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 180;
      height = 203;
   }

   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   FMVoiceParams mVoiceParams;
   FloatSlider* mVolSlider{ nullptr };
   ADSRDisplay* mAdsrDisplayVol{ nullptr };
   FloatSlider* mPhaseOffsetSlider0{ nullptr };

   FloatSlider* mHarmSlider{ nullptr };
   ADSRDisplay* mAdsrDisplayHarm{ nullptr };
   FloatSlider* mModSlider{ nullptr };
   ADSRDisplay* mAdsrDisplayMod{ nullptr };
   int mHarmRatioBase{ 1 }; //negative means 1/val
   float mHarmRatioTweak{ 1 };
   DropdownList* mHarmRatioBaseDropdown{ nullptr };
   FloatSlider* mPhaseOffsetSlider1{ nullptr };

   FloatSlider* mHarmSlider2{ nullptr };
   ADSRDisplay* mAdsrDisplayHarm2{ nullptr };
   FloatSlider* mModSlider2{ nullptr };
   ADSRDisplay* mAdsrDisplayMod2{ nullptr };
   int mHarmRatioBase2{ 1 }; //negative means 1/val
   float mHarmRatioTweak2{ 1 };
   DropdownList* mHarmRatioBaseDropdown2{ nullptr };
   FloatSlider* mPhaseOffsetSlider2{ nullptr };

   ChannelBuffer mWriteBuffer;
};
