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
//  OutputChannel.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/17/12.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "PeakTracker.h"
#include "LevelMeterDisplay.h"

class OutputChannel : public IAudioProcessor, public IDrawableModule, public IDropdownListener
{
public:
   OutputChannel();
   virtual ~OutputChannel();
   static IDrawableModule* Create() { return new OutputChannel(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioReceiver
   InputMode GetInputMode() override { return mChannelSelectionIndex < mStereoSelectionOffset ? kInputMode_Mono : kInputMode_Multichannel; }

   //IAudioSource
   void Process(double time) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   int GetNumChannels() const { return mChannelSelectionIndex < mStereoSelectionOffset ? 1 : 2; }

   float mWidth{ 64 };
   float mHeight{ 40 };
   DropdownList* mChannelSelector{ nullptr };
   int mChannelSelectionIndex{ 0 };
   int mStereoSelectionOffset{ 0 };
   float mLimit{ 1 };

   LevelMeterDisplay mLevelMeterDisplay;
};
