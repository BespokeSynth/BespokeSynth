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
//  SliderSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/25/14.
//
//

#pragma once

#include "Transport.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "TextEntry.h"

class SliderSequencer;

class SliderLine
{
public:
   SliderLine(SliderSequencer* owner, int x, int y, int index);
   void Draw();
   void CreateUIControls();

   float mPoint{ 0 };
   FloatSlider* mSlider{ nullptr };
   float mVelocity{ 0 };
   FloatSlider* mVelocitySlider{ nullptr };
   int mPitch{ 0 };
   TextEntry* mNoteSelector{ nullptr };
   double mPlayTime{ 0 };
   bool mPlaying{ false };
   Checkbox* mPlayingCheckbox{ nullptr };
   int mX{ 0 };
   int mY{ 0 };
   SliderSequencer* mOwner{ nullptr };
   int mIndex{ 0 };
};

class SliderSequencer : public IDrawableModule, public INoteSource, public IAudioPoller, public IFloatSliderListener, public IDropdownListener, public IIntSliderListener, public ITextEntryListener
{
public:
   SliderSequencer();
   ~SliderSequencer();
   static IDrawableModule* Create() { return new SliderSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool on) override { mEnabled = on; }

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   double MeasurePos(double time);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 320;
      height = 165;
   }

   float mLastMeasurePos{ 0 };
   std::vector<SliderLine*> mSliderLines;
   int mDivision{ 1 };
   IntSlider* mDivisionSlider{ nullptr };
};
